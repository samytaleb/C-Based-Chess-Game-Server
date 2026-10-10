#include "../SRC/chess.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static void play(Chess *g, const char *move) {
    char error[120] = {0};
    if (!chess_move(g, move, error, sizeof error)) { fprintf(stderr, "%s: %s\n", move, error); assert(0); }
}
static void refuse(Chess *g, const char *move) {
    char error[120] = {0};
    Chess snapshot = *g;
    assert(!chess_move(g, move, error, sizeof error));
    assert(memcmp(g, &snapshot, sizeof snapshot) == 0);
}
int main(void) {
    Chess g; chess_init(&g);
    refuse(&g, "e2e5"); refuse(&g, "e7e5"); refuse(&g, "f1b5");
    play(&g, "f2f3"); play(&g, "e7e5"); play(&g, "g2g4"); play(&g, "d8h4");
    assert(chess_status(&g) == 1);
    chess_init(&g);
    play(&g,"e2e4"); play(&g,"a7a6"); play(&g,"e4e5"); play(&g,"d7d5");
    play(&g,"e5d6"); assert(g.squares[2*8+3] == 'P'); assert(g.squares[3*8+3] == '.');
    chess_init(&g);
    play(&g,"g1f3"); play(&g,"g8f6"); play(&g,"e2e4"); play(&g,"e7e5");
    play(&g,"f1e2"); play(&g,"f8e7"); play(&g,"e1g1");
    assert(g.squares[62] == 'K' && g.squares[61] == 'R');
    chess_init(&g);
    for (int i = 0; i < 64; ++i) g.squares[i] = '.';
    g.squares[60] = 'K'; g.squares[4] = 'k'; g.squares[8] = 'P';
    refuse(&g,"a7a8"); play(&g,"a7a8q"); assert(g.squares[0] == 'Q');
    chess_init(&g); refuse(&g,"e2e4xxxxxxxx"); refuse(&g,"e2e9"); refuse(&g,"g1f3q");
    puts("Chess rules tests OK"); return 0;
}
