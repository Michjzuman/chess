#include "chess.h"





int main(void) {
    
    Game chess;

    draw_game(&chess);

    draw_game_with_cursor(&chess, (P){3, 4});

}


