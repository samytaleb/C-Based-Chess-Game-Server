#define _POSIX_C_SOURCE 200809L
#include "chess.h"
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_GAMES 5
#define MAX_LINE 128

typedef struct { int players[2]; } Match;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int active_games = 0;

static int write_all(int fd, const char *message) {
    size_t len = strlen(message), off = 0;
    while (off < len) {
        ssize_t n = send(fd, message + off, len - off, 0);
        if (n <= 0) return 0;
        off += (size_t)n;
    }
    return 1;
}

static int read_line(int fd, char *out, size_t capacity) {
    size_t used = 0;
    for (;;) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n <= 0) return 0;
        if (ch == '\r') continue;
        if (ch == '\n') { out[used] = '\0'; return 1; }
        if (used + 1 >= capacity) return 0;
        out[used++] = ch;
    }
}

static void game_over(Match *m, const char *reason) {
    char line[160];
    snprintf(line, sizeof line, "GAMEOVER %s\n", reason);
    write_all(m->players[0], line);
    write_all(m->players[1], line);
}

static int send_board(Match *m, const Chess *g) {
    char line[88];
    snprintf(line, sizeof line, "BOARD %.64s %c\n", g->squares, g->turn == 0 ? 'W' : 'B');
    return write_all(m->players[0], line) && write_all(m->players[1], line);
}

static void *play_game(void *argument) {
    Match *m = argument;
    Chess game;
    chess_init(&game);
    if (!write_all(m->players[0], "HELLO WHITE\n") ||
        !write_all(m->players[1], "HELLO BLACK\n") || !send_board(m, &game)) goto cleanup;
    for (;;) {
        int player = game.turn;
        char command[MAX_LINE], error[120];
        if (!write_all(m->players[player], "YOUR_TURN\n")) {
            game_over(m, "PLAYER_DISCONNECTED"); break;
        }
        if (!read_line(m->players[player], command, sizeof command)) {
            game_over(m, "PLAYER_DISCONNECTED"); break;
        }
        if (strcmp(command, "QUIT") == 0) {
            game_over(m, "PLAYER_QUIT"); break;
        }
        if (strncmp(command, "MOVE ", 5) != 0 ||
            !chess_move(&game, command + 5, error, sizeof error)) {
            char reply[160];
            snprintf(reply, sizeof reply, "ERROR %s\n", strncmp(command, "MOVE ", 5) == 0 ? error : "Expected MOVE e2e4");
            if (!write_all(m->players[player], reply)) break;
            continue;
        }
        if (!send_board(m, &game)) break;
        int status = chess_status(&game);
        if (status == 1) {
            game_over(m, game.turn == 0 ? "CHECKMATE BLACK_WINS" : "CHECKMATE WHITE_WINS");
            break;
        }
        if (status == 2) { game_over(m, "STALEMATE"); break; }
        if (chess_in_check(&game, game.turn)) {
            write_all(m->players[0], "INFO Check\n");
            write_all(m->players[1], "INFO Check\n");
        }
    }
cleanup:
    shutdown(m->players[0], SHUT_RDWR);
    shutdown(m->players[1], SHUT_RDWR);
    close(m->players[0]); close(m->players[1]);
    free(m);
    pthread_mutex_lock(&mutex);
    --active_games;
    pthread_mutex_unlock(&mutex);
    return NULL;
}

static int valid_port(const char *value) {
    char *end = NULL;
    long num = strtol(value, &end, 10);
    return value[0] && end && !*end && num >= 1 && num <= 65535 ? (int)num : 0;
}

int main(int argc, char **argv) {
    const char *ip = argc > 2 ? argv[2] : "127.0.0.1";
    int port = argc > 1 ? valid_port(argv[1]) : 9999;
    if (argc > 3 || !port) {
        fprintf(stderr, "Usage: %s [port] [bind_ipv4]\n", argv[0]); return 1;
    }
    signal(SIGPIPE, SIG_IGN);
    int listener = socket(AF_INET, SOCK_STREAM, 0);
    if (listener < 0) { perror("socket"); return 1; }
    int reuse = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", ip); close(listener); return 1;
    }
    if (bind(listener, (struct sockaddr *)&addr, sizeof addr) != 0 ||
        listen(listener, 10) != 0) { perror("bind/listen"); close(listener); return 1; }
    printf("Chess server listening on %s:%d (up to %d concurrent games)\n", ip, port, MAX_GAMES);
    fflush(stdout);
    for (;;) {
        pthread_mutex_lock(&mutex);
        int full = active_games >= MAX_GAMES;
        pthread_mutex_unlock(&mutex);
        if (full) { sleep(1); continue; }
        int white = accept(listener, NULL, NULL);
        if (white < 0) { if (errno == EINTR) continue; perror("accept"); break; }
        if (!write_all(white, "INFO Waiting for a second player...\n")) { close(white); continue; }
        int black = accept(listener, NULL, NULL);
        if (black < 0) { close(white); if (errno == EINTR) continue; perror("accept"); break; }
        Match *match = malloc(sizeof *match);
        if (!match) { close(white); close(black); continue; }
        match->players[0] = white; match->players[1] = black;
        pthread_mutex_lock(&mutex); ++active_games; pthread_mutex_unlock(&mutex);
        pthread_t thread;
        if (pthread_create(&thread, NULL, play_game, match) != 0) {
            perror("pthread_create"); close(white); close(black); free(match);
            pthread_mutex_lock(&mutex); --active_games; pthread_mutex_unlock(&mutex);
            continue;
        }
        pthread_detach(thread);
    }
    close(listener);
    return 1;
}
