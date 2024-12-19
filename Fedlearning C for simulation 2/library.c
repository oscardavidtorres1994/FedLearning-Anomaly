#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int _inputLayers = 0;
int _outputLayers = 0;

const int lineCharLength = 30;  // Definir el tamaño máximo de la línea

// bool readData(FILE* in, float* input, float* output) {
//     char line[256];
//     int i = 0;

//     printf("Leyendo datos: _inputLayers = %d\n", _inputLayers);

//     if (fgets(line, sizeof(line), in) == NULL) {
//         printf("Error: fin de archivo o error al leer el archivo.\n");
//         return false;
//     }
//     printf("Línea leída: %s\n", line);

//     line[strcspn(line, "\r\n")] = '\0';  // Limpia saltos de línea
//     printf("Línea limpia: %s\n", line);

//     char* token = strtok(line, ",");
//     while (token != NULL && i < _inputLayers) {
//         input[i] = atof(token);
//         printf("Token %d: %s -> %f\n", i, token, input[i]);  // Depuración de tokens
//         i++;
//         token = strtok(NULL, ",");
//     }

//     if (i != _inputLayers) {
//         printf("Error: número inesperado de columnas. Se esperaban %d, se encontraron %d.\n", _inputLayers, i);
//         return false;
//     }

//     for (int j = 0; j < _inputLayers; j++) {
//         output[j] = input[j];
//     }

//     return true;
// }


bool readData(FILE* in, float* input, float* output) {
    char line[256];  // Buffer para leer una línea completa, tamaño fijo
    int i = 0;
    // printf("%d\n", _inputLayers);


    if (fgets(line, sizeof(line), in) == NULL) {
        return false; // Fin de archivo o error
    }

    // Tokenizar la línea leída usando comas
    char* token = strtok(line, ",");
    while (token != NULL && i < _inputLayers) {
        input[i] = atof(token);  // Convertir cada token en un valor flotante
        i++;
        token = strtok(NULL, ",");
        // printf(token);
    }

    if (i != _inputLayers) {
        printf("Error: número inesperado de columnas en una línea del CSV\n");
        return false;
    }

    for (int j = 0; j < _inputLayers; j++) {
        output[j] = input[j];  // Copiar input a output para el autoencoder
    }

    return true;
}


void initDataframe(int inputLayers, int outputLayers) {
    _inputLayers = inputLayers;
    _outputLayers = outputLayers;
}

void saveWeightsJson(genann const* ann, const char* path, int items) {
    FILE* file = fopen(path, "w");
    if (!file) {
        printf("Could not open file: weights\n");
        return;
    }

    fprintf(file, "%d\n", items);
    for (int i = 0; i < ann->total_weights; ++i) {
        fprintf(file, "%.20f\n", ann->weight[i]);
    }

    fclose(file);
}

void loadWeightsJson(genann* ann, const char* path) {
    FILE* file = fopen(path, "r");
    if (!file) {
        printf("Could not open weights file\n");
        return;
    }

    char line[64];  // Buffer para leer cada línea
    int items;
    if (fgets(line, sizeof(line), file)) {
        items = atoi(line);  // Leer items (aunque no se usa)
    }

    int i = 0;
    while (fgets(line, sizeof(line), file) && i < ann->total_weights) {
        ann->weight[i++] = atof(line);  // Convertir la línea a float
    }

    fclose(file);
    printf("Weights loaded successfully\n");
}
