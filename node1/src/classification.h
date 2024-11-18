#ifndef CLASSIFICATION_H
#define CLASSIFICATION_H
#include "library.h"

void predict(genann const* ann, float* input, float* output);

void resetMetrics();
void initMetrics(int outputLayers);

void printResult(FILE* file);
float getMacroAccuracy();

int getNumberDataset();
void startTrainingTimer();
void printTrainingTimer(FILE* file);

#endif /* TEST_H */