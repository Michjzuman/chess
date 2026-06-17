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
                y == 0 || y == SIZE - 1 ? (Piece){
                    (
                        x == 0 || x == SIZE - 1 ? ROOK :
                        x == 1 || x == SIZE - 2 ? KNIGHT :
                        x == 2 || x == SIZE - 3 ? BISHOP :
                        x == 3 ? QUEEN : KING
                    ),
                    y == 0 ? WHITE : BLACK
                } :
                (Piece){EMPTY, BLANK}
            );
        }
    }

    return board;
}

void calculate_possible_moves(Game *game) {
    
}


void do_move(Game *game, Move move) {
    game->amount_of_moves++;

    if (game->amount_of_moves > game->moves_capacity) {
        if (game->moves_capacity == 0) {
            game->moves_capacity = 2;
	        game->moves = malloc(
                game->moves_capacity * sizeof(Move)
            );
	} else {
            game->moves_capacity *= 2;
            game->moves = realloc(
                game->moves,
                game->moves_capacity * sizeof(Move)
            );
	}
    }

    game->moves[game->amount_of_moves - 1] = move;
}



