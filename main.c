#include "chess.h"





int main(void) {
    
    Game chess;

    draw_game(&chess);


    printf("sizeof P:     %lu\n", sizeof(P));
    printf("sizeof Move:  %lu\n", sizeof(Move));
    printf("sizeof Game:  %lu\n", sizeof(Game));
    printf("sizeof Board: %lu\n", sizeof(Board));

}


