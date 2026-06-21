#include "chess.h"

/////////////////////////////////////////
///[ chess.c ]///////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////


static const U8 piece_values[] = PIECE_VALUES;
static const char piece_letters[] = PIECE_LETTERS;
static const char abc[] = ABC;


static void calculate_board(Game *game) {
    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            game->board[y][x] = (
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
        game->board[
            game->moves[i].end.y
        ][
            game->moves[i].end.x
        ] = game->board[
            game->moves[i].start.y
        ][
            game->moves[i].start.x
        ];
        game->board[
            game->moves[i].start.y
        ][
            game->moves[i].start.x
        ] = (Piece){EMPTY, BLANK};
    }
}


static void check_check(Game *game) {
    bool done = false;
    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            if (
                game->board[y][x].type == KING &&
                game->board[y][x].color == game->turn
            ) {
                Piece king = game->board[y][x];

                for (I8 ay = -2; ay <= 2; ay++) {
                    for (I8 ax = -1; ax <= 2; ax++) {
                        if (
                            ax >= 0 && ax < SIZE && ay >= 0 && ay < SIZE && 
                            game->board[y + ay][x + ax].color != king.color &&
                            game->board[y + ay][x + ax].color != BLANK && (
                                (
                                    game->board[y + ay][x + ax].type == KNIGHT &&
                                    abs(ax) != abs(ay) && ax != 0 && ay != 0
                                ) || (
                                    game->board[y + ay][x + ax].type == PAWN &&
                                    abs(ax) == 1 && ay == (king.color == WHITE ? -1 : 1)
                                ) || (
                                    game->board[y + ay][x + ax].type == KING &&
                                    abs(ax) == 1 && abs(ay) == 1
                                )
                            )
                        ) {
                            game->check = true;
                            break;
                        }
                    }
                    if (game->check) break;
                }
                
                if (game->check) break;
                
                I8 directions[][2] = {
                    {-1, -1}, {-1, 0}, {-1, 1},
                    {0, -1}, {0, 1},
                    {1, -1}, {1, 0}, {1, 1},
                };

                for (U8 d = 0; d < 8; d++) {
                    for (U8 i = 1; i < SIZE - 1; i++) {
                        I8 ax = directions[d][0] * i;
                        I8 ay = directions[d][1] * i;
                        if (game->board[y + ay][x + ax].color != BLANK) {
                            if (
                                ax >= 0 && ax < SIZE && ay >= 0 && ay < SIZE &&
                                game->board[y + ay][x + ax].color !=
                                king.color && (
                                    (
                                        abs(ax) == abs(ay) &&
                                        game->board[y + ay][x + ax].type == BISHOP
                                    ) || (
                                        abs(ax) != abs(ay) &&
                                        game->board[y + ay][x + ax].type == ROOK
                                    ) || game->board[y + ay][x + ax].type == QUEEN
                                )
                            ) game->check = true;
                            break;
                        }
                    }
                    if (game->check) break;
                }
                
                done = true;
                break;
            }
        }
        if (done) break;
    }
}


static void add_legal_move(Game *game, Move move) {
    if (
        move.end.x >= 0 && move.end.x < SIZE &&
        move.end.y >= 0 && move.end.y < SIZE &&
        game->board[move.start.y][move.start.x].color !=
        game->board[move.end.y][move.end.x].color &&
        game->board[move.start.y][move.start.x].color == game->turn
    ) {
        game->amount_of_legal_moves++;
        if (game->legal_moves_capacity < game->amount_of_legal_moves) {
            game->legal_moves_capacity *= 2;
            game->legal_moves = realloc(
                game->legal_moves, game->legal_moves_capacity * sizeof(Move)
            );
        }

        char letter_char = piece_letters[game->board[move.start.y][move.start.x].type];
        char *letter = malloc(letter_char == ' ' ? 0 : 1);
        if (letter_char != ' ') letter[0] = letter_char;
        bool takes = game->board[move.end.y][move.end.x].color != BLANK;
        snprintf(move.notation, sizeof(move.notation), "%s%s%c%d",
            letter,           // piece type
            takes ? "x" : "", // takes
            abc[move.end.x],  // end x
            move.end.y + 1    // end y
        );
        free(letter);

        game->legal_moves[game->amount_of_legal_moves - 1] = move;
    }
}


