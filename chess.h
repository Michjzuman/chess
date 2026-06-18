#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

///////////////////////////

#define SIZE 8
#define PIECE_VALUES { \
    0, 1, 3, 3, 5, 9, 0 \
}
#define ABC "abcdefgh"
#define PIECE_LETTERS "  NBRQK"

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
    U8 x;
    U8 y;
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
    Piece squares[SIZE][SIZE];
} Board;

///////////////////////////

typedef struct {
    P start;
    P end;
} Move;

typedef struct {
    Move *moves;
    U16 amount_of_moves;
    U16 moves_capacity;

    Move *possible_moves;
    U16 amount_of_possible_moves;
    U16 possible_moves_capacity;
    
    bool check;
    bool moved_king;
    bool moved_rook_r;
    bool moved_rook_l;
    Color turn;

    Board board;
} Game;

///////////////////////////

typedef struct {
    U8 *moves;
} CompactGame;

///////////////////////////

void draw_game_small(const Game *game);

void draw_game(const Game *game);

void draw_game_with_cursor(
    const Game *game, P cursor
);

///////////////////////////

Game new_game();

void calculate_possible_moves(Game *game);

Board calculate_board(const Game *game);

void do_move(Game *game, Move move);



