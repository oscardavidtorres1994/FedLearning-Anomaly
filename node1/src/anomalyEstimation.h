#ifndef ANOMALYESTIMATION_H
#define ANOMALYESTIMATION_H
#include "library.h"

void predict(genann const* ann, float* input, float* output);
void predictAnomaly(FILE* file, genann const* ann, float* input, float* output, float* bestValues); 
void predictAnomalyVal(FILE* file, genann const* ann, float* input, float* output, float* bestValues); 



void resetMetrics();
void initMetrics(int outputLayers);

void printResult(FILE* file);
float getMacroAccuracy();

int getNumberDataset();
void startTrainingTimer();
void printTrainingTimer(int epoch, FILE* file);

#endif /* TEST_H */