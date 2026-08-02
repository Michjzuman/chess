#ifndef TUI_H
#define TUI_H

#include <termios.h>

#include "chess.h"

///////////////////////////

U0 tui_small(const Game *, bool testing);

U0 tui(const Game *, bool testing);

U0 draw_game_with_cursor(const Game *, P cursor);

U0 draw_game_with_cursor_and_marks(
    const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks
);

U0 draw_game_with_cursor_and_marks_and_promotion_menu(
    const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks, U8 promotion_cursor
);

///////////////////////////

U8 human(const Game *, U0 *);

#endif