static void calculate_legal_moves(Game *game) {
    if (game->amount_of_legal_moves > 0) {
        free(game->legal_moves);
    }
    game->amount_of_legal_moves = 0;
    game->legal_moves_capacity = 2;
    game->legal_moves = malloc(
        game->legal_moves_capacity * sizeof(Move)
    );
    if (game->legal_moves == NULL) {
        fprintf(stderr, "Ou shiii 👀. Out of Memory");
        exit(1);
    }

    /* --- TODO LIST --- *\
    * 
    * En Passant
    * Promotion
    * No self-check
    * Castling
    * 
    \* --- ---- ---- --- */

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            Piece square = game->board[y][x];
            if (square.type > 0) {
                if (square.type == KING) {
                    for (I8 ay = -1; ay <= 1; ay++) {
                        for (I8 ax = -1; ax <= 1; ax++) {
                            if (!(ax == 0 && ay == 0)) {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
                        }
                    }
                } else if (square.type == KNIGHT) {
                    for (I8 ay = -2; ay <= 2; ay++) {
                        for (I8 ax = -2; ax <= 2; ax++) {
                            if (
                                abs(ax) != abs(ay) &&
                                ax != 0 && ay != 0
                            ) {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
                        }
                    }
                } else if (square.type == PAWN) {
                    U8 ay = (
                        square.color == WHITE ? 1 : -1
                    );

                    if (game->board[y + ay][x].color == BLANK) {
                        add_legal_move(game, (Move){
                            (P){x, y}, (P){x, y + ay}
                        });
                        
                        if (y == (square.color == WHITE ? 1 : 6)) {
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x, y + ay * 2}
                            });
                        }
                    }
                    for (I8 ax = -1; ax <= 1; ax += 2) {
                        if (
                            game->board[y + ay][x + ax].color != BLANK &&
                            game->board[y + ay][x + ax].color !=
                            square.color
                        ) {
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x + ax, y + ay}
                            });
                        }
                    }
                } else if (
                    square.type == ROOK ||
                    square.type == BISHOP ||
                    square.type == QUEEN
                ) {
                    const U8 amount_of_directions = (
                        square.type == QUEEN ? 8 : 4
                    );
                    static const I8 directions[][2] = {
                        {0, 1}, {0, -1}, {-1, 0}, {1, 0},
                        {1, 1}, {-1, -1}, {-1, 1}, {1, -1}
                    };
                    const U8 direction_start_index = (
                        square.type == BISHOP ? 4: 0
                    );
                    for (
                        U8 d = direction_start_index;
                        d < amount_of_directions + direction_start_index;
                        d++
                    ) {
                        for (U8 i = 1; i < SIZE - 1; i++) {
                            I8 ax = directions[d][0] * i;
                            I8 ay = directions[d][1] * i;
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x + ax, y + ay}
                            });
                            if (
                                game->board[y + ay][x + ax].color != BLANK
                            ) break;
                        }
                    }
                }
            }
        }
    }
}


static void raw_move(Game *game, Move move) {
    game->amount_of_moves++;

    if (game->amount_of_moves > game->moves_capacity) {
        game->moves_capacity *= 2;
        game->moves = realloc(
            game->moves,
            game->moves_capacity * sizeof(Move)
        );
    }

    if (!game->moved_king && game->board[move.start.y][move.start.x].type == KING) {
        game->moved_king = true;
    } else if (
        !game->moved_rook_l &&
        game->board[move.start.y][move.start.x].type == ROOK &&
        move.start.x == 0
    ) {
        game->moved_rook_l = true;
    } else if (
        !game->moved_rook_r &&
        game->board[move.start.y][move.start.x].type == ROOK &&
        move.start.x == 7
    ) {
        game->moved_rook_r = true;
    }

    game->moves[game->amount_of_moves - 1] = move;

    game->turn = game->turn == WHITE ? BLACK : WHITE;
    game->check = false;

    calculate_board(game);
    calculate_legal_moves(game);
    check_check(game);

    /* --- TODO LIST --- *\
    * 
    * Draw:
    *   50-move rule
    *   3-time repetion
    *   insufficient material:
    *   stalemate
    * 
    \* --- ---- ---- --- */
}


Game new_game() {
    Game game;
    
    game.amount_of_moves = 0;
    game.moves_capacity = 2;
    game.moves = malloc(
        game.moves_capacity * sizeof(Move)
    );
    game.amount_of_legal_moves = 0;
    game.legal_moves_capacity = 2;
    game.legal_moves = malloc(
        game.moves_capacity * sizeof(Move)
    );
    
    game.moved_king = false;
    game.moved_rook_r = false;
    game.moved_rook_l = false;
    game.check = false;
    game.checkmate = false;
    game.draw = false;
    game.turn = WHITE;

    calculate_board(&game);
    calculate_legal_moves(&game);

    return game;
}


bool do_move(Game *game, char *notation) {
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (strcmp(game->legal_moves[i].notation, notation) == 0) {

            raw_move(game, game->legal_moves[i]);
            
            return true;
            // i was here
        }
    }
    return false;
}