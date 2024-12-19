#include "classification.h"
#include "genann.h" // Incluye la librería Genann
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>

// Prototipo de la función millis
unsigned long millis();

#define columns 12
int totals = 0;
unsigned short confusionMatrix[5][5];
int __outputLayers;
float mse[columns];
unsigned long testTime;

int higherIndex(const float* output) {
    float t = -1;
    int index = -1;
    for (int i = 0; i < __outputLayers; i++) {
        if (output[i] > t) {
            t = output[i];
            index = i;
        }
    }
    return index;
}

void predict(genann const* ann, float* input, float* output) {
    const float* pd = genann_run(ann, input);
    totals++;
    for (int i = 0; i < columns; i++) {
        float error = powf(output[i] - pd[i], 2); // Error cuadrático
        mse[i] += error; // Acumula el error
    }
}

void predictAnomaly(FILE* file, genann const* ann, float* input, float* output, float* bestValues) {
    const float* run = genann_run(ann, input);
    int allValid = 1; // Variable para verificar si todos los valores son válidos
    float mseTest[columns]; // Arreglo para errores

    for (int i = 0; i < columns; i++) {
        float error = powf(output[i] - run[i], 2); // Calcula error cuadrático
        mseTest[i] = error;

        if (error > bestValues[i]) {
            fprintf(file, "%f,false,", error);
            allValid = 0; // Si cualquier valor es mayor, no es válido
        } else {
            fprintf(file, "%f,true,", error);
        }
    }

    fprintf(file, "%s\n", allValid ? "true" : "false");
}

void resetMetrics() {
    memset(mse, 0, sizeof(mse)); // Reinicia el arreglo mse a 0
    totals = 0;
    testTime = millis(); // Ahora reconoce la función millis correctamente
}

void initMetrics(int outputLayers) {
    __outputLayers = outputLayers;
}

float getMse(int column) {
    return mse[column] / totals;
}

void startTrainingTimer() {
    testTime = millis();
}

void printSdValue(FILE* file, float value, int epoch) {
    fprintf(file, "Epoch: %d, Value: %.4f\n", epoch, value);
}

void printSdValueScape(FILE* file) {
    fprintf(file, "\n");
}

void printTrainingTimer(int epoch, FILE* file) {
    printSdValue(file, (float)(millis() - testTime), epoch);
}

void printResult(FILE* file) {
    printSdValue(file, (float)(millis() - testTime), 1);
}

int getNumberDataset() {
    return totals;
}

// Implementación de millis para C
unsigned long millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}
