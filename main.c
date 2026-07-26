#include "chess.h"
#include "tui.h"
#include "bots.h"

struct {char *name; U8(*function)(const Game *);} bots[] = {
    {"jonkler", jonkler},
    {"thief", thief},
    {"murderer", murderer},

    {"gemma4:e2b", gemma4_e2b_mlx},
    {"gemma4:12b", gemma4_12b_mlx},

    {"gpt-5.4", gpt_5_4_mini},
    {"gpt-5.5", gpt_5_5}
};

int main() {
    U8 count[3] = {0};
    while (true) {
        U8 winner = play(tui, gemma4_e2b_mlx, gpt_5_4_mini);
        count[winner]++;
        printf(
            "+-----------\n"
            "| draw:  %d\n"
            "| green: %d\n"
            "| blue:  %d\n"
            "+-----------\n",
            count[0], count[1], count[2]
        );
    }
    return 0;
}

