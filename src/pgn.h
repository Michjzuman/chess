#ifndef PGN_H
#define PGN_H

#include "chess.h"

typedef struct {
    char *p1, *p2;
    U8 result;
    char *path;
} PGNArgs;

U0 create_pgn(const Game *, const PGNArgs *);

U8 play_with_pgn(VF visualize, PF p1, U0 *, PF p2, U0 *, PGNArgs *);
U8 play_full(VF visualize, PF p1, U0 *, PF p2, U0 *, bool pgn, PGNArgs *);

#endif