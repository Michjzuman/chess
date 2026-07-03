#include "chess.h"

// Random Moves Bot

U8 jonkler(const Game *game) {
    return rand() % game->amount_of_legal_moves;
}