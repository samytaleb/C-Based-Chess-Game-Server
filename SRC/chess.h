#ifndef CHESS_H
#define CHESS_H
#include <stddef.h>

typedef struct {
    char squares[64];
    int turn; /* 0 white, 1 black */
    unsigned castle; /* 1 white K, 2 white Q, 4 black K, 8 black Q */
    int en_passant; /* target square or -1 */
} Chess;

void chess_init(Chess *game);
/* Move in coordinate notation (e2e4, e7e8q). Returns 1 when legal. */
int chess_move(Chess *game, const char *move, char *error, size_t error_size);
int chess_in_check(const Chess *game, int side);
int chess_has_legal_move(const Chess *game);
/* Returns 0 ongoing, 1 checkmate, 2 stalemate. */
int chess_status(const Chess *game);
#endif
