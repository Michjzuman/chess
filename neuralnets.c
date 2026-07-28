#include "chess.h"


typedef struct {
    U32 amount_of_inputs;
    U32 amount_of_outputs;
    U32 amount_of_hidden_layers;
    U32 *hidden_layers;
    U8 ***weights;
} NN;


NN new_nn(
    U32 amount_of_inputs, U32 amount_of_outputs,
    U32 amount_of_hidden_layers, U32 *hidden_layers
) {
    NN nn = {
        .amount_of_inputs = amount_of_inputs,
        .amount_of_outputs = amount_of_outputs,
        .amount_of_hidden_layers = amount_of_hidden_layers,
        .hidden_layers = hidden_layers
    };
    U64 hidden_layers_sum = 0;
    for (U32 i = 0; i < amount_of_hidden_layers; i++) {
        hidden_layers_sum += hidden_layers[i];
    }
    nn.weights = malloc((hidden_layers_sum + 1) * sizeof(U8 *));
    for (U32 i = 0; i < amount_of_hidden_layers; i++) {
        nn.weights[i] = malloc(hidden_layers[i] * sizeof(U8));
    }
    nn.weights[amount_of_hidden_layers] = malloc(amount_of_outputs * sizeof(U8));

    

    return nn;
}

NN new_chess_nn(U32 amount_of_hidden_layers, U32 *hidden_layers) {
    return new_nn(
        67, 130,
        amount_of_hidden_layers,
        hidden_layers
    );
}

U0 close_nn(NN *nn) {
    free(nn->hidden_layers);
    for (U32 i = 0; i < nn->amount_of_hidden_layers + 1; i++) {
        free(nn->weights[i]);
    }
    free(nn->weights);
}







