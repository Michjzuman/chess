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

NN new_chess_nn();

U0 save_nn(const NN *, char *path);
NN open_nn(char *path);
U0 close_nn(NN *);

float *ask_nn(const NN *, float *);

U0 tournament();

#endif
