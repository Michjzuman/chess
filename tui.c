#include "chess.h"
#include "tui.h"

/////////////////////////////////////////
///[ tui.c ]/////////////////////////////
////////////////[ Author: Michjzuman ]///
/////////////////////////////////////////

static const char abc[] = ABC;

static const char piece_letters[] = PIECE_LETTERS;

static const char symbols[] = " PNBRQK";

static U8 max_notation_len = 0;

static P start_cursor[2] = {{4, 1}, {4, 6}};

static char *notation_line(const Game *game, U16 line_y) {
    if (game->amount_of_moves + 1 >= line_y * 2) {
        U16 y = (
            game->amount_of_moves <= 30 ? line_y :
            game->amount_of_moves / 2 - (15 - line_y) + (game->amount_of_moves % 2)
        );
        char *result = malloc(32);
        if (result == NULL) out_of_memory_err();
        snprintf(
            result, 32, "%d. %s %s", y,
            game->moves[(y - 1) * 2],
            game->amount_of_moves % 2 == 0 ||
            game->amount_of_moves + 1 > y * 2 ?
            game->moves[(y - 1) * 2 + 1] : ""
        );
        if (strlen(result) > max_notation_len) {
            max_notation_len = strlen(result);
        }
        U8 spaces_len = max_notation_len - strlen(result);
        char *spaces = malloc(spaces_len);
        for (U8 i = 0; i < spaces_len; i++) spaces[i] = ' ';
        snprintf(
            result, 32, "%s%s", result, spaces
        );
        return result;
    } else {
        char *result = malloc(0);
        return result;
    }
}

static struct termios original_terminal;

static U8 draw_game_full(
    const Game *game,
    P cursor, bool show_cursor,
    P *marks, U8 amount_of_marks, P main_mark, bool show_marks,
    U8 promotion_cursor, bool show_promotion_menu,
    bool testing
) {
    const U8 size = testing ? 0 : SIZE * 2 + 4;
    if (size > 0) printf("\033[%dF", size);
    printf("\n\033[90m┌");
    for (U8 x = 0; x < SIZE - 1; x++) printf("───┬");
    printf("───┐\033[0m\n");
    for (I8 y = SIZE - 1; y >= 0; y--) {
        for (U8 x = 0; x < SIZE; x++) {
            bool marked = false;
            for (U8 i = 0; i < amount_of_marks; i++) {
                if (marks[i].x == x && marks[i].y == y) {
                    marked = true;
                    break;
                }
            }
            bool bold = ((
                xy(game, x, y).type != EMPTY &&
                cursor.x == x && cursor.y == y &&
                xy(game, x, y).color == game->turn
            ) || (
                show_marks && main_mark.x == x && main_mark.y == y &&
                marked
            )) && show_cursor;
            bool star = (
                marked && symbols[xy(game, x, y).type] == ' '
            );
            bool is_cursor = ((
                show_cursor && cursor.x == x && cursor.y == y
            ) || (
                show_marks && main_mark.x == x && main_mark.y == y
            ));
            printf(
                "\033[90m│\033[0m%s%s%s%c%s%c%s%s%c",
                show_marks ? "\033[90m" : "",
                bold ? "\033[1m" : "",
                marked ? "\033[31m" : "",
                is_cursor ? '[' : ' ',
                marked ? "" : (
                    xy(game, x, y).color == WHITE ? "\033[32m" : "\033[34m"
                ),
                star ? '*' : symbols[xy(game, x, y).type],
                bold ? "\033[1m" : "",
                marked ? "" : (show_marks ? "\033[90m" : "\033[0m"),
                is_cursor ? ']' : ' '
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
            printf("───┘\n ");
            for (U8 x = 0; x < SIZE; x++) printf(
                " %s%c  ",
                cursor.x == x && show_cursor ? "\033[0m" : "\033[90m",
                abc[x]
            );
        }
    }
    printf("\033[0m\n\n");
    if (show_promotion_menu) {
        char *color = game->turn == WHITE ? "\033[32m" : "\033[34m";
        printf(" %s", color);
        for (U8 i = 2; i < 6; i++) {
            bool is_cursor = (
                promotion_cursor + 2 == i
            );
            printf("%s%s%c%s%s",
                is_cursor ? "\033[0m[" : " ",
                is_cursor ? color : "",
                symbols[i],
                is_cursor ? "\033[0m]" : " ",
                is_cursor ? color : ""
            );
        }
    } else {
        printf("             \033[1F\n");
    }
    /*
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        printf("[%s]", game->legal_moves[i].notation);
    }
    */
    return size;
}


U0 restore_terminal() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_terminal);
}

U0 enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_terminal);
    atexit(restore_terminal);
    struct termios raw = original_terminal;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}


U0 tui_small(const Game *game, bool testing) {
    for (I8 y = SIZE - 1; y >= 0; y--) {
        for (U8 x = 0; x < SIZE; x++) {
            printf(
                "%s%c\033[0m ",
                xy(game, x, y).type == EMPTY ? "\033[90m" :
                xy(game, x, y).color == WHITE ? "\033[32m" : "\033[34m",
                symbols[xy(game, x, y).type] == ' ' ? '.' :
                symbols[xy(game, x, y).type]
            );
        }
        printf("\n");
    }
}

