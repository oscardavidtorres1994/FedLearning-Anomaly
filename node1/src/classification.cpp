#include "classification.h"
#define columns 12
int totals = 0;
unsigned short confusionMatrix[5][5];
int __outputLayers;
float mse[columns];
float mseTest[columns];

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
    // Serial.print("llego: ");
    for (int i = 0; i < columns; i++) {
        float error = powf(output[i] - pd[i], 2);  // Calcula el error cuadrático para la columna i
        mse[i] += error;  // Acumula el error cuadrático en mse[i]
        // Serial.print("Predicho: ");
        // Serial.print(pd[i]);
        // Serial.print(" Esperado: ");
        // Serial.println(output[i]);
        // Serial.print("mse: ");
        // Serial.println(mse[i]);

    }
    



   
}

void predictAnomaly(FILE* file, genann const* ann, float* input, float* output, float* bestValues) {
    const float* run = genann_run(ann, input);
    bool allValid = true;  // Variable para verificar si todos los valores son válidos
    float mseTest[columns];  // Arreglo para almacenar los errores calculados

    // Calcula el error y aplica la lógica de comparación con bestValues
    for (int i = 0; i < columns; i++) {
        float error = powf(output[i] - run[i], 2); // Calcula el error cuadrático
        mseTest[i] = error;  // Guarda el error en el arreglo mseTest

        // Comparar el error con el valor correspondiente en bestValues
        if (error > bestValues[i]) {
            fprintf(file, "%f,false,", error);  // Guarda el error y false en el archivo
            allValid = false;  // Si cualquier valor es mayor, todo es falso
        } else {
            fprintf(file, "%f,true,", error);  // Guarda el error y true en el archivo
        }
    }

    // Agrega la columna final con el estado general basado en `allValid`
    if (allValid) {
        fprintf(file, "true\n");
    } else {
        fprintf(file, "false\n");
    }
}


void resetMetrics() {
    mse[columns] = {0};
    totals = 0;
    // for (int i = 0; i < __outputLayers; i++)
    //     for (int m = 0; m < __outputLayers; m++)
    //         confusionMatrix[i][m] = 0;
    testTime = millis();
}

void initMetrics(int outputLayers) {
    __outputLayers = outputLayers;
    //confusionMatrix = (int**)malloc(outputLayers * sizeof(int*));
    //for (int i = 0; i < outputLayers; i++) {
    //    confusionMatrix[i] = (int*)malloc(outputLayers * sizeof(int*));
    //}
}

// float getTP(int _class) {
//     return confusionMatrix[_class][_class];
// }

// float getTN(int _class) {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         if (i != _class) {
//             for (int m = 0; m < __outputLayers; m++) {
//                 if (m != _class)
//                     toReturn += confusionMatrix[i][m];
//             }
//         }
//     }
//     return toReturn;
// }

// float getFP(int _class) {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         if (i != _class)
//             toReturn += confusionMatrix[_class][i];
//     }
//     return toReturn;
// }

// float getFN(int _class) {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         if (i != _class)
//             toReturn += confusionMatrix[i][_class];
//     }
//     return toReturn;
// }

// float getAccuracy(int _class) {
//     float Tp = getTP(_class), Tn = getTN(_class), Fp = getFP(_class), Fn = getFN(_class);
//     return (Tp + Tn) / (Tp + Tn + Fp + Fn);
// }

// float getPrecision(int _class) {
//     float Tp = getTP(_class), Fp = getFP(_class);
//     if(Tp == 0 && Fp == 0) return 1;
//     return (Tp / (Tp + Fp));
// }

// float getRecall(int _class) {
//     float Tp = getTP(_class), Fn = getFN(_class);
//     if(Tp == 0 && Fn == 0) return 1;
//     return (Tp / (Tp + Fn));
// }

// float getF1(int _class) {
//     float p = getPrecision(_class);
//     float r = getRecall(_class);
//     return 2 * ((p * r) / (p + r));
// }

// float getMacroAccuracy() {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         toReturn += confusionMatrix[i][i];
//     }
//     return toReturn / totals;
// }

// float getMicroAccuracy(){
//     return getMacroAccuracy();
// }


// float getMacroPrecision() {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         toReturn += getPrecision(i);
//     }
//     return toReturn / __outputLayers;
// }

// float getMicroPrecision() {
//     float Tp = 0, Fp = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         Tp += getTP(i);
//         Fp += getFP(i);
//     }
//     return Tp / (Tp+ Fp);
// }


// float getMacroRecall() {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         toReturn += getRecall(i);
//     }
//     return toReturn / __outputLayers;
// }


// float getMicroRecall() {
//     float Tp = 0, Fn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         Tp += getTP(i);
//         Fn += getFN(i);
//     }
//     return (Tp / (Tp + Fn));
// }


// float getMacroF1() {
//     float toReturn = 0;
//     for (int i = 0; i < __outputLayers; i++) {
//         toReturn += getF1(i);
//     }
//     return toReturn / __outputLayers;
// }

// float getMicroF1() {
//     float p = getMicroPrecision();
//     float r = getMicroRecall();
//     return 2 * ((p * r) / (p + r));
// }

// void printConfusion() {
//     printf("\n");
//     for (int i = 0; i < __outputLayers; i++) {
//         for (int m = 0; m < __outputLayers; m++)
//             printf("%d\t", confusionMatrix[i][m]);
//         printf("\n");
//     }
// }

float getMse(int column) {
    // Serial.println(mse[column]);
    return mse[column] / totals;
}

// float getMultiClassEntropy() {
//     return multiClassEntropy / totals;
// }

void startTrainingTimer() {
    testTime = millis();
}

void printSdValue(FILE* file, float value, int epoch) {
    fprintf(file, "epoch: ");
    fprintf(file, "Epoch: %d, Value: %.4f \n", epoch, value);
}

void printSdValueScape(FILE* file) {
    fprintf(file, "\n");
}

void printTrainingTimer( int epoch, FILE* file) {
    printSdValue(file, static_cast<float>(millis() - testTime), epoch);
}

void printResult(FILE* file) {
    printSdValue(file, static_cast<float>(millis() - testTime), 1);
    // for (int i = 0; i < columns; i++) {
    //     printSdValue(file, getMse(i));
    //     // printSdValue(file, getPrecision(i));
    //     // printSdValue(file, getRecall(i));
    //     // printSdValue(file, getF1(i));
    // }
    // printSdValueScape(file);
    // printSdValue(file, getMacroAccuracy());
    // printSdValue(file, getMacroPrecision());
    // printSdValue(file, getMacroRecall());
    // printSdValue(file, getMacroF1());
    // printSdValue(file, getMicroAccuracy());
    // printSdValue(file, getMicroPrecision());
    // printSdValue(file, getMicroRecall());
    // printSdValue(file, getMicroF1());
    // printSdValue(file, getMse());
    // fprintf(file, "%.4f\n", getMultiClassEntropy());
}

int getNumberDataset() {
    return totals;
}

