#ifndef BOTS_H
#define BOTS_H

#include "chess.h"

///////////////////////////

U8 jonkler(const Game *);

U8 thief(const Game *game);

U8 murderer(const Game *game);



// LLMs

U8 gemma4_e2b_mlx(const Game *);
U8 gemma4_12b_mlx(const Game *);

U8 gpt_5_4_mini(const Game *);
U8 gpt_5_4(const Game *);
U8 gpt_5_5(const Game *);
U8 gpt_5_6_Luna(const Game *);
U8 gpt_5_6_Terra(const Game *);
U8 gpt_5_6_Sol(const Game *);

///////////////////////////

#endif