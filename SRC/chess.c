#include "chess.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define WHITE 0
#define BLACK 1
#define W_K 1u
#define W_Q 2u
#define B_K 4u
#define B_Q 8u

static int row(int sq) { return sq / 8; }
static int col(int sq) { return sq % 8; }
static int sq(int r, int c) { return r * 8 + c; }
static int side_of(char p) { return isupper((unsigned char)p) ? WHITE : BLACK; }
static int enemy(char p, int side) { return p != '.' && side_of(p) != side; }
static void fail(char *error, size_t n, const char *message) {
    if (error && n) snprintf(error, n, "%s", message);
}

void chess_init(Chess *g) {
    memcpy(g->squares, "rnbqkbnrpppppppp................................PPPPPPPPRNBQKBNR", 64);
    g->turn = WHITE;
    g->castle = W_K | W_Q | B_K | B_Q;
    g->en_passant = -1;
}

static int clear_path(const Chess *g, int from, int to) {
    int dr = (row(to) > row(from)) - (row(to) < row(from));
    int dc = (col(to) > col(from)) - (col(to) < col(from));
    int r = row(from) + dr, c = col(from) + dc;
    while (r != row(to) || c != col(to)) {
        if (g->squares[sq(r, c)] != '.') return 0;
        r += dr; c += dc;
    }
    return 1;
}

static int attacked(const Chess *g, int target, int by) {
    int tr = row(target), tc = col(target);
    for (int from = 0; from < 64; ++from) {
        char p = g->squares[from];
        if (p == '.' || side_of(p) != by) continue;
        int dr = tr - row(from), dc = tc - col(from);
        int ar = abs(dr), ac = abs(dc);
        switch (tolower((unsigned char)p)) {
            case 'p': if (dr == (by == WHITE ? -1 : 1) && ac == 1) return 1; break;
            case 'n': if ((ar == 1 && ac == 2) || (ar == 2 && ac == 1)) return 1; break;
            case 'b': if (ar == ac && ar && clear_path(g, from, target)) return 1; break;
            case 'r': if (((dr == 0 && dc != 0) || (dc == 0 && dr != 0)) && clear_path(g, from, target)) return 1; break;
            case 'q': if ((((dr == 0 && dc != 0) || (dc == 0 && dr != 0)) || (ar == ac && ar)) && clear_path(g, from, target)) return 1; break;
            case 'k': if (ar <= 1 && ac <= 1 && (ar || ac)) return 1; break;
        }
    }
    return 0;
}

int chess_in_check(const Chess *g, int side) {
    char king = side == WHITE ? 'K' : 'k';
    for (int i = 0; i < 64; ++i)
        if (g->squares[i] == king) return attacked(g, i, 1 - side);
    return 1; /* missing king must never make a position legal */
}

