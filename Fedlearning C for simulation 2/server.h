#ifndef SERVER_H
#define SERVER_H

#include "genann.h"

// Función para realizar FedAvg (Federated Averaging) sobre los modelos de los nodos
void fedAvg(genann** ann_nodes, int num_nodes, const char* pathSaveWeightsGlobal);

// Función para guardar los pesos globales en un archivo
void saveGlobalWeights(genann* global_model, const char* path);

#endif // SERVER_H
