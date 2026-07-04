#include "chess.h"

/////////////////////////////////////////
///[ chess.c ]///////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const U8 piece_values[] = PIECE_VALUES;
static const char piece_letters[] = PIECE_LETTERS;
static const char abc[] = ABC;

static void calculate_legal_moves(Game *game);

void out_of_memory_err() {
    fprintf(stderr, "Ou shiii 👀. Out of Memory");
    exit(1);
}

static void raw_move_minimal(Game *game, Move move) {
    game->amount_of_moves++;
    U16 capacity = 1;
    for (U8 i = 0; i < game->moves_capacity; i++) capacity *= 2;
    if (game->amount_of_moves > capacity) {
        game->moves_capacity += 1;
        game->moves = realloc(
            game->moves, capacity * 2 * sizeof(Move)
        );
    }

    // this is where castling will be

    game->board[move.end.y][move.end.x] = game->board[move.start.y][move.start.x];
    game->board[move.start.y][move.start.x] = (Piece){EMPTY, BLANK};
}

void is_check(Game *game) {
    game->check = false;
    bool done = false;
    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            if (
                game->board[y][x].type == KING &&
                game->board[y][x].color == game->turn
            ) {
                Piece king = game->board[y][x];

                for (I8 ay = -2; ay <= 2; ay++) {
                    for (I8 ax = -2; ax <= 2; ax++) {
                        if (
                            x + ax >= 0 && x + ax < SIZE &&
                            y + ay >= 0 && y + ay < SIZE &&
                            game->board[y + ay][x + ax].color != king.color &&
                            game->board[y + ay][x + ax].color != BLANK && (
                                (
                                    game->board[y + ay][x + ax].type == KNIGHT &&
                                    abs(ax) != abs(ay) && ax != 0 && ay != 0
                                ) || (
                                    game->board[y + ay][x + ax].type == PAWN &&
                                    abs(ax) == 1 && ay == (king.color == WHITE ? 1 : -1)
                                ) || (
                                    game->board[y + ay][x + ax].type == KING &&
                                    abs(ax) <= 1 && abs(ay) <= 1
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
                
                P directions[] = {
                    (P){-1, -1}, (P){-1, 0}, (P){-1, 1},
                    (P){0, -1},  /* ---- */  (P){0, 1},
                    (P){1, -1},  (P){1, 0},  (P){1, 1}
                };

                for (U8 d = 0; d < sizeof(directions) / sizeof(P); d++) {
                    for (I8 i = 1; i < SIZE - 1; i++) {
                        I8 ax = directions[d].x * i;
                        I8 ay = directions[d].y * i;
                        if (
                            x + ax >= 0 && x + ax < SIZE &&
                            y + ay >= 0 && y + ay < SIZE &&
                            game->board[y + ay][x + ax].color != BLANK
                        ) {
                            if (
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

void is_draw(Game *game) {
    if (
        game->amount_of_legal_moves <= 0 &&
        !game->check
    ) {
        game->draw = true;
        return;
    }

    bool only_kings = true;
    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            if (
                game->board[y][x].type != KING &&
                game->board[y][x].type != EMPTY
            ) {
                only_kings = false;
            }
        }
    }

    if (only_kings) {
        game->draw = true;
        return;
    }
}

Game copy_game(const Game *source) {
    Game copy = *source;

    copy.moves = malloc(copy.amount_of_moves * sizeof(Move));
    if (copy.moves == NULL) out_of_memory_err();
    memcpy(copy.moves, source->moves, copy.amount_of_moves * sizeof(Move));

    copy.legal_moves = malloc(copy.amount_of_legal_moves * sizeof(Move));
    if (copy.legal_moves == NULL) out_of_memory_err();
    memcpy(copy.legal_moves, source->legal_moves, copy.amount_of_legal_moves * sizeof(Move));

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            copy.board[y][x] = source->board[y][x];
        }
    }

    return copy;
}

void close_game(Game *game) {
    free(game->moves);
    free(game->legal_moves);
}

static void add_legal_move(Game *game, Move move) {
    if (
        move.end.x >= 0 && move.end.x < SIZE &&
        move.end.y >= 0 && move.end.y < SIZE &&
        game->board[move.start.y][move.start.x].color !=
        game->board[move.end.y][move.end.x].color &&
        game->board[move.start.y][move.start.x].color == game->turn
    ) {
        Game test_game = copy_game(game);
        raw_move_minimal(&test_game, move);
        is_check(&test_game);
        bool self_check = test_game.check;
        test_game.turn = test_game.turn == WHITE ? BLACK : WHITE;
        is_check(&test_game);
        /*
        if (test_game.check) {
            calculate_legal_moves(&test_game);
        }
        */
        close_game(&test_game);

        if (!self_check) {
            game->amount_of_legal_moves++;
            U8 capacity = 1;
            for (U8 i = 0; i < game->legal_moves_capacity; i++) capacity *= 2;
            if (capacity < game->amount_of_legal_moves) {
                game->legal_moves_capacity += 1;
                game->legal_moves = realloc(
                    game->legal_moves, capacity * 2 * sizeof(Move)
                );
            }
    
            char letter_char = piece_letters[game->board[move.start.y][move.start.x].type];
            char *letter = malloc(letter_char == ' ' ? 0 : 1);
            if (letter == NULL) out_of_memory_err();
            
            if (letter_char != ' ') letter[0] = letter_char;

            bool takes = game->board[move.end.y][move.end.x].color != BLANK;

            bool show_start_x = (
                (game->board[move.start.y][move.start.x].type == PAWN && takes)
                // duplicate notations
            );
            char *start_x = malloc(show_start_x ? 1 : 0);
            if (start_x == NULL) out_of_memory_err();
            if (show_start_x) start_x[0] = abc[move.start.x];

            snprintf(move.notation, sizeof(move.notation), "%s%s%s%s%c%d%s%s",
                letter,                     // piece type
                start_x,                    // start x
                "",                         // start y
                takes ? "x" : "",           // takes
                abc[move.end.x],            // end x
                move.end.y + 1,             // end y
                "",                         // pawn promotion
                test_game.check && test_game.amount_of_legal_moves <= 0 ? "#" : // checkmate
                test_game.check ? "+" : ""  // check
            );
            free(letter);
            free(start_x);
    
            game->legal_moves[game->amount_of_legal_moves - 1] = move;
        }

    }
}

static void calculate_legal_moves(Game *game) {
    if (game->amount_of_legal_moves > 0) {
        free(game->legal_moves);
    }
    game->amount_of_legal_moves = 0;
    game->legal_moves_capacity = 1;
    game->legal_moves = malloc(
        game->legal_moves_capacity * 2 * sizeof(Move)
    );
    if (game->legal_moves == NULL) out_of_memory_err();

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
                    I8 ay = (
                        square.color == WHITE ? 1 : -1
                    );

                    if (game->board[y + ay][x].color == BLANK) {
                        if (y == (square.color == WHITE ? 6 : 1)) {
                            // Promotion

                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x, y + ay}
                            });

                        } else {
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x, y + ay}
                            });
                        }
                        
                        if (y == (square.color == WHITE ? 1 : 6)) {
                            if (game->board[y + ay * 2][x].color == BLANK) {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x, y + ay * 2}
                                });
                            }
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

    // Castling
    const U8 y = game->turn == WHITE ? 0 : 7;
    if (
        game->board[y][4].type == KING &&
        game->board[y][4].color == game->turn &&
        game->board[y][5].type == EMPTY &&
        game->board[y][6].type == EMPTY &&
        game->board[y][7].type == ROOK &&
        game->board[y][7].color != game->turn
    ) {
        
    }
}

