#define _POSIX_C_SOURCE 200809L
#include <arpa/inet.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int send_all(int fd, const char *s) {
    size_t n = strlen(s), off = 0;
    while (off < n) {
        ssize_t sent = send(fd, s + off, n - off, 0);
        if (sent <= 0) return 0;
        off += (size_t)sent;
    }
    return 1;
}
static int read_line(int fd, char *buffer, size_t capacity) {
    size_t used = 0;
    for (;;) {
        char ch;
        ssize_t n = recv(fd, &ch, 1, 0);
        if (n <= 0) return 0;
        if (ch == '\r') continue;
        if (ch == '\n') { buffer[used] = '\0'; return 1; }
        if (used + 1 >= capacity) return 0;
        buffer[used++] = ch;
    }
}
static void print_board(const char *board) {
    if (strlen(board) != 64) { puts("Invalid board message"); return; }
    puts("\n     a b c d e f g h");
    puts("   +-----------------+");
    for (int r = 0; r < 8; ++r) {
        printf(" %d |", 8-r);
        for (int c = 0; c < 8; ++c) printf(" %c", board[r*8+c]);
        printf(" | %d\n", 8-r);
    }
    puts("   +-----------------+");
    puts("     a b c d e f g h\n");
}
static int valid_port(const char *s) {
    char *end = NULL;
    long n = strtol(s, &end, 10);
    return s[0] && end && !*end && n >= 1 && n <= 65535 ? (int)n : 0;
}
int main(int argc, char **argv) {
    const char *host = argc > 1 ? argv[1] : "127.0.0.1";
    int port = argc > 2 ? valid_port(argv[2]) : 9999;
    if (argc > 3 || !port) { fprintf(stderr, "Usage: %s [server_ipv4] [port]\n", argv[0]); return 1; }
    signal(SIGPIPE, SIG_IGN);
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { perror("socket"); return 1; }
    struct sockaddr_in address = {0};
    address.sin_family = AF_INET;
    address.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, host, &address.sin_addr) != 1) {
        fprintf(stderr, "Invalid IPv4 address: %s\n", host); close(fd); return 1;
    }
    if (connect(fd, (struct sockaddr *)&address, sizeof address) < 0) { perror("connect"); close(fd); return 1; }
    char line[256];
    while (read_line(fd, line, sizeof line)) {
        if (strncmp(line, "BOARD ", 6) == 0 && strlen(line) == 72) {
            char board[65]; memcpy(board, line + 6, 64); board[64] = '\0';
            print_board(board);
            printf("To play: %s\n", line[71] == 'W' ? "White" : "Black");
        } else if (strcmp(line, "YOUR_TURN") == 0) {
            char input[64];
            printf("Your move (e2e4, e7e8q, or quit): "); fflush(stdout);
            if (!fgets(input, sizeof input, stdin)) { send_all(fd, "QUIT\n"); break; }
            input[strcspn(input, "\r\n")] = '\0';
            if (strcmp(input, "quit") == 0) {
                send_all(fd, "QUIT\n"); break;
            }
            if (strlen(input) < 4 || strlen(input) > 5) {
                puts("Enter 4 coordinates and optionally a promotion piece (q/r/b/n).");
                send_all(fd, "MOVE ?\n"); /* server will request retry */
            } else {
                char command[80];
                snprintf(command, sizeof command, "MOVE %s\n", input);
                if (!send_all(fd, command)) break;
            }
        } else if (strncmp(line, "GAMEOVER ", 9) == 0) {
            printf("Game over: %s\n", line+9); break;
        } else if (strncmp(line, "HELLO ", 6) == 0 || strncmp(line, "INFO ", 5) == 0 || strncmp(line, "ERROR ", 6) == 0) {
            puts(line);
        } else {
            puts("Unexpected server response"); break;
        }
    }
    close(fd);
    return 0;
}
