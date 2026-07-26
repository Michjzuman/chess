#ifndef TUI_H
#define TUI_H

#include <termios.h>

#include "chess.h"

///////////////////////////

U8 draw_game_small(const Game *);

U8 draw_game(const Game *);

U8 tui(const Game *);

U8 draw_game_with_cursor(const Game *, P cursor);

U8 draw_game_with_cursor_and_marks(
    const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks
);

U8 draw_game_with_cursor_and_marks_and_promotion_menu(
    const Game *game, P cursor, P *marks, P main_mark, U8 amount_of_marks, U8 promotion_cursor
);

// can be removed later
U8 draw_game_testing(const Game *);

///////////////////////////

U8 human(const Game *);

#endif