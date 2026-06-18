#include "chess.h"

/////////////////////////////////////////
///[ chess.c ]///////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

const U8 piece_values[] = PIECE_VALUES;
const char abc[] = ABC;

Game new_game() {
    Game game;
    
    game.amount_of_moves = 0;
    game.moves_capacity = 2;
    game.moves = malloc(
        game.moves_capacity * sizeof(Move)
    );

    game.turn = WHITE;

    return game;
}

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

    for (U16 i = 0; i < game->amount_of_moves; i++) {
        board.squares[
            game->moves[i].end.y
        ][
            game->moves[i].end.x
        ] = board.squares[
            game->moves[i].start.y
        ][
            game->moves[i].start.x
        ];
        board.squares[
            game->moves[i].start.y
        ][
            game->moves[i].start.x
        ] = (Piece){EMPTY, BLANK};
    }

    return board;
}

void do_move(Game *game, Move move) {
    game->amount_of_moves++;

    if (game->amount_of_moves > game->moves_capacity) {
        game->moves_capacity *= 2;
        game->moves = realloc(
            game->moves,
            game->moves_capacity * sizeof(Move)
        );
    }

    // TODO: moved_king, moved_rook_r, moved_rook_l

    game->moves[game->amount_of_moves - 1] = move;
}


static void add_possible_move(Game *game, Move move) {
    if (
        move.end.x > 0 && move.end.x < SIZE &&
        move.end.y > 0 && move.end.y < SIZE /*&&
        board.squares[y][x].color !=
        board.squares[y + ay][x + ax].color &&
        board.squares[y][x].color == game->turn*/
    ) {
        game->amount_of_possible_moves++;
        if (game->possible_moves_capacity < game->amount_of_possible_moves) {
            game->possible_moves_capacity *= 2;
            game->possible_moves = realloc(
                game->possible_moves, game->possible_moves_capacity * sizeof(Move)
            );
        }
        game->possible_moves[game->amount_of_possible_moves - 1] = move;
    }
}

void calculate_possible_moves(Game *game) {
    Board board = calculate_board(game);
    
    game->amount_of_possible_moves = 0;
    game->possible_moves_capacity = 2;
    game->possible_moves = malloc(
        game->possible_moves_capacity * sizeof(Move)
    );

    /* --- TODO LIST --- *\
    *
    * KNIGHT
    * ROOK
    * BISHOP
    * PAWN
    * Promotion
    * Castling
    * En Passant
    * 
    \* --- ---- ---- --- */

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            Piece square = board.squares[y][x];
            if (square.type > 0) {
                if (square.type == KING) {
                    for (I8 ay = -1; ay <= 1; ay++) {
                        for (I8 ax = -1; ax <= 1; ax++) {
                            if (
                                !(ax == 0 && ay == 0) &&
                                x + ax > 0 && x + ax < SIZE &&
                                y + ay > 0 && y + ay < SIZE &&
                                board.squares[y][x].color !=
                                board.squares[y + ay][x + ax].color &&
                                board.squares[y][x].color == game->turn
                            ) {
                                add_possible_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
                        }
                    }
                } else if (square.type == KNIGHT) {
                    for (I8 ay = -2; ay <= 2; ay++) {
                        for (I8 ax = -2; ax <= 2; ax++) {
                            if (

                                x + ax > 0 && x + ax < SIZE &&
                                y + ay > 0 && y + ay < SIZE &&
                                board.squares[y][x].color !=
                                board.squares[y + ay][x + ax].color &&
                                board.squares[y][x].color == game->turn
                            ) {
                                add_possible_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
                        }
                    }
                } else if (square.type == PAWN) {
                    //printf("%d, %d\n", x, y);
                }
                if (square.type == ROOK || square.type == QUEEN) {
                    //printf("%d, %d\n", x, y);
                }
                if (square.type == BISHOP || square.type == QUEEN) {
                    //printf("%d, %d\n", x, y);
                }
            }
        }
    }
}



