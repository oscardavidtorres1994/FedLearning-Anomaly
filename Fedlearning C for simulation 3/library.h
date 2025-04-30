#ifndef LIBRARY_H
#define LIBRARY_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "genann.h"

// Declaración de funciones para el entrenamiento y manejo de datos
void initDataframe(int inputLayers, int outputLayers);
bool readData(FILE* in, float* input, float* output);
void saveWeightsJson(genann const* ann, const char* path, int items);
void loadWeightsJson(genann* ann, const char* path);
bool calculateMetrics(const char* metricsFilePath, const char* labelsFilePath, const char* resultsFilePath);


#endif /* LIBRARY_H */
