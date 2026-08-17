#ifndef BOTS_H
#define BOTS_H

#include "chess.h"

///////////////////////////

U8 jonkler(const Game *, U0 *);
U8 jonkler_with_delay(const Game *, U0 *);

U8 thief(const Game *, U0 *);

U8 murderer(const Game *, U0 *);

U8 codex(const Game *, U0 *model);

U8 ollama(const Game *, U0 *model);

U8 neural_network(const Game *, U0 *nn);

U8 stockfih(const Game *game);

///////////////////////////

#endif