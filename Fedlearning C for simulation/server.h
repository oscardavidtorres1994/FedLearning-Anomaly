#ifndef SERVER_H
#define SERVER_H

#include "genann.h"

// Declaración de la función FedAvg
void average_weights(genann *anns[], int num_nodes, genann *global_ann);

#endif

