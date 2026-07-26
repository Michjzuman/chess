#include "chess.h"

U8 jonkler(const Game *game) {
    return rand() % game->amount_of_legal_moves;
}

U8 jonkler_with_delay(const Game *game) {
    usleep(300000);
    return jonkler(game);
}

U8 thief(const Game *game) {
    U8 best = 0;
    U8 result = jonkler(game);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        char *x = strchr(game->legal_moves[i].notation, 'x');
        if (x != NULL) {
            U8 value = xy(game, game->legal_moves[i].end.x, game->legal_moves[i].end.y).type;
            if (value > best) {
                best = value;
                result = i;
            }
        }
    }
    return result;
}

U8 murderer(const Game *game) {
    U8 result = jonkler(game);
    for (U8 i = 0; i < game->amount_of_legal_moves; i++) {
        if (
            strchr(game->legal_moves[i].notation, '+') ||
            strchr(game->legal_moves[i].notation, '#')
        ) {
            return i;
        }
    }
    return result;
}