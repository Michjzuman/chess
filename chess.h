#ifndef CHESS_H
#define CHESS_H

///////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

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

///////////////////////////

typedef uint8_t U8;
typedef uint16_t U16;
typedef uint32_t U32;
typedef uint64_t U64;

typedef int8_t I8;
typedef int16_t I16;
typedef int32_t I32;
typedef int64_t I64;

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

enum {BLANK, WHITE, BLACK};
typedef U8 Color;

typedef struct {
    PieceType type;
    Color color;
} Piece;

///////////////////////////

typedef struct {
    P start;
    P end;
    char notation[7];
} Move;

typedef struct {
    Move *moves;
    U16 amount_of_moves;
    U16 moves_capacity;

    Move *legal_moves;
    U8 amount_of_legal_moves;
    U8 legal_moves_capacity;
    
    bool moved_king[2];
    bool moved_rook_r[2];
    bool moved_rook_l[2];
    
    bool check;
    bool checkmate;
    bool draw;

    Color turn;

    Piece board[SIZE][SIZE];
} Game;

///////////////////////////

typedef struct {
    U8 *moves;
} CompactGame;

///////////////////////////

Game new_game();

Game copy_game(
    const Game *source
);

void is_check(Game *game);

bool do_move(
    Game *game,
    char *notation
);

Color play(
    U8(*visualize)(const Game *),
    U8(*p1)(const Game *),
    U8(*p2)(const Game *)
);

#endif

