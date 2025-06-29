#include "genann.h"
#include <stdio.h>
#include <stdlib.h>
#include "server.h"
// void fedAvg(genann** ann_nodes, int num_nodes, const char* pathSaveWeightsGlobal) {
//     int total_weights = ann_nodes[0]->total_weights;  // Número total de pesos por modelo
//     float* global_weights = (float*)calloc(total_weights, sizeof(float));

//     // Sum all weights
//     for (int i = 0; i < num_nodes; i++) {
//         for (int j = 0; j < total_weights; j++) {
//             global_weights[j] += ann_nodes[i]->weight[j];
//         }
//     }

//     // average the weights 
//     for (int j = 0; j < total_weights; j++) {
//         global_weights[j] /= num_nodes;
//     }

//     // Update local models
//     for (int i = 0; i < num_nodes; i++) {
//         for (int j = 0; j < total_weights; j++) {
//             ann_nodes[i]->weight[j] = global_weights[j];
//         }
//     }

//     printf("FedAvg aggregation completed. Updating weights of all nodes.\n");

//     // Guardar el modelo global (opcional)
//     if (pathSaveWeightsGlobal) {
//         printf("Saving global model to %s...\n", pathSaveWeightsGlobal);
//         saveGlobalWeights(ann_nodes[0], pathSaveWeightsGlobal);
//     }

//     free(global_weights);  // Liberar memoria
// }

void fedAvg(genann** ann_nodes, int num_nodes, const char* pathSaveWeightsGlobal, const char* initialWeightsPath, float alpha) {
    int total_weights = ann_nodes[0]->total_weights;  // Número total de pesos por modelo
    float* global_weights = (float*)calloc(total_weights, sizeof(float));
    float* initial_weights = (float*)calloc(total_weights, sizeof(float));

    // Cargar los pesos iniciales desde el archivo
    FILE* file = fopen(initialWeightsPath, "r");
    if (!file) {
        printf("Error: No se pudo abrir el archivo de pesos iniciales: %s\n", initialWeightsPath);
        free(global_weights);
        free(initial_weights);
        return;
    }

    for (int i = 0; i < total_weights; i++) {
        if (fscanf(file, "%f", &initial_weights[i]) != 1) {
            printf("Error: No se pudieron leer todos los pesos iniciales.\n");
            fclose(file);
            free(global_weights);
            free(initial_weights);
            return;
        }
    }
    fclose(file);
    printf("Adding new average ");
    // Sumar todos los pesos de los nodos
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < total_weights; j++) {
            global_weights[j] += ann_nodes[i]->weight[j];
        }
    }

    // Calcular el promedio de los pesos de los nodos
    for (int j = 0; j < total_weights; j++) {
        global_weights[j] /= num_nodes;
    }

    // Combinar los pesos globales con los iniciales usando alpha
    for (int j = 0; j < total_weights; j++) {
        global_weights[j] = alpha * initial_weights[j] + (1 - alpha) * global_weights[j];
    }

    // Actualizar los modelos locales con los pesos combinados
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < total_weights; j++) {
            ann_nodes[i]->weight[j] = global_weights[j];
        }
    }

    printf("FedAvg aggregation completed with alpha = %.2f. Updating weights of all nodes.\n", alpha);

    // Guardar el modelo global (opcional)
    if (pathSaveWeightsGlobal) {
        printf("Saving global model to %s...\n", pathSaveWeightsGlobal);
        saveGlobalWeights(ann_nodes[0], pathSaveWeightsGlobal);
    }

    free(global_weights);  // Liberar memoria
    free(initial_weights);  // Liberar memoria de los pesos iniciales
}

void saveGlobalWeights(genann* global_model, const char* path) {
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Error: Unable to open file %s for writing.\n", path);
        return;
    }

    for (int i = 0; i < global_model->total_weights; i++) {
        fprintf(file, "%f\n", global_model->weight[i]);
    }

    fclose(file);
    printf("Global model saved to %s successfully.\n", path);
}
