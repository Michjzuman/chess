#include <math.h>

#include "chess.h"
#include "nn.h"

static float linear(float x) { return x; }
static float relu(float x) { return x > 0.0f ? x : 0.0f; }
static float sigmoid(float x) { return 1.0f / (1.0f + exp(-x)); }
static float silu(float x) { return x * sigmoid(x); }

static const AF activations[] = {
    linear, relu, sigmoid, silu
};

static float rand_float(float min, float max) {
    return min + (max - min) * ((float)rand() / (float)RAND_MAX);
}

static U32 amount_of_outputs(const NN *nn) {
    return (
        nn->layers[nn->amount_of_layers - 1]
        .conf.amount_of_neurons
    );
}

U0 close_nn(NN *nn) {
    for (U32 i1 = 0; i1 < nn->amount_of_layers; i1++) {
        U32 layer = nn->layers[i1].conf.amount_of_neurons;
        for (U32 i2 = 0; i2 < layer; i2++) {
            free(nn->layers[i1].neurons[i2].weights);
        }
        free(nn->layers[i1].neurons);
    }
    free(nn->layers);
}

U0 save_nn(const NN *nn, char *path) {
    FILE *file = fopen(path, "wb");
    if (file == NULL) {
        fprintf(stderr, "file %s could not be opened\n", path);
        return;
    }
    if (
        fwrite(&nn->amount_of_inputs, sizeof(U32), 1, file) != 1 ||
        fwrite(&nn->amount_of_layers, sizeof(U32), 1, file) != 1
    ) {
        perror("file could not be written\n");
        fclose(file);
        return;
    }
    for (U32 i = 0; i < nn->amount_of_layers; i++) {
        if (fwrite(&nn->layers[i].conf, sizeof(LayerConf), 1, file) != 1) {
            perror("file could not be written (layers)\n");
            fclose(file);
            return;
        };
    }
    for (U32 i1 = 0; i1 < nn->amount_of_layers; i1++) {
        U32 prev_layer = (
            i1 == 0 ? nn->amount_of_inputs :
            nn->layers[i1 - 1].conf.amount_of_neurons
        );
        U32 layer = (
            nn->layers[i1].conf.amount_of_neurons
        );
        for (U32 i2 = 0; i2 < layer; i2++) {
            if (fwrite(&nn->layers[i1].neurons[i2].bias, sizeof(float), 1, file) != 1) {
                perror("file could not be written (bias)\n");
                fclose(file);
                return;
            }
            if (fwrite(nn->layers[i1].neurons[i2].weights, sizeof(float), prev_layer, file) != prev_layer) {
                perror("file could not be written (weights)\n");
                fclose(file);
                return;
            }
        }
    }
    fclose(file);
}

NN open_nn(char *path) {
    NN nn;
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        fprintf(stderr, "file %s could not be opened\n", path);
        return nn;
    }
    if (
        fread(&nn.amount_of_inputs, sizeof(U32), 1, file) != 1 ||
        fread(&nn.amount_of_layers, sizeof(U32), 1, file) != 1
    ) {
        perror("file could not be read\n");
        fclose(file);
        return nn;
    }
    nn.layers = malloc(nn.amount_of_layers * sizeof(Layer));
    for (U32 i = 0; i < nn.amount_of_layers; i++) {
        if (fread(&nn.layers[i].conf, sizeof(LayerConf), 1, file) != 1) {
            perror("file could not be written (layer conf)\n");
            fclose(file);
            return nn;
        };
    }
    for (U32 i1 = 0; i1 < nn.amount_of_layers; i1++) {
        U32 prev_layer = (
            i1 == 0 ? nn.amount_of_inputs :
            nn.layers[i1 - 1].conf.amount_of_neurons
        );
        U32 layer = (
            nn.layers[i1].conf.amount_of_neurons
        );
        nn.layers[i1].neurons = malloc(layer * sizeof(Neuron));
        for (U32 i2 = 0; i2 < layer; i2++) {
            nn.layers[i1].neurons[i2].weights = malloc(prev_layer * sizeof(float));
            if (fread(&nn.layers[i1].neurons[i2].bias, sizeof(float), 1, file) != 1) {
                perror("file could not be read (bias)\n");
                fclose(file);
                return nn;
            }
            if (fread(nn.layers[i1].neurons[i2].weights, sizeof(float), prev_layer, file) != prev_layer) {
                perror("file could not be read (weights)\n");
                fclose(file);
                return nn;
            }
        }
    }

    fclose(file);
    return nn;
}

