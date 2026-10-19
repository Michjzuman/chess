#include "chess.h"
#include "nn.h"

#include "tui.h"
#include "bots.h"
#include <math.h>

#define STOCKFIH_PATH "./brains/stockfih.nn"

// this is in development

U0 educate_nn(NN *nn, double loss) {
    
}

NN *stockfih_birth(U0) {
    if (access(STOCKFIH_PATH, F_OK) == 0) {
        return open_nn(STOCKFIH_PATH);
    } else {
        NN *r = malloc(sizeof(NN));
        *r = new_chess_nn();
        save_nn(r, STOCKFIH_PATH);
        return r;
    }
    return NULL;
}

double get_loss() {
    return 6.7f;
}

U0 draw_nn_outputs(NN *nn, Game *game) {
    float *answer = ask_nn(nn, get_chess_nn_inputs(game, nn));
    for (U8 i = 0; i < 2; i++) {
        printf("----------------\n");
        for (I8 y = SIZE - 1; y >= 0; y--) {
            for (U8 x = 0; x < SIZE; x++) {
                float v = answer[i * SIZE * SIZE + y * SIZE + x] * 0.1;
                U8 a = v < 0 ? 0 : (v > 255 ? 255 : (U8)roundf(v));
                printf("\033[48;2;%u;%u;%um  ", a, a, a);
            }
            printf("\033[0m\n");
        }
    }
    free(answer);
}

U0 train_stockfih(U0) {
    NN *stockfih = stockfih_birth();

    Game game = new_game();
    tui_small(&game, false);
    draw_nn_outputs(stockfih, &game);
    printf("%s\n", game.legal_moves[ask_chess_nn(&game, stockfih)].notation);


    //play(tui, neural_network, stockfih, human, NULL);
    
    close_nn(stockfih);
}

