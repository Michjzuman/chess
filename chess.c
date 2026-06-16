#include "chess.h"

/////////////////////////////////////////
///[ chess.c ]///////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

const U8 piece_values[] = PIECE_VALUES;
const char abc[] = ABC;
 

Board calculate_board(const Game *game) {
    Board board;

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            board.squares[y][x] = (
                y == 1 || y == SIZE - 2 ? (Piece){
                    PAWN, y == 1 ? WHITE : BLACK
                } :
                (Piece){EMPTY, BLANK}
            );
        }
    }



    return board;
}

void calculate_possible_moves(Game *game) {

}

