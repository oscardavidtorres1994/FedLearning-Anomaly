#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "genann.h" // Asegúrate de tener genann.h y genann.c en el directorio
#include "server.h" // Incluye las funciones del servidor

#define NODES 3                 // Número de nodos (clientes)
#define EPOCHS 3             // Número de épocas
#define numberInputLayer 12                // Número de características de entrada
#define numberOutputLayer 12               // Número de salidas
#define numberHiddenLayer 2        // Número de capas ocultas
#define NEURONS_PER_LAYER 8     // Número de neuronas por capa oculta

void read_csv_data(const char *filename, double data[][INPUTS + OUTPUTS], int *rows);
void train_local_node(genann *ann, double data[][INPUTS + OUTPUTS], int rows);

int main() {
    const char *node_files[NODES] = {"trainId002.csv", "trainId003.csv", "trainId006.csv"};
    int rows[NODES] = {0};
    double data[NODES][100][INPUTS + OUTPUTS];

    // Leer archivos CSV para cada nodo
    for (int i = 0; i < NODES; i++) {
        read_csv_data(node_files[i], data[i], &rows[i]);
        printf("Nodo %d: %d filas cargadas desde %s\n", i + 1, rows[i], node_files[i]);
    }

    // Inicializar redes neuronales
    genann *local_anns[NODES];
    for (int i = 0; i < NODES; i++) {
        local_anns[i] = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer,"tanh");
    }
    genann *global_ann = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer,"tanh");

    // Entrenamiento local y federado
    for (int epoch = 0; epoch < EPOCHS; epoch++) {
        printf("\n--- Epoch %d ---\n", epoch + 1);

        // Entrenamiento local
        for (int i = 0; i < NODES; i++) {
            train_local_node(local_anns[i], data[i], rows[i]);
            printf("Nodo %d entrenado.\n", i + 1);
        }

        // Promediar pesos en el servidor
        average_weights(local_anns, NODES, global_ann);
        printf("Pesos globales actualizados.\n");

        // Sincronizar pesos globales
        for (int i = 0; i < NODES; i++) {
            memcpy(local_anns[i]->weight, global_ann->weight, sizeof(double) * global_ann->total_weights);
        }
    }

    // Liberar memoria
    for (int i = 0; i < NODES; i++) genann_free(local_anns[i]);
    genann_free(global_ann);

    printf("\nEntrenamiento federado finalizado.\n");
    return 0;
}

// Leer datos desde archivo CSV
void read_csv_data(const char *filename, double data[][INPUTS + OUTPUTS], int *rows) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error al abrir el archivo: %s\n", filename);
        exit(1);
    }

    char line[1024];
    *rows = 0;
    while (fgets(line, 1024, file) && *rows < 100) {
        char *token = strtok(line, ",");
        int col = 0;
        while (token) {
            data[*rows][col++] = atof(token);
            token = strtok(NULL, ",");
        }
        (*rows)++;
    }
    fclose(file);
}

// Entrenar nodo local
void train_local_node(genann *ann, double data[][INPUTS + OUTPUTS], int rows) {
    for (int i = 0; i < rows; i++) {
        genann_train(ann, data[i], &data[i][INPUTS], 0.01); // 0.01 = tasa de aprendizaje
    }
}
