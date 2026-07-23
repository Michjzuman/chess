#include "chess.h"

// Random Moves Bot

U8 jonkler(const Game *game) {
    return rand() % game->amount_of_legal_moves;
}

U8 jonkler_with_delay(const Game *game) {
    usleep(300000);
    return jonkler(game);
}