#include "chess.h"

/////////////////////////////////////////
///[ tui.c ]/////////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const char abc[] = ABC;

static void draw_game_full(
    const Game *game,
    P cursor, bool show_cursor
) {
    Board board = calculate_board(game);

    printf("┌");
    for (U8 x = 0; x < SIZE - 1; x++) printf("───┬");
    printf("───┐\n");
    for (I8 y = SIZE - 1; y >= 0; y--) {
        for (U8 x = 0; x < SIZE; x++) {
            char symbol = ' ';
            printf("│ %c ", symbol);
        }
        printf("│ %d", y + 1);
        if (y > 0) {
            printf("\n├");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┼");
            printf("───┤\n");
        } else {
            printf("\n└");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┴");
            printf("───┘\n ");
            for (U8 x = 0; x < SIZE; x++) printf(" %c  ", abc[x]);
        }
    }
    printf("\n");
}

void draw_game(const Game *game) {
draw_game_full(game, (P){0, 0}, false);
}

void draw_game_with_cursor(const Game *game, P cursor) {
draw_game_full(game, cursor, true);
}


