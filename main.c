#include "chess.h"
#include "tui.h"
#include "bots.h"

int main() {
    U32 count[3] = {0};
    while (true) {
        U8 winner = play(tui, human, jonkler);
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