static int apply(Chess *g, int from, int to, char promotion) {
    char p = g->squares[from], destination = g->squares[to];
    int side = g->turn;
    int dr = row(to) - row(from), dc = col(to) - col(from);
    int ar = abs(dr), ac = abs(dc);
    int special_castle = 0, special_ep = 0;
    if (from == to || p == '.' || side_of(p) != side ||
        (destination != '.' && side_of(destination) == side) ||
        tolower((unsigned char)destination) == 'k') return 0;
    if (promotion && tolower((unsigned char)p) != 'p') return 0;
    switch (tolower((unsigned char)p)) {
        case 'p': {
            int step = side == WHITE ? -1 : 1;
            int start = side == WHITE ? 6 : 1;
            if (dc == 0 && destination == '.' &&
                (dr == step || (dr == 2 * step && row(from) == start &&
                    g->squares[from + step * 8] == '.'))) {
                /* ordinary pawn advance */
            } else if (ac == 1 && dr == step && enemy(destination, side)) {
                /* ordinary capture */
            } else if (ac == 1 && dr == step && to == g->en_passant && destination == '.' &&
                       g->squares[to - step * 8] == (side == WHITE ? 'p' : 'P')) {
                special_ep = 1;
            } else return 0;
            if (row(to) == (side == WHITE ? 0 : 7)) {
                if (!promotion || !strchr("qrbn", promotion)) return 0;
            } else if (promotion) return 0;
            break;
        }
        case 'n': if (!((ar == 1 && ac == 2) || (ar == 2 && ac == 1))) return 0; break;
        case 'b': if (!(ar == ac && ar && clear_path(g, from, to))) return 0; break;
        case 'r': if (!(((dr == 0 && dc) || (dc == 0 && dr)) && clear_path(g, from, to))) return 0; break;
        case 'q': if (!(((((dr == 0 && dc) || (dc == 0 && dr)) || (ar == ac && ar))) && clear_path(g, from, to))) return 0; break;
        case 'k': {
            if (ar <= 1 && ac <= 1) break;
            int home = side == WHITE ? 7 : 0;
            if (row(from) != home || col(from) != 4 || row(to) != home || ar != 0 || ac != 2 || chess_in_check(g, side)) return 0;
            if (dc == 2) {
                if (!(g->castle & (side == WHITE ? W_K : B_K)) ||
                    g->squares[sq(home, 7)] != (side == WHITE ? 'R' : 'r') ||
                    g->squares[sq(home, 5)] != '.' || g->squares[sq(home, 6)] != '.' ||
                    attacked(g, sq(home, 5), 1 - side) || attacked(g, sq(home, 6), 1 - side)) return 0;
                special_castle = 1;
            } else if (dc == -2) {
                if (!(g->castle & (side == WHITE ? W_Q : B_Q)) ||
                    g->squares[sq(home, 0)] != (side == WHITE ? 'R' : 'r') ||
                    g->squares[sq(home, 1)] != '.' || g->squares[sq(home, 2)] != '.' ||
                    g->squares[sq(home, 3)] != '.' || attacked(g, sq(home, 3), 1 - side) ||
                    attacked(g, sq(home, 2), 1 - side)) return 0;
                special_castle = 2;
            } else return 0;
            break;
        }
        default: return 0;
    }
    Chess next = *g;
    next.en_passant = -1;
    next.squares[to] = p;
    next.squares[from] = '.';
    if (special_ep) next.squares[to + (side == WHITE ? 8 : -8)] = '.';
    if (special_castle) {
        int home = side == WHITE ? 7 : 0;
        int rook_from = sq(home, special_castle == 1 ? 7 : 0);
        int rook_to = sq(home, special_castle == 1 ? 5 : 3);
        next.squares[rook_to] = next.squares[rook_from];
        next.squares[rook_from] = '.';
    }
    if (tolower((unsigned char)p) == 'p') {
        if (ar == 2) next.en_passant = (from + to) / 2;
        if (promotion) next.squares[to] = side == WHITE ? (char)toupper((unsigned char)promotion) : promotion;
    }
    if (p == 'K') next.castle &= ~(W_K | W_Q);
    if (p == 'k') next.castle &= ~(B_K | B_Q);
    if (from == 63 || to == 63) next.castle &= ~W_K;
    if (from == 56 || to == 56) next.castle &= ~W_Q;
    if (from == 7 || to == 7) next.castle &= ~B_K;
    if (from == 0 || to == 0) next.castle &= ~B_Q;
    if (chess_in_check(&next, side)) return 0;
    next.turn = 1 - side;
    *g = next;
    return 1;
}

int chess_move(Chess *g, const char *move, char *error, size_t error_size) {
    size_t len = strlen(move);
    if ((len != 4 && len != 5) || move[0] < 'a' || move[0] > 'h' ||
        move[2] < 'a' || move[2] > 'h' || move[1] < '1' || move[1] > '8' ||
        move[3] < '1' || move[3] > '8' || (len == 5 && !strchr("qrbn", move[4]))) {
        fail(error, error_size, "Use coordinates like e2e4 or e7e8q"); return 0;
    }
    int from = sq(8 - (move[1] - '0'), move[0] - 'a');
    int to = sq(8 - (move[3] - '0'), move[2] - 'a');
    if (!apply(g, from, to, len == 5 ? move[4] : 0)) {
        fail(error, error_size, "Illegal move (check turn, movement and king safety)"); return 0;
    }
    return 1;
}

int chess_has_legal_move(const Chess *g) {
    for (int from = 0; from < 64; ++from) {
        char p = g->squares[from];
        if (p == '.' || side_of(p) != g->turn) continue;
        for (int to = 0; to < 64; ++to) {
            Chess trial = *g;
            int promotes = tolower((unsigned char)p) == 'p' && row(to) == (g->turn == WHITE ? 0 : 7);
            if (apply(&trial, from, to, promotes ? 'q' : 0)) return 1;
        }
    }
    return 0;
}

int chess_status(const Chess *g) {
    if (chess_has_legal_move(g)) return 0;
    return chess_in_check(g, g->turn) ? 1 : 2;
}