void raw_move(Game *game, Move move) {
    raw_move_minimal(game, move);

    if (
        !(
            game->turn == WHITE ?
            game->moved_king_w : game->moved_king_b
        ) &&
        game->board[move.start.y][move.start.x].type == KING
    ) {
        if (game->turn == WHITE) {
            game->moved_king_w = true;
        } else {
            game->moved_king_b = true;
        }
    } else if (
        !(
            game->turn == WHITE ?
            game->moved_rook_l_w : game->moved_rook_l_b
        ) &&
        game->board[move.start.y][move.start.x].type == ROOK &&
        move.start.x == 0
    ) {
        if (game->turn == WHITE) {
            game->moved_rook_l_w = true;
        } else {
            game->moved_rook_l_w = true;
        }
    } else if (
        !(
            game->turn == WHITE ?
            game->moved_rook_r_w : game->moved_rook_r_b
        ) &&
        game->board[move.start.y][move.start.x].type == ROOK &&
        move.start.x == 7
    ) {
        if (game->turn == WHITE) {
            game->moved_rook_r_w = true;
        } else {
            game->moved_rook_r_w = true;
        }
    }

    game->moves[game->amount_of_moves - 1] = move;

    game->turn = game->turn == WHITE ? BLACK : WHITE;

    calculate_legal_moves(game);
    is_check(game);
    is_draw(game);
}

Game new_game() {
    Game game;
    
    game.amount_of_moves = 0;
    game.moves_capacity = 1;
    game.moves = malloc(
        game.moves_capacity * 2 * sizeof(Move)
    );
    if (game.moves == NULL) out_of_memory_err();
    game.amount_of_legal_moves = 0;
    game.legal_moves_capacity = 1;
    game.legal_moves = malloc(
        game.legal_moves_capacity * 2 * sizeof(Move)
    );
    if (game.legal_moves == NULL) out_of_memory_err();
    
    game.moved_king_w = false;
    game.moved_rook_r_w = false;
    game.moved_rook_l_w = false;
    game.moved_king_b = false;
    game.moved_rook_r_b = false;
    game.moved_rook_l_b = false;
    game.check = false;
    game.draw = false;
    game.turn = WHITE;

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            game.board[y][x] = (
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

    calculate_legal_moves(&game);

    return game;
}

bool do_move(Game *game, char *notation) {
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (strcmp(game->legal_moves[i].notation, notation) == 0) {
            raw_move(game, game->legal_moves[i]);
            return true;
        }
    }
    return false;
}

Color play(U8(*visualize)(const Game *), U8(*p1)(const Game *), U8(*p2)(const Game *)) {
    srand(time(NULL));

    Game game = new_game();

    const U8 height = visualize(&game);

    while (true) {
        usleep(10000);

        U8 (*player)(const Game *game) = game.turn == WHITE ? p1 : p2;
        
        char *notation = game.legal_moves[player(&game)].notation;

        bool legal = do_move(&game, notation);

        if (!legal) {
            free(game.moves);
            free(game.legal_moves);
            return 0;
        };
        
        if (height > 0) printf("\033[%dF", height);
        visualize(&game);

        /*
        for (U8 i = 0; i < game.amount_of_legal_moves; i++) {
            printf("%s ", game.legal_moves[i].notation);
        }
        printf("\n");
        */

        if (game.amount_of_legal_moves <= 0 && game.check) {
            close_game(&game);
            return game.turn == WHITE ? BLACK : WHITE;
        }
        if (game.draw) {
            close_game(&game);
            return BLANK;
        }
    }
}


/* --- TODO LIST --- *\
* 
* Draw:
*   50-move rule
*   3-time repetion
*   insufficient material
* 
* Promotion
* Castling
* En Passant
* Resignation & Remis
* 
\* --- ---- ---- --- */

