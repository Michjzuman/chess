#ifndef CHESS_H
#define CHESS_H

///////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "types.h"

///////////////////////////

#define SIZE 8
#define PIECE_VALUES { \
    0, 1, 3, 3, 5, 9, 0 \
}
#define ABC "abcdefgh"
#define PIECE_LETTERS \
    "  NBRQK"
#define PROMOTION_PIECES { \
    2, 3, 4, 5 \
}
#define MAX_MOVE_NOTATION_LEN 10

///////////////////////////

typedef struct {
    I8 x: 4;
    I8 y: 4;
} P;

///////////////////////////

enum {
    EMPTY,
    PAWN, KNIGHT, BISHOP,
    ROOK, QUEEN, KING
};
typedef U8 PieceType;

enum {WHITE, BLACK};
typedef U8 Color;

typedef struct {
    PieceType type: 3;
    Color color: 1;
} Piece;

typedef union {
    Piece Piece;
    U8 U8;
} PieceObject;

typedef struct {
    U8 left: 4;
    U8 right: 4;
} TwoPieces;

///////////////////////////

typedef struct {
    P start;
    P end;
    char notation[
        MAX_MOVE_NOTATION_LEN
    ];
} Move;

typedef struct {
    char (*moves)[
        MAX_MOVE_NOTATION_LEN
    ];
    U16 amount_of_moves;
    U8 moves_capacity: 4;

    Move *legal_moves;
    U8 amount_of_legal_moves;
    U8 legal_moves_capacity: 3;
    
    bool moved_king_w: 1;
    bool moved_rook_r_w: 1;
    bool moved_rook_l_w: 1;
    
    bool moved_king_b: 1;
    bool moved_rook_r_b: 1;
    bool moved_rook_l_b: 1;

    bool king_can_go_left: 1;
    bool king_can_go_right: 1;

    U8 moves_without_pawn_moves_or_takes: 6;

    U8 en_passant_line_plus1: 4;
    
    bool check: 1;
    bool draw: 1;

    Color turn: 1;

    TwoPieces board[SIZE][SIZE / 2];
} Game;

Piece xy(const Game *game, U8 x, U8 y);

U0 set_xy(Game *game, U8 x, U8 y, Piece new);

///////////////////////////

typedef struct {
    U8 *moves;
} CompactGame;

///////////////////////////

U0 out_of_memory_err();

///////////////////////////

Game new_game();

Game copy_game(
    const Game *source
);

U0 close_game(Game *game);

U0 is_check(Game *game);

bool do_move(
    Game *game,
    char *notation
);

U8 play(
    U0(*visualize)(const Game *, bool),
    U8(*p1)(const Game *),
    U8(*p2)(const Game *)
);

U0 bg(const Game *game, bool testing);

///////////////////////////

U0 out_of_memory_err();

#endif