U0 tui(const Game *game, bool testing) {
    draw_game_full(game, (P){0, 0}, false, NULL, 0, (P){6, 7}, false, 0, false, testing);
}

U0 draw_game_with_cursor(const Game *game, P cursor) {
    draw_game_full(game, cursor, true, NULL, 0, (P){6, 7}, false, 0, false, false);
}

U0 draw_game_with_cursor_and_marks(const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks) {
    draw_game_full(game, cursor, true, marks, amount_of_marks, main_mark, true, 0, false, false);
}

U0 draw_game_with_cursor_and_marks_and_promotion_menu(const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks, U8 promotion_cursor) {
    draw_game_full(game, cursor, true, marks, amount_of_marks, main_mark, true, promotion_cursor, true, false);
}


U8 human(const Game *game) {
    P cursor = start_cursor[game->turn];
    bool selected = false;
    bool promotion_menu = false;
    U8 promotion_cursor = 3;
    draw_game_with_cursor(game, cursor);
    P *marks;
    P main_mark;
    U8 amount_of_marks;
    U8 move;
    enable_raw_mode();
    while (true) {
        if (selected) {
            if (promotion_menu) {
                draw_game_with_cursor_and_marks_and_promotion_menu(
                    game, cursor, marks, main_mark, amount_of_marks, promotion_cursor
                );
            } else {
                draw_game_with_cursor_and_marks(
                    game, cursor, marks, main_mark, amount_of_marks
                );
            }
        } else {
            draw_game_with_cursor(game, cursor);
        }
        int key = getchar();
        if (key == '\r' || key == '\n' || key == ' ') {
            if (selected) {
                bool legal = false;
                bool just_set_promotion_menu = false;
                for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
                    char *notation = game->legal_moves[i].notation;
                    U8 notation_len = strlen(notation);
                    if (
                        game->legal_moves[i].start.x == cursor.x &&
                        game->legal_moves[i].start.y == cursor.y &&
                        game->legal_moves[i].end.x == main_mark.x &&
                        game->legal_moves[i].end.y == main_mark.y && (
                            (
                                notation[notation_len - 2] == '=' &&
                                notation[notation_len - 1] == piece_letters[promotion_cursor + 2]
                            ) || (
                                notation[notation_len - 3] == '=' &&
                                notation[notation_len - 2] == piece_letters[promotion_cursor + 2]
                            ) || !promotion_menu
                        )
                    ) {
                        move = i;
                        legal = true;
                        if (
                            ((game->turn == WHITE && game->legal_moves[i].end.y == SIZE - 1) ||
                            (game->turn == BLACK && game->legal_moves[i].end.y == 0)) &&
                            xy(game, game->legal_moves[i].start.x, game->legal_moves[i].start.y).type == PAWN
                        ) {
                            if (!promotion_menu) {
                                just_set_promotion_menu = true;
                            }
                            promotion_menu = true;
                        }
                        break;
                    }
                }
                if (!just_set_promotion_menu && legal) {
                    start_cursor[game->turn] = main_mark;
                    return move;
                }
            } else if (
                xy(game, cursor.x, cursor.y).color == game->turn &&
                xy(game, cursor.x, cursor.y).type != EMPTY
            ) {
                marks = malloc(0);
                amount_of_marks = 0;
                for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
                    if (
                        game->legal_moves[i].start.x == cursor.x &&
                        game->legal_moves[i].start.y == cursor.y
                    ) {
                        if (amount_of_marks == 0) move = i;
                        amount_of_marks++;
                        marks = realloc(marks, amount_of_marks * sizeof(P));
                        marks[amount_of_marks - 1] = game->legal_moves[i].end;
                    }
                }
                main_mark = cursor;
                if (amount_of_marks == 0) {
                    free(marks);
                } else {
                    selected = true;
                }
            }
        }
        if (key == 27) {
            int second = getchar();
            if (selected && second == 27) {
                free(marks);
                selected = false;
                promotion_menu = 0;
            }
            if (second == '[') {
                int third = getchar();
                if (selected) {
                    if (promotion_menu) {
                        switch (third) {
                            case 'C':
                                if (promotion_cursor < 3) promotion_cursor++;
                                break;
                            case 'D':
                                if (promotion_cursor > 0) promotion_cursor--;
                                break;
                        }
                    } else {
                        if (
                            third == 'A' || third == 'B' ||
                            third == 'C' || third == 'D'
                        ) {
                            switch (third) {
                                case 'A':
                                    if (main_mark.y < SIZE - 1) main_mark.y++;
                                    break;
                                case 'B':
                                    if (main_mark.y > 0) main_mark.y--;
                                    break;
                                case 'C':
                                    if (main_mark.x < SIZE - 1) main_mark.x++;
                                    break;
                                case 'D':
                                    if (main_mark.x > 0) main_mark.x--;
                                    break;
                            }
                        }
                    }
                } else {
                    switch (third) {
                        case 'A':
                            if (cursor.y < SIZE - 1) cursor.y++;
                            break;
                        case 'B':
                            if (cursor.y > 0) cursor.y--;
                            break;
                        case 'C':
                            if (cursor.x < SIZE - 1) cursor.x++;
                            break;
                        case 'D':
                            if (cursor.x > 0) cursor.x--;
                            break;
                    }
                }
            }
        }
    }
    return 0;
}