float *ask_nn(const NN *nn, float *inputs) {
    float *prev_signals = inputs;
    for (U32 i1 = 0; i1 < nn->amount_of_layers; i1++) {
        U32 prev_layer = (
            i1 == 0 ? nn->amount_of_inputs :
            nn->layers[i1 - 1].conf.amount_of_neurons
        );
        U32 layer = nn->layers[i1].conf.amount_of_neurons;
        float *signals = malloc(layer * sizeof(float));
        for (U32 i2 = 0; i2 < layer; i2++) {
            signals[i2] = nn->layers[i1].neurons[i2].bias;
            for (U32 i3 = 0; i3 < prev_layer; i3++) {
                float weight = nn->layers[i1].neurons[i2].weights[i3];
                signals[i2] += prev_signals[i3] * weight;
            }
            AF f = activations[nn->layers[i1].conf.activation];
            signals[i2] = f(signals[i2]);
        }
        free(prev_signals);
        prev_signals = signals;
    }
    return prev_signals;
}

NN new_chess_nn() {
    NN nn = {
        .amount_of_inputs = 2,
        .amount_of_layers = 2,
        .layers = malloc(nn.amount_of_layers * sizeof(Layer))
    };
    for (U32 i1 = 0; i1 < nn.amount_of_layers; i1++) {
        nn.layers[i1].conf = (LayerConf){
            .activation = rand() % 4,
            .amount_of_neurons = (
                i1 == nn.amount_of_layers - 1 ?
                130 : 67
            )
        };
        nn.layers[i1].neurons = malloc(
            nn.layers[i1].conf.amount_of_neurons * sizeof(Neuron)
        );
        for (U32 i2 = 0; i2 < nn.layers[i1].conf.amount_of_neurons; i2++) {
            U32 prev_layer = (
                i1 == 0 ? nn.amount_of_inputs :
                nn.layers[i1 - 1].conf.amount_of_neurons
            );
            nn.layers[i1].neurons[i2] = (Neuron){
                .bias = rand_float(-1.0f, 1.0f),
                .weights = malloc(prev_layer * sizeof(float))
            };
            for (U32 i3 = 0; i3 < prev_layer; i3++) {
                nn.layers[i1].neurons[i2].weights[i3] = 2.0f;
            }
        }
    }
    return nn;
}

U8 ask_chess_nn(const Game *game, const NN *nn) {
    float *inputs = malloc(nn->amount_of_inputs * sizeof(float));
    U32 index = 0;
    for (U8 piece = 1; piece < 6; piece++) {
        for (U8 y = 0; y < SIZE; y++) {
            for (U8 x = 0; x < SIZE; x++) {
                inputs[index] = (
                    xy(game, x, y).type == piece ? 1.0f : 0.0f
                );
                index++;
            }
        }
    }

    return 0;
}

U0 tournament() {
    srand(time(NULL));

    {
        NN nn = new_chess_nn();
        save_nn(&nn, "brains/test_brain.nn");
        close_nn(&nn);
    }

    {
        NN nn = open_nn("brains/test_brain.nn");
        
        float *question = malloc(2 * sizeof(float));
        question[0] = 1.0f;
        question[1] = 1.0f;
        float *answer = ask_nn(&nn, question);

        for (U32 i = 0; i < amount_of_outputs(&nn); i++) {
            printf("%f ", answer[i]);
        }
        printf("\n");

        free(answer);
        close_nn(&nn);
    }
}



