#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H
// #include "genann.h"
#include <stdio.h>
#include "library.h"
// #include "library.h"

void predict(genann const* ann, float* input, float* output);
void predictAnomaly(FILE* file, genann const* ann, float* input, float* output, float* bestValues); 

void resetMetrics();
void initMetrics(int outputLayers);

void printResult(FILE* file);
float getMacroAccuracy();

int getNumberDataset();
void startTrainingTimer();
void printTrainingTimer(int epoch, FILE* file);

#endif /* TEST_H */