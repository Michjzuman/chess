#include "chess.h"

/////////////////////////////////////////
///[ tui.c ]/////////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const char abc[] = ABC;

static const char symbols[] = " PNBRQK";

static void draw_game_full(
    const Game *game,
    P cursor, bool show_cursor
) {
    printf("\n\033[90m┌");
    for (U8 x = 0; x < SIZE - 1; x++) printf("───┬");
    printf("───┐\033[0m\n");
    for (I8 y = SIZE - 1; y >= 0; y--) {
        for (U8 x = 0; x < SIZE; x++) {
            printf(
                "\033[90m│\033[0m%c%s%c\033[0m%c",
                cursor.x == x && cursor.y == y && show_cursor ? '[' : ' ',
                game->board[y][x].color == WHITE ? "\033[32m" : "\033[34m",
                symbols[game->board[y][x].type],
                cursor.x == x && cursor.y == y && show_cursor ? ']' : ' '
            );
        }
        printf(
            "\033[90m│\033[0m %s%d\033[0m",
            cursor.y == y && show_cursor ? "" : "\033[90m", y + 1
        );
        if (y > 0) {
            printf("\n\033[90m├");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┼");
            printf("───┤\033[0m\n");
        } else {
            printf("\n\033[90m└");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┴");
            printf("───┘\n ");
            for (U8 x = 0; x < SIZE; x++) printf(" %s%c\033[90m  ",
                cursor.x == x && show_cursor ? "\033[0m" : "",
                abc[x]
            );
        }
    }
    printf("\033[0m\n\n");
}

void draw_game_small(const Game *game) {
    for (I8 y = SIZE - 1; y >= 0; y--) {
        for (U8 x = 0; x < SIZE; x++) {
            printf(
                "%s%c\033[0m ",
                game->board[y][x].color == WHITE ? "\033[32m" :
                game->board[y][x].color == BLACK ? "\033[34m" : "\033[90m",
                symbols[game->board[y][x].type] == ' ' ? '.' :
                symbols[game->board[y][x].type]
            );
        }
        printf("\n");
    }
}

void draw_game(const Game *game) {
    draw_game_full(game, (P){0, 0}, false);
}

void draw_game_with_cursor(const Game *game, P cursor) {
    draw_game_full(game, cursor, true);
}


