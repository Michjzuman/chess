#ifndef TUI_H
#define TUI_H

#include "chess.h"

///////////////////////////

U8 draw_game_small(const Game *game);

U8 draw_game(const Game *game);

U8 draw_game_with_cursor(const Game *game, P cursor);

// can be removed later
U8 draw_game_testing(const Game *game);

///////////////////////////

U8 human(const Game *game);

#endif