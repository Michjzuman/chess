#include "chess.h"

/////////////////////////////////////////
///[ chess.c ]///////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const char piece_letters[] = PIECE_LETTERS;
static const char abc[] = ABC;

U0 calculate_legal_moves(Game *game);

U0 raw_move(Game *game, Move move);

U0 out_of_memory_err() {
    fprintf(stderr, "Ou shiii 👀. Out of Memory");
    exit(1);
}

Piece xy(const Game *game, U8 x, U8 y) {
    TwoPieces loc = game->board[y][x/2];
    U8 piece_U8 = x % 2 == 0 ? loc.left : loc.right;
    return (PieceObject){.U8 = piece_U8}.Piece;
}

U0 set_xy(Game *game, U8 x, U8 y, Piece new) {
    TwoPieces *loc = &game->board[y][x/2];
    if (x % 2 == 0) {
        loc->left = (PieceObject){.Piece = new}.U8;
    } else {
        loc->right = (PieceObject){.Piece = new}.U8;
    }
}

U0 raw_move_minimal(Game *game, Move move) {
    game->amount_of_moves++;
    U16 capacity = 1;
    for (U8 i = 0; i < game->moves_capacity; i++) capacity *= 2;
    if (game->amount_of_moves > capacity) {
        game->moves_capacity += 1;
        game->moves = realloc(
            game->moves, capacity * 2 * MAX_MOVE_NOTATION_LEN * sizeof(char *)
        );
    }

    bool en_passant = (
        xy(game, move.start.x, move.start.y).type == PAWN &&
        move.start.x != move.end.x &&
        xy(game, move.end.x, move.end.y).type == EMPTY
    );

    bool is_king = xy(game, move.start.x, move.start.y).type == KING;
    bool short_castling = is_king && move.end.x - move.start.x == 2;
    bool long_castling = is_king && move.end.x - move.start.x == -2;

    U8 notation_len = strlen(move.notation);
    bool promotion = (
        move.notation[notation_len - 2] == '=' || (
            move.notation[notation_len - 3] == '=' && (
                move.notation[notation_len - 1] == '+' ||
                move.notation[notation_len - 1] == '#'
            )
        )
    );
    PieceType promoted_piece_type;
    switch (
        move.notation[notation_len - 2] == '=' ?
        move.notation[notation_len - 1] :
        move.notation[notation_len - 2]
    ) {
        case piece_letters[2]: promoted_piece_type = KNIGHT; break;
        case piece_letters[3]: promoted_piece_type = BISHOP; break;
        case piece_letters[4]: promoted_piece_type = ROOK; break;
        case piece_letters[5]: promoted_piece_type = QUEEN; break;
    }

    bool move_without_pawn_move_or_take = (
        xy(game, move.start.x, move.start.y).type != PAWN &&
        xy(game, move.end.x, move.end.y).type == EMPTY
    );

    game->en_passant_line_plus1 = (
        (
            abs(move.start.y - move.end.y) >= 2 &&
            xy(game, move.start.x, move.start.y).type == PAWN
        ) ? move.start.x + 1 : 0
    );

    set_xy(game, move.end.x, move.end.y, xy(game, move.start.x, move.start.y));
    set_xy(game, move.start.x, move.start.y, (Piece){.type = EMPTY});
    if (en_passant) {
        set_xy(game, move.end.x, move.start.y, (Piece){.type = EMPTY});
    }
    if (short_castling) {
        const U8 y = game->turn == WHITE ? 0 : 7;
        set_xy(game, 5, y, xy(game, 7, y));
        set_xy(game, 7, y, (Piece){.type = EMPTY});
    }
    if (long_castling) {
        const U8 y = game->turn == WHITE ? 0 : 7;
        set_xy(game, 3, y, xy(game, 7, y));
        set_xy(game, 0, y, (Piece){.type = EMPTY});
    }
    if (promotion) {
        set_xy(game, move.end.x, move.end.y, (Piece){
            .color = xy(game, move.end.x, move.end.y).color,
            .type = promoted_piece_type
        });
    }
    if (move_without_pawn_move_or_take) {
        game->moves_without_pawn_moves_or_takes++;
    } else {
        game->moves_without_pawn_moves_or_takes = 0;
    }
}

