#include "chess.h"

int main(void) {
    
    Game chess;

    do_move(&chess, (Move){
        (P){4, 1},
        (P){4, 3}
    });

    draw_game(&chess);

    draw_game_with_cursor(&chess, (P){3, 3});

    printf("sizeof P:     %lu\n", sizeof(P));
    printf("sizeof Move:  %lu\n", sizeof(Move));
    printf("sizeof Piece: %lu\n", sizeof(Piece));
    printf("sizeof Game:  %lu\n", sizeof(Game));
    printf("sizeof Board: %lu\n", sizeof(Board));

}


