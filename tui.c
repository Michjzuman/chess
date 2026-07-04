#include "chess.h"

/////////////////////////////////////////
///[ tui.c ]/////////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const char abc[] = ABC;

static const char symbols[] = " PNBRQK";

static char *notation_line(const Game *game, U16 line_y) {
    if (game->amount_of_moves + 1 >= line_y * 2) {
        U16 y = (
            game->amount_of_moves <= 30 ? line_y :
            game->amount_of_moves / 2 - (15 - line_y) + (game->amount_of_moves % 2)
        );
        char *result = malloc(32);
        if (result == NULL) out_of_memory_err();
        snprintf(
            result, 32, "%d. %s %s                 ", y,
            game->moves[(y - 1) * 2].notation,
            game->amount_of_moves % 2 == 0 ||
            game->amount_of_moves + 1 > y * 2 ?
            game->moves[(y - 1) * 2 + 1].notation : ""
        );
        return result;
    } else {
        char *result = malloc(0);
        return result;
    }
}

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
        char *notation = notation_line(game, (SIZE - y) * 2 - 1);
        printf(
            "\033[90m│\033[0m %s%d\033[0m     %s",
            cursor.y == y && show_cursor ? "" : "\033[90m", y + 1,
            notation
        );
        free(notation);
        if (y > 0) {
            printf("\n\033[90m├");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┼");
            char *notation = notation_line(game, (SIZE - y) * 2);
            printf("───┤\033[0m       %s\n", notation);
            free(notation);
        } else {
            printf("\n\033[90m└");
            for (U8 x = 0; x < SIZE - 1; x++) printf("───┴");
            printf("───┘\n");
            for (U8 x = 0; x < SIZE; x++) printf(
                " %s%c  ",
                cursor.x == x && show_cursor ? "\033[0m" : "\033[90m",
                abc[x]
            );
        }
    }
    printf("\033[0m\n\n");
}

U8 draw_game_small(const Game *game) {
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
    return SIZE;
}

U8 draw_game(const Game *game) {
    draw_game_full(game, (P){0, 0}, false);
    return SIZE * 2 + 4;
}

U8 draw_game_testing(const Game *game) {
    draw_game_full(game, (P){0, 0}, false);
    return 0;
}

U8 draw_game_with_cursor(const Game *game, P cursor) {
    draw_game_full(game, cursor, true);
    return SIZE * 2 + 4;
}


U8 human(const Game *game) {
    return 0;
}