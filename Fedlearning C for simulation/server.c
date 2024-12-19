#include "server.h"

// Función para realizar FedAvg (promediar pesos)
void average_weights(genann *anns[], int num_nodes, genann *global_ann) {
    for (int i = 0; i < global_ann->total_weights; i++) {
        double sum = 0.0;
        for (int j = 0; j < num_nodes; j++) {
            sum += anns[j]->weight[i];
        }
        global_ann->weight[i] = sum / num_nodes;
    }
}
