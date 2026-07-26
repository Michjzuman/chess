#include "chess.h"


typedef struct {
    U32 amount_of_inputs;
    U32 amount_of_outputs;
    U32 amount_of_hidden_layers;
    U32 *hidden_layers;
    U8 **weights;
} NN;


NN new_nn(
    U32 amount_of_inputs,
    U32 amount_of_outputs,
    U32 amount_of_hidden_layers,
    U32 *hidden_layers
) {
    NN nn = {
        .amount_of_inputs = amount_of_inputs,
        .amount_of_outputs = amount_of_outputs,
        .amount_of_hidden_layers = amount_of_hidden_layers
    };

    

    return nn;
}

U0 close_nn(NN *nn) {
    free(nn->hidden_layers);
    for (U32 i = 0; i < nn->amount_of_hidden_layers + 1; i++) {
        free(nn->weights[i]);
    }
    free(nn->weights);
}