U0 is_check(Game *game) {
    game->check = false;
    bool done = false;
    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            if (
                xy(game, x, y).type == KING &&
                xy(game, x, y).color == game->turn
            ) {
                Piece king = xy(game, x, y);

                for (I8 ay = -2; ay <= 2; ay++) {
                    for (I8 ax = -2; ax <= 2; ax++) {
                        if (
                            x + ax >= 0 && x + ax < SIZE &&
                            y + ay >= 0 && y + ay < SIZE &&
                            xy(game, x + ax, y + ay).color != king.color &&
                            xy(game, x + ax, y + ay).type != EMPTY && (
                                (
                                    xy(game, x + ax, y + ay).type == KNIGHT &&
                                    abs(ax) != abs(ay) && ax != 0 && ay != 0
                                ) || (
                                    xy(game, x + ax, y + ay).type == PAWN &&
                                    abs(ax) == 1 && ay == (king.color == WHITE ? 1 : -1)
                                ) || (
                                    xy(game, x + ax, y + ay).type == KING &&
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
                    for (I8 i = 1; i < SIZE; i++) {
                        I8 ax = directions[d].x * i;
                        I8 ay = directions[d].y * i;
                        if (
                            x + ax >= 0 && x + ax < SIZE &&
                            y + ay >= 0 && y + ay < SIZE &&
                            xy(game, x + ax, y + ay).type != EMPTY
                        ) {
                            if (
                                xy(game, x + ax, y + ay).color !=
                                king.color && (
                                    (
                                        abs(ax) == abs(ay) &&
                                        xy(game, x + ax, y + ay).type == BISHOP
                                    ) || (
                                        abs(ax) != abs(ay) &&
                                        xy(game, x + ax, y + ay).type == ROOK
                                    ) || xy(game, x + ax, y + ay).type == QUEEN
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

U0 is_draw(Game *game) {
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
                xy(game, x, y).type != KING &&
                xy(game, x, y).type != EMPTY
            ) {
                only_kings = false;
            }
        }
    }

    if (only_kings) {
        game->draw = true;
        return;
    }

    if (game->moves_without_pawn_moves_or_takes >= 50) {
        game->draw = true;
        return;
    }

    // repetition detection

    Game test_game = new_game();

    TwoPieces (*positions)[SIZE][SIZE / 2] = malloc(
        game->amount_of_moves * sizeof(TwoPieces) * SIZE * (SIZE / 2)
    );
    if (positions == NULL) out_of_memory_err();

    for (U16 i1 = 0; i1 < game->amount_of_moves; i1++) {
        bool works = false;
        for (U8 i = 0; i < test_game.amount_of_legal_moves; i++) {
            if (strcmp(test_game.legal_moves[i].notation, game->moves[i1]) == 0) {
                raw_move_minimal(&test_game, test_game.legal_moves[i]);
                works = true;
                break;
            }
        }
        if (!works) {
            fprintf(stderr, "what!?!?!? %d: %s\n legal_moves:", i1, game->moves[i1]);
            for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
                fprintf(stderr, " %s", game->legal_moves[i].notation);
            }
            exit(1);
        }
        test_game.turn = test_game.turn == WHITE ? BLACK : WHITE;
        calculate_legal_moves(&test_game);
        is_check(&test_game);

        for (U8 y = 0; y < SIZE; y++) {
            for (U8 x = 0; x < SIZE / 2; x++) {
                positions[i1][y][x] = test_game.board[y][x];
            }
        }
        U8 count_sames = 0;
        for (U16 i2 = 0; i2 < i1; i2++) {
            bool same = true;
            for (U8 y = 0; y < SIZE; y++) {
                for (U8 x = 0; x < SIZE / 2; x++) {
                    if (
                        positions[i1][y][x].left != positions[i2][y][x].left ||
                        positions[i1][y][x].right != positions[i2][y][x].right
                    ) {
                        same = false;
                        break;
                    }
                }
                if (!same) break;
            }
            if (same) count_sames++;
            if (count_sames >= 2) {
                game->draw = true;
                return;
            }
        }
    }
    free(positions);
    close_game(&test_game);
}

Game copy_game(const Game *source) {
    Game copy = *source;

    copy.moves = malloc(copy.amount_of_moves * MAX_MOVE_NOTATION_LEN * sizeof(char *));
    if (copy.moves == NULL) out_of_memory_err();
    memcpy(copy.moves, source->moves, copy.amount_of_moves * MAX_MOVE_NOTATION_LEN * sizeof(char *));

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

U0 close_game(Game *game) {
    free(game->moves);
    free(game->legal_moves);
}

U0 add_legal_move(Game *game, Move move) {
    if (
        move.end.x >= 0 && move.end.x < SIZE &&
        move.end.y >= 0 && move.end.y < SIZE &&
        (
            xy(game, move.start.x, move.start.y).color !=
            xy(game, move.end.x, move.end.y).color ||
            xy(game, move.end.x, move.end.y).type == EMPTY
        ) &&
        xy(game, move.start.x, move.start.y).color == game->turn
    ) {
        Game test_game = copy_game(game);
        raw_move_minimal(&test_game, move);
        is_check(&test_game);
        bool self_check = test_game.check;
        test_game.turn = test_game.turn == WHITE ? BLACK : WHITE;
        is_check(&test_game);
        if (test_game.check) {
            //calculate_legal_moves(&test_game);
        }
        close_game(&test_game);

        if (!self_check) {
            if (
                xy(game, move.start.x, move.start.y).type == KING &&
                move.start.y == move.end.y
            ) {
                switch (move.start.x - move.end.x) {
                    case -1:
                        game->king_can_go_right = true;
                        break;
                    case 1:
                        game->king_can_go_left = true;
                        break;
                }
            }
            game->amount_of_legal_moves++;
            U8 capacity = 1;
            for (U8 i = 0; i < game->legal_moves_capacity; i++) capacity *= 2;
            if (capacity < game->amount_of_legal_moves) {
                game->legal_moves_capacity += 1;
                game->legal_moves = realloc(
                    game->legal_moves, capacity * 2 * sizeof(Move)
                );
            }
    
            if (move.notation[0] != 'O') {
                bool is_promotion = move.notation[0] == '=';
                char *promotion = malloc(is_promotion ? 2 : 0);
                if (is_promotion) strcpy(promotion, move.notation);
                char letter_char = piece_letters[xy(game, move.start.x, move.start.y).type];
                char *letter = malloc(letter_char == ' ' ? 0 : 1);
                if (letter == NULL) out_of_memory_err();
                
                if (letter_char != ' ') letter[0] = letter_char;
    
                bool takes = xy(game, move.end.x, move.end.y).type != EMPTY;
    
                bool show_start_x = (
                    (xy(game, move.start.x, move.start.y).type == PAWN && takes)
                    // duplicate notations
                );
                char *start_x = malloc(show_start_x ? 1 : 0);
                if (start_x == NULL) out_of_memory_err();
                if (show_start_x) start_x[0] = abc[move.start.x];
    
                snprintf(move.notation, sizeof(move.notation), "%s%s%s%c%d%s%s",
                    letter,           // piece type
                    start_x,          // start x
                    takes ? "x" : "", // takes
                    abc[move.end.x],  // end x
                    move.end.y + 1,   // end y
                    promotion,        // promotion
                    test_game.check ? (
                        /*test_game.amount_of_legal_moves == 0 ? "#" : */"+"
                    ) : ""            // check
                );
                free(letter);
                free(start_x);

                if (
                    xy(game, move.start.x, move.start.y).type == KNIGHT ||
                    xy(game, move.start.x, move.start.y).type == BISHOP ||
                    xy(game, move.start.x, move.start.y).type == ROOK ||
                    xy(game, move.start.x, move.start.y).type == QUEEN
                ) {
                    for (U8 i = 0; i < game->amount_of_legal_moves - 1; i++) {
                        if (strcmp(game->legal_moves[i].notation, move.notation) == 0) {
                            if (game->legal_moves[i].start.x != move.start.x) {
                                memmove(&move.notation[2], &move.notation[1], strlen(&move.notation[1]) + 1);
                                move.notation[1] = abc[move.start.x];

                                memmove(
                                    &game->legal_moves[i].notation[2],
                                    &game->legal_moves[i].notation[1],
                                    strlen(&game->legal_moves[i].notation[1]) + 1
                                );
                                game->legal_moves[i].notation[1] = abc[game->legal_moves[i].start.x];
                            } else if (game->legal_moves[i].start.y != move.start.y) {
                                memmove(&move.notation[2], &move.notation[1], strlen(&move.notation[1]) + 1);
                                move.notation[1] = (move.start.y + 1) + '0';

                                memmove(
                                    &game->legal_moves[i].notation[2],
                                    &game->legal_moves[i].notation[1],
                                    strlen(&game->legal_moves[i].notation[1]) + 1
                                );
                                game->legal_moves[i].notation[1] = (game->legal_moves[i].start.y + 1) + '0';
                            }
                            break;
                        }
                    }
                }
            }

            game->legal_moves[game->amount_of_legal_moves - 1] = move;
        }

    }
}

U0 calculate_legal_moves(Game *game) {
    if (game->amount_of_legal_moves > 0) {
        free(game->legal_moves);
    }
    game->amount_of_legal_moves = 0;
    game->legal_moves_capacity = 1;
    game->legal_moves = malloc(
        game->legal_moves_capacity * 2 * MAX_MOVE_NOTATION_LEN * sizeof(char *)
    );
    if (game->legal_moves == NULL) out_of_memory_err();

    game->king_can_go_right = false;
    game->king_can_go_left = false;

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            Piece square = xy(game, x, y);
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

                    if (xy(game, x, y + ay).type == EMPTY) {
                        if (y == (square.color == WHITE ? 6 : 1)) {
                            // Promotion
                            char notation[] = "= ";
                            for (U8 i = 2; i < 6; i++) {
                                notation[1] = piece_letters[i];
                                Move move = {
                                    (P){x, y}, (P){x, y + ay}
                                };
                                strcpy(move.notation, notation);
                                add_legal_move(game, move);
                            }
                        } else {
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x, y + ay}
                            });
                        }
                        
                        if (y == (square.color == WHITE ? 1 : 6)) {
                            if (xy(game, x, y + ay * 2).type == EMPTY) {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x, y + ay * 2}
                                });
                            }
                        }
                    }
                    if (y == (square.color == WHITE ? 4 : 3)) {
                        for (I8 ax = -1; ax <= 1; ax += 2) {
                            if (
                                game->en_passant_line_plus1 == x + ax + 1 &&
                                xy(game, x + ax, y).type != EMPTY &&
                                xy(game, x + ax, y).color != square.color
                            ) {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
                        }
                    }
                    for (I8 ax = -1; ax <= 1; ax += 2) {
                        if (
                            xy(game, x + ax, y + ay).type != EMPTY &&
                            xy(game, x + ax, y + ay).color != square.color
                        ) {
                            if (y == (square.color == WHITE ? 6 : 1)) {
                                // Promotion
                                char notation[] = "= ";
                                for (U8 i = 2; i < 6; i++) {
                                    notation[1] = piece_letters[i];
                                    Move move = {
                                        (P){x, y}, (P){x + ax, y + ay}
                                    };
                                    strcpy(move.notation, notation);
                                    add_legal_move(game, move);
                                }
                            } else {
                                add_legal_move(game, (Move){
                                    (P){x, y}, (P){x + ax, y + ay}
                                });
                            }
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
                        for (U8 i = 1; i < SIZE; i++) {
                            I8 ax = directions[d][0] * i;
                            I8 ay = directions[d][1] * i;
                            add_legal_move(game, (Move){
                                (P){x, y}, (P){x + ax, y + ay}
                            });
                            if (
                                xy(game, x + ax, y + ay).type != EMPTY
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
        xy(game, 4, y).type == KING &&
        xy(game, 4, y).color == game->turn && (
            (!game->moved_king_w && game->turn == WHITE) ||
            (!game->moved_king_b && game->turn == BLACK)
        ) && !game->check
    ) {
        if (
            xy(game, 5, y).type == EMPTY &&
            xy(game, 6, y).type == EMPTY &&
            xy(game, 7, y).type == ROOK &&
            xy(game, 7, y).color == game->turn && (
                (!game->moved_rook_r_w && game->turn == WHITE) ||
                (!game->moved_rook_r_b && game->turn == BLACK)
            ) &&
            game->king_can_go_right
        ) {
            add_legal_move(game, (Move){
                (P){4, y}, (P){6, y}, "O-O"
            });
        }
        if (
            xy(game, 3, y).type == EMPTY &&
            xy(game, 2, y).type == EMPTY &&
            xy(game, 1, y).type == EMPTY &&
            xy(game, 0, y).type == ROOK &&
            xy(game, 0, y).color == game->turn && (
                (!game->moved_rook_l_w && game->turn == WHITE) ||
                (!game->moved_rook_l_b && game->turn == BLACK)
            ) &&
            game->king_can_go_left
        ) {
            add_legal_move(game, (Move){
                (P){4, y}, (P){2, y}, "O-O-O"
            });
        }
    }
}

U0 raw_move(Game *game, Move move) {
    if (
        !(
            game->turn == WHITE ?
            game->moved_king_w : game->moved_king_b
        ) &&
        xy(game, move.start.x, move.start.y).type == KING
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
        xy(game, move.start.x, move.start.y).type == ROOK &&
        move.start.x == 0
    ) {
        if (game->turn == WHITE) {
            game->moved_rook_l_w = true;
        } else {
            game->moved_rook_l_b = true;
        }
    } else if (
        !(
            game->turn == WHITE ?
            game->moved_rook_r_w : game->moved_rook_r_b
        ) &&
        xy(game, move.start.x, move.start.y).type == ROOK &&
        move.start.x == 7
    ) {
        if (game->turn == WHITE) {
            game->moved_rook_r_w = true;
        } else {
            game->moved_rook_r_b = true;
        }
    }

    raw_move_minimal(game, move);
    
    for (U8 i = 0; i < MAX_MOVE_NOTATION_LEN; i++) {
        game->moves[game->amount_of_moves - 1][i] = move.notation[i];
    }
    
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
        game.moves_capacity * 2 * MAX_MOVE_NOTATION_LEN * sizeof(char *)
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
    game.en_passant_line_plus1 = 0;
    game.moves_without_pawn_moves_or_takes = 0;

    for (U8 y = 0; y < SIZE; y++) {
        for (U8 x = 0; x < SIZE; x++) {
            set_xy(&game, x, y,
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
                (Piece){.type = EMPTY}
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

U8 play(U8(*visualize)(const Game *), U8(*p1)(const Game *), U8(*p2)(const Game *)) {
    srand(time(NULL));

    Game game = new_game();

    const U8 height = visualize(&game);

    while (true) {
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
        printf("\n[");
        for (U8 i = 0; i < game.amount_of_legal_moves; i++) {
            printf(" %s", game.legal_moves[i].notation);
        } printf("]\n\n");
        */

        if (game.amount_of_legal_moves <= 0 && game.check) {
            close_game(&game);
            return game.turn == WHITE ? 2 : 1;
        }
        if (game.draw) {
            close_game(&game);
            return 0;
        }
    }
}

U8 bg(const Game *game) {
    return 0;
};


/* --- TODO LIST --- *\
* 
* Draw:
*   insufficient material
* 
* Move notation "#" for checkmate
* Resignation & Remis
* 
\* --- ---- ---- --- */

