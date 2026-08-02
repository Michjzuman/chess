#ifndef NN_H
#define NN_H

#include "chess.h"

typedef float(*AF)(float);

typedef struct {
    float bias;
    float *weights;
} Neuron;

typedef struct {
    U32 amount_of_neurons: 30;
    U8 activation: 2;
} LayerConf;

typedef struct {
    LayerConf conf;
    Neuron *neurons;
} Layer;

typedef struct {
    U32 amount_of_inputs;
    U32 amount_of_layers;
    Layer *layers;
} NN;

U0 close_nn(NN *);
NN copy_nn(const NN *);
NN mutate_nn(const NN *);
U0 save_nn(const NN *, char *path);
NN open_nn(char *path);

float *ask_nn(const NN *, float *);

NN new_chess_nn();
U8 ask_chess_nn(const Game *, const NN *);

U8 neural_network(const Game *, U0 *nn);

U0 tournament();

#endif
