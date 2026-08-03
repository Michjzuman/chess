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
    
    tournament();
    */

    //NN nn = new_chess_nn();
    //save_nn(&nn, "./brains/test_brain.nn");
    /*
    NN nn = open_nn("./brains/test_brain.nn");
    play(tui, thief, NULL, neural_network, &nn);
    close_nn(&nn);
    */

    tournament();

    return 0;
}

