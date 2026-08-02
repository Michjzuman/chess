#include "chess.h"
#include "tui.h"
#include "bots.h"
#include "nn.h"

int main() {
    srand(time(NULL));
    /*
    U32 count[3] = {0};
    while (true) {
        U8 winner = play(tui, human, human);
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
    */
    
    tournament();

    return 0;
}

