#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
// #include "genann.h"
#include "classification.h"
#include <unistd.h> 
#include "server.h"
#include <dirent.h>
#include <sys/stat.h> // Añadí esto arribaç
#include <ctype.h>

#define nodes 3
#define BASE_PATH "C:/Users/socia/Documents/FederatedLearningC/Data to put on nodes v2/Data to put on nodes v2/Experiment 1/Corrections 2"

#define MAX_WEEKS 100

float alpha = 0.7;

int weeksFound[MAX_WEEKS];
int numWeeks = 0;
int currentWeekIndex = 0;

char pathTrain[nodes][MAX_PATH_LEN];
char pathSaveWeights[nodes][MAX_PATH_LEN];
char pathSaveWeightsGlobal[nodes][MAX_PATH_LEN];

char pathGlobalUnused[nodes][MAX_PATH_LEN];  // para pesos globales no usados esta semana



// char *pathSaveWeightsGlobal="weigthsGlobal.txt";
FILE *trainSetFile[nodes];

int realnodes = 0;  // Número real de nodos encontrados
int testing = 0;
// char pathTrain[nodes][MAX_PATH_LEN];
// char pathSaveWeights[nodes][MAX_PATH_LEN];


// Configuración de la red neuronal
#define numberInputLayer  12
#define numberHiddenLayer  64
#define numberOutputLayer  12
// #define nodes 3
// #define nodes 2

genann *ann_general;         // Modelo general
genann *ann_nodes[nodes];    // Modelos para cada nodo
const int numberEpochs = 5;
const int numberIterations = 16;

float learningRate = 0.1;
const float anneal = 0.995;

int samplesPerNode[nodes];
float input[nodes][numberInputLayer];
float output[nodes][numberOutputLayer];
float bestValues[numberInputLayer];
genann* ann;
int epoch = 0;
int iterations=0;
int iterationRead=16;
int experimentNumber=0;


int week=6;
int nextweek=7;

bool offlineTraining=false;
bool trainingDisabled = false;
bool testingDisabled = false;
bool useTransferLearning = true;
bool useFedAvg = true;

// Rutas de archivo
// const char *pathTrain[nodes] = {"train_multifunctions_sensors.csv", "train_pa_sensors.csv", "train_perfuradora_sensors.csv"};
// const char *pathTrain[nodes] = {"train_multifunctions_sensors.csv", "train_pa_sensors.csv"};
// const char *pathTrain[nodes] = {"train_multifunctions_sensors.csv"};
// const char *pathTrain[nodes] = {"trainId002.csv", "trainId006.csv"};
// char *pathSaveWeights[nodes] = {"weights_train_node1.txt", "weights_train_node2.txt","weights_train_node3.txt"};
// char *pathSaveWeights[nodes] = {"weights_train_node1.txt", "weights_train_node2.txt"};
// char *pathSaveWeights[nodes]= {"weights_train_node1.txt"};

char *pathInitialWeights="weights_initial_node.txt";
const char *pathResult = "result.csv";
const char *pathIterationNumber = "IterationNumber.txt";

// String pathResult = "result.csv";
// String pathAnomalies = "anomaly.csv";
// String pathExperimentFile = "experimentNumber.txt";
const char *pathBest = "best.csv";
// const char *pathTest = "test_val_combined_sensors.csv";
char pathTest[MAX_PATH_LEN];
const char *pathVal = "valMachines.csv";
char pathAnomalies[MAX_PATH_LEN];
char pathTransferFiles[nodes][MAX_PATH_LEN];
// const char *pathAnomalies = "anomaly.csv";

const char *pathAnomaliesVal = "valanomaly.csv";
const char *pathwithin5 = "within5min.csv";
const char *pathmetrics = "metrics.csv";
const char *pathlabels = "labels.csv";
const char *resultsFilePath="results";


// String pathVal = "val.csv";
// String pathSaveWeights = "weights.txt";
// String pathIterationNumber = "IterationNumber.txt";

// Archivos
// FILE *trainSetFile[nodes] = {NULL};
// FILE* trainSetFile;
FILE *resultFile = NULL;
FILE *testSetFile = NULL;
FILE *valSetFile = NULL;
FILE *testAnomaly = NULL;
FILE *valAnomaly = NULL;
FILE *wihitn5min=NULL;
FILE *metrics=NULL;
FILE *labels=NULL;
FILE *best = NULL;
FILE *iterationNumberFile = NULL;


int load_week_train_paths(int weekFolder, int nextweek) {
    // Reiniciar variables globales usadas por otras funciones
    for (int i = 0; i < nodes; i++) {
        pathTrain[i][0] = '\0';
        pathSaveWeights[i][0] = '\0';
        pathSaveWeightsGlobal[i][0] = '\0';
        pathTransferFiles[i][0] = '\0';
        pathGlobalUnused[i][0] = '\0';
    }

    pathAnomalies[0] = '\0';
    pathTest[0] = '\0';
    realnodes = 0;
    iterations = 0;
    offlineTraining = 0;
    testing = 0;
    trainingDisabled = 0;

    DIR *d;
    struct dirent *dir;
    char folderPath[MAX_PATH_LEN];
    char folderPathNext[MAX_PATH_LEN];
    char seenGlobal[nodes][MAX_PATH_LEN];  // scoped OK
    int count = 0;
    int totalGlobals = 0;
    int unusedGlobals = 0;

    snprintf(folderPath, sizeof(folderPath), "%s/week_%d", BASE_PATH, weekFolder);
    snprintf(folderPathNext, sizeof(folderPathNext), "%s/week_%d", BASE_PATH, nextweek);

    d = opendir(folderPath);
    if (!d) {
        printf("[ERROR] No se pudo abrir la carpeta %s\n", folderPath);
        return -1;
    }

    while ((dir = readdir(d)) != NULL) {
        printf("[DEBUG] Encontrado archivo: %s\n", dir->d_name);

        if (strncmp(dir->d_name, "train", 5) == 0 && count < nodes) {
            const char *basename = dir->d_name + 5;

            snprintf(pathTrain[count], MAX_PATH_LEN, "%s/%s", folderPath, dir->d_name);
            snprintf(pathSaveWeights[count], MAX_PATH_LEN, "%s/weights%s", folderPath, basename);
            snprintf(pathSaveWeightsGlobal[count], MAX_PATH_LEN, "%s/global_weights%s", folderPathNext, basename);
            snprintf(pathTransferFiles[count], MAX_PATH_LEN, "%s/global_weights%s", folderPath, basename);
            snprintf(seenGlobal[count], MAX_PATH_LEN, "global_weights%s", basename);

            printf("[DEBUG] pathTrain[%d] = %s\n", count, pathTrain[count]);
            printf("[DEBUG] pathSaveWeights[%d] = %s\n", count, pathSaveWeights[count]);
            printf("[DEBUG] pathSaveWeightsGlobal[%d] = %s\n", count, pathSaveWeightsGlobal[count]);
            printf("[DEBUG] pathTransferFiles[%d] = %s\n", count, pathTransferFiles[count]);
            printf("[DEBUG] Seen Global[%d] = %s\n", count, seenGlobal[count]);

            count++;
        }
    }
    closedir(d);

    DIR *d2 = opendir(folderPath);
    if (d2) {
        struct dirent *dir2;
        while ((dir2 = readdir(d2)) != NULL) {
            snprintf(pathAnomalies, MAX_PATH_LEN, "%s/anomaly.csv", folderPath);
            // snprintf(pathTest, MAX_PATH_LEN,"%s/test_val_combined_sensors.csv",  folderPath);
            snprintf(pathTest, MAX_PATH_LEN,"%s/testMachines.csv",  folderPath);
            // snprintf(pathTest, MAX_PATH_LEN,"%s/test_val_combined_sensors.csv",  folderPath);
            

            if (strncmp(dir2->d_name, "global_weights", 14) == 0) {
                int alreadyUsed = 0;
                for (int i = 0; i < count; i++) {
                    if (strcmp(dir2->d_name, seenGlobal[i]) == 0) {
                        alreadyUsed = 1;
                        break;
                    }
                }

                if (!alreadyUsed && unusedGlobals < nodes) {
                    char srcFile[MAX_PATH_LEN];
                    snprintf(srcFile, sizeof(srcFile), "%s/%s", folderPath, dir2->d_name);
                    snprintf(pathGlobalUnused[unusedGlobals], MAX_PATH_LEN, "%s/%s", folderPathNext, dir2->d_name);

                    FILE *src = fopen(srcFile, "rb");
                    FILE *dst = fopen(pathGlobalUnused[unusedGlobals], "wb");
                    if (src && dst) {
                        char buffer[512];
                        size_t bytes;
                        while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                            fwrite(buffer, 1, bytes, dst);
                        }
                        printf("[INFO] Copiado archivo %s a %s\n", srcFile, pathGlobalUnused[unusedGlobals]);
                    } else {
                        printf("[ERROR] No se pudo copiar %s a %s\n", srcFile, pathGlobalUnused[unusedGlobals]);
                    }
                    if (src) fclose(src);
                    if (dst) fclose(dst);

                    unusedGlobals++;
                }

                if (count == 0 && totalGlobals < nodes) {
                    snprintf(pathTransferFiles[totalGlobals], MAX_PATH_LEN, "%s/%s", folderPath, dir2->d_name);
                    printf("[DEBUG] [testing] pathTransferFiles[%d] = %s\n", totalGlobals, pathTransferFiles[totalGlobals]);
                    totalGlobals++;
                }
            }
        }
        closedir(d2);
    }

    realnodes = count;

    if (realnodes == 0) {
        testing = 1;
        trainingDisabled = 1;
        printf("[INFO] No se encontraron archivos 'train' en %s. Modo testing activado.\n", folderPath);
    } else {
        printf("[INFO] %d archivos de entrenamiento encontrados en %s.\n", realnodes, folderPath);
    }

    return 0;
}



// Función para inicializar archivos
bool initFiles() {
    // Abrir archivos de entrenamiento
    for (int i = 0; i < realnodes; i++) {
        trainSetFile[i] = fopen(pathTrain[i], "r");
        if (!trainSetFile[i]) {
            printf("No se pudo abrir el archivo de entrenamiento: %s\n", pathTrain[i]);
            return false;
        }
        printf("Archivo de entrenamiento %d abierto correctamente.\n", i + 1);
    }


    resultFile = fopen(pathResult, "w");
    if (!resultFile) {
        printf("No se pudo crear el archivo de resultados.\n");
        for (int i = 0; i < 3; i++) fclose(trainSetFile[i]);
        // fclose(trainSetFile); // Cerrar archivos abiertos
        return false;
    }
    printf("Archivo de resultados creado correctamente.\n");


    // Calcular iteraciones restantes
    iterations = numberIterations - iterationRead;
    if (iterations > 0) {
        useTransferLearning = true;
    }

    printf("Archivos inicializados correctamente.\n");
    return true;
}


bool initEstimationfiles(){

    testSetFile = fopen(pathTest, "r");
    if (!testSetFile) {
        printf("Could not create test file. \n");
        return false;
    }

    valSetFile = fopen(pathVal, "r");
    if (!valSetFile) {
        printf("Could not create test file. \n");

        fclose(testSetFile);
        return false;
    }
    // printf("3.1 \n");


    testAnomaly = fopen(pathAnomalies, "w");
    if (!testAnomaly) {
        printf("Could not create test anomaly file.");
        fclose(testSetFile);
        fclose(valSetFile);
        return false;
    }
    // printf("4 \n");

    valAnomaly= fopen(pathAnomaliesVal, "w");
    if (!valAnomaly) {
        printf("Could not create test anomaly file.");
        fclose(testSetFile);
        fclose(valSetFile);
        return false;
    }
    // printf("4 \n");

    wihitn5min= fopen(pathwithin5, "r");
    if (!wihitn5min) {
        printf("Could not create test anomaly file.");
        fclose(testSetFile);
        fclose(valSetFile);
        return false;
    }
    // printf("4 \n");

    metrics= fopen(pathmetrics, "a");
    if (!metrics) {
        printf("Could not create test anomaly file.");
        fclose(testSetFile);
        fclose(valSetFile);
        return false;
    }
    // printf("4 \n");

    labels= fopen(pathlabels, "r");
    if (!labels) {
        printf("Could not create test anomaly file.");
        fclose(testSetFile);
        fclose(valSetFile);
        return false;
    }
    // printf("4 \n");

    best = fopen(pathBest, "r");
    if (!best) {
        printf("Could not open test anomaly best threshold file.\n");
        fclose(testSetFile);
        fclose(testAnomaly);
        fclose(valAnomaly);
        fclose(valSetFile);
        return false;
    } else {
        char buffer[1024];
        if (fgets(buffer, sizeof(buffer), best) == NULL) {
            printf("Error: Could not read from the file.\n");
            fclose(best);
            fclose(testSetFile);
            fclose(testAnomaly);
            fclose(valAnomaly);
            fclose(valSetFile);
            return false;
        }

        int count = 0;
        char *token = strtok(buffer, ",");
        while (token != NULL && count < numberInputLayer) {
            bestValues[count] = strtof(token, NULL);  // Convertir el valor a float y almacenarlo
            // printf("Value %d: %f\n", count, bestValues[count]); // Imprimir el valor leído
            count++;
            token = strtok(NULL, ",");
        }

        if (count != numberInputLayer) {
            printf("Error: The number of values in the file does not match 'numberInputLayer'.\n");
            fclose(best);
            fclose(testSetFile);
            fclose(testAnomaly);
            fclose(valAnomaly);
            fclose(valSetFile);
            return false;
        }

        fclose(best);  // Cerrar el archivo después de procesarlo
        
    }

    printf("All files processed correctly \n");
    // printf("5 \n");

    return true;




}

int countTrainingSamples(FILE *file) {
    int count = 0;
    char line[256];

    // Guardar posición actual
    long pos = ftell(file);
    rewind(file); // Volver al inicio del archivo

    while (fgets(line, sizeof(line), file)) {
        // Podés agregar filtros si hay líneas vacías o cabeceras
        if (line[0] != '\n' && line[0] != '#') {
            count++;
        }
    }

    fseek(file, pos, SEEK_SET); // Restaurar posición original
    return count;
}

void execute(int epoch) {
    for (int i = 0; i < realnodes; i++) {
        if (!trainSetFile[i]) {
            printf("Archivo de entrenamiento no disponible para el nodo %d.\n", i + 1);
            continue;
        }

        fseek(trainSetFile[i], 0, SEEK_END);

        if (epoch<1){
            samplesPerNode[i] = countTrainingSamples(trainSetFile[i]);
            printf("Nodo %d: %d muestras de entrenamiento.\n", i + 1, samplesPerNode[i]);
        }
        

        printf("Nodo %d: Iniciando entrenamiento para la epoca %d...\n", i + 1, epoch + 1);
        startTrainingTimer();

        printTrainingTimer(epoch + 1, resultFile);

        learningRate *= anneal;

        fseek(trainSetFile[i], 0, SEEK_SET);
        // resetMetrics();
        while (readData(trainSetFile[i], input[i], output[i])) {
            genann_train(ann_nodes[i], input[i], output[i], learningRate);
        }
        printf("Nodo %d: Epoca %d completada.\n", i + 1, epoch + 1);


        saveWeightsJson(ann_nodes[i], pathSaveWeights[i], getNumberDataset());
    }
}


// void loop() {
//     while(true){
//         printf("llego aqui ");
//         if (!offlineTraining){
//             if( !trainingDisabled  && iterations < numberIterations){
//                 for(int i=0; i<numberEpochs; i++ ){
//                     execute(epoch);
//                     epoch++;
                    
//                 }
//                 printf("Iteration Number:%d  finished \n", iterations);

//                 fedAvg(ann_nodes, samplesPerNode, realnodes,pathSaveWeightsGlobal, pathInitialWeights, alpha);
//                 iterations++;
//                 epoch=0;
//                 if(iterations >= numberIterations || useFedAvg){
                    
//                     if(iterations >= numberIterations){
//                         printf("Training ended.");
//                         trainingDisabled = true;

                        
//                         for (int i = 0; i < 3; i++) fclose(trainSetFile[i]);

//                         initEstimationfiles();
//                         fseek(testSetFile, 0, SEEK_SET);
//                         fseek(wihitn5min, 0, SEEK_SET);
//                         char withinBuffer[1024]; 

//                         while (readData(testSetFile, input[0], output[0])) {
//                             if (fgets(withinBuffer, sizeof(withinBuffer), wihitn5min) == NULL) {
//                                 printf("Error: Synchronization issue, within5min file has fewer lines than testSetFile.\n");
//                                 break; // Termina si no hay más líneas en within5min
//                             }
//                             bool within5minbool = (strstr(withinBuffer, "true") != NULL);

//                             // Llamar a predictAnomaly con el nuevo parámetro
//                             predictAnomaly(testAnomaly, ann_nodes[0], input[0], output[0], bestValues, within5minbool, metrics);
//                         }
//                          printf("Estimation end \n");

//                         //  fclose(metrics);
//                          fclose(labels);

                        

//                         if (calculateMetrics(pathmetrics, pathlabels, resultsFilePath)) {
//                             printf("Metrics calculated and saved successfully.\n");
//                         } else {
//                             printf("Error calculating metrics.\n");
//                         }

//                         printf("Stimating Val Anomaly.\n");
//                         fseek(valSetFile, 0, SEEK_SET);
//                         while (readData(valSetFile, input[0], output[0])) {
//                             bool within5minbool=false;
//                             predictAnomaly(valAnomaly, ann_nodes[0], input[0], output[0], bestValues, within5minbool, metrics);
//                         }






//                     }
//                 }
//             }
//         }

//         // 

//     }
    
    

    
// }

void loop() {
    while (true) {
        printf("llego aqui\n");

        if (!offlineTraining) {
            // CASO DE ENTRENAMIENTO NORMAL
            if (!trainingDisabled && iterations < numberIterations) {
                for (int i = 0; i < numberEpochs; i++) {
                    execute(epoch);
                    epoch++;
                }

                printf("Iteration Number: %d finished\n", iterations);

                fedAvg(ann_nodes, samplesPerNode, realnodes, pathSaveWeightsGlobal, pathInitialWeights, alpha);
                iterations++;
                epoch = 0;

                if (iterations >= numberIterations || useFedAvg) {
                    if (iterations >= numberIterations) {
                        printf("Training ended.\n");
                        trainingDisabled = true;

                        for (int i = 0; i < 3; i++) fclose(trainSetFile[i]);

                        // for (int i = 0; i < nodes; i++) {
                        //     if (strlen(pathGlobalUnused[i]) > 0) {
                        //         saveGlobalWeights(ann_nodes[i], pathGlobalUnused[i]);
                        //         printf("[INFO] Saved untrained weights as global weights: %s\n", pathGlobalUnused[i]);
                        //     } else {
                        //         printf("[WARN] No available path in pathGlobalUnused[%d], skipping.\n", i);
                        //     }
                        // }

                        goto DO_ESTIMATION;
                    }
                }
            }

            // CASO DE SOLO TESTING (nunca entrenó)
            if (trainingDisabled && iterations == 0) {
                printf("No training performed. Proceeding with testing only.\n");

                // Guardar los pesos iniciales como globales en las rutas detectadas como no usadas
                for (int i = 0; i < nodes; i++) {
                    if (strlen(pathGlobalUnused[i]) > 0) {
                        saveGlobalWeights(ann_nodes[i], pathGlobalUnused[i]);
                        printf("[INFO] Saved untrained weights as global weights: %s\n", pathGlobalUnused[i]);
                    } else {
                        printf("[WARN] No available path in pathGlobalUnused[%d], skipping.\n", i);
                    }
                }

                goto DO_ESTIMATION;
            }

        }

        continue; // sigue iterando

        // === BLOQUE DE ESTIMACIÓN (extraído para usarlo desde ambos casos) ===
        DO_ESTIMATION:
            printf("[INFO] Iniciando estimacion...\n");

            initEstimationfiles();
            fseek(testSetFile, 0, SEEK_SET);
            fseek(wihitn5min, 0, SEEK_SET);

            char withinBuffer[1024];
            while (readData(testSetFile, input[0], output[0])) {
                if (fgets(withinBuffer, sizeof(withinBuffer), wihitn5min) == NULL) {
                    printf("Error: Synchronization issue, within5min file has fewer lines than testSetFile.\n");
                    break;
                }
                bool within5minbool = (strstr(withinBuffer, "true") != NULL);
                predictAnomaly(testAnomaly, ann_nodes[0], input[0], output[0], bestValues, within5minbool, metrics);
            }

            printf("Estimation finished.\n");

            fclose(labels);
            if (calculateMetrics(pathmetrics, pathlabels, resultsFilePath)) {
                printf("Metrics calculated and saved successfully.\n");
            } else {
                printf("Error calculating metrics.\n");
            }

            printf("Estimating Val Anomaly.\n");
            fseek(valSetFile, 0, SEEK_SET);
            while (readData(valSetFile, input[0], output[0])) {
                bool within5minbool = false;
                predictAnomaly(valAnomaly, ann_nodes[0], input[0], output[0], bestValues, within5minbool, metrics);
            }
            fclose(testSetFile);
            fclose(wihitn5min);
            fclose(valSetFile);
            fclose(testAnomaly);
            fclose(valAnomaly);


            break;  // salir del while (true)
    }
}


// Función principal
// int main() {
    
//     load_week_train_paths(week, nextweek);

//     if (testing) {
//         printf("Modo testing activado. No se abrirán archivos de entrenamiento.\n");
//         // Aquí podrías abrir archivo(s) de testing en su lugar
//     } else {
//         if (!initFiles()) {
//             printf("Error al abrir archivos de entrenamiento.\n");
//             return 1;
//         }

//         // Continuar con entrenamiento usando trainSetFile[0 .. nodes-1]
//     }
    
//     if(!trainingDisabled || !testingDisabled){
//       if(!initFiles()){
//         printf("Initialization failed. Exit");
//         while(1);
//         return 0;
//       }
//     }

//     initDataframe(numberInputLayer, numberOutputLayer);
//     initMetrics(numberOutputLayer);

//     ann_general = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer, "tanh");

//     for (int i = 0; i < nodes; i++) {
//         ann_nodes[i] = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer, "tanh");
//         if (useTransferLearning) {
//             printf("Loading transfer learning model for node %d...\n", i + 1);
//             // char filename[256];  // Asegúrate de que el buffer sea lo suficientemente grande
//             // sprintf(filename, "weights_node%d.txt", i + 1);  // Formatear el nombre del archivo
//             loadWeightsJson(ann_nodes[i], pathTransferFiles[i]);  // Pasar el nombre del archivo formateado
//             printf("Transfer learning model loaded for node %d.\n", i + 1);
//         } else {
//             genann_randomize(ann_nodes[i]);
//         }
//     }

//     printf("Programa iniciado.\n");




//     loop();

//     return EXIT_SUCCESS;
// 	//test

// }

int list_weeks_in_base(const char *basePath, int *weeksFound, int maxWeeks) {
    DIR *dir = opendir(basePath);
    if (!dir) {
        printf("[ERROR] No se pudo abrir la carpeta base %s\n", basePath);
        return 0;
    }

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            // Buscar carpetas que empiecen con "week_"
            if (strncmp(entry->d_name, "week_", 5) == 0) {
                // Extraer el número después de "week_"
                const char *p = entry->d_name + 5;
                int valid = 1;
                for (int i = 0; p[i] != '\0'; i++) {
                    if (!isdigit(p[i])) {
                        valid = 0;
                        break;
                    }
                }
                if (valid) {
                    int weekNum = atoi(p);
                    if (count < maxWeeks) {
                        weeksFound[count++] = weekNum;
                    }
                }
            }
        }
    }

    closedir(dir);
    return count;
}

int main() {
    // int numWeeks = 38;
    // int weeksFound[MAX_WEEKS] = {
    //     6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
    //     16, 17, 18, 19,20, 21, 23, 29, 30, 31,
    //     32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
    //     42, 43,44, 45, 46, 53, 54, 55
    // };
    int numWeeks = 36;
    int weeksFound[MAX_WEEKS] = {
        6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 21, 23, 29, 30, 31,
        32, 33, 34, 35, 36, 37, 38, 39, 40,
        42, 43, 45, 46, 53, 54, 55, 56
    };
    // int numWeeks = 2;
    // int weeksFound[MAX_WEEKS] = {
    //     54,55 
    // };

    printf("Carpetas week encontradas: [");
    for (int i = 0; i < numWeeks; i++) {
        printf("%d", weeksFound[i]);
        if (i < numWeeks - 1) printf(", ");
    }
    printf("]\n");

    // Recorremos todas las semanas (de 0 a numWeeks-2) para usar pares consecutivos
    for (currentWeekIndex = 0; currentWeekIndex < numWeeks-1 ; currentWeekIndex++) {
        week = weeksFound[currentWeekIndex];
        nextweek = weeksFound[currentWeekIndex + 1];

        printf("Procesando semana %d y semana %d\n", week, nextweek);

        int res = load_week_train_paths(week, nextweek);
        if (res != 0) {
            printf("[ERROR] No se pudo cargar paths para semanas %d y %d\n", week, nextweek);
            continue; // saltar a la siguiente pareja
        }

        if (testing) {
            printf("Modo testing activado para semanas %d-%d\n", week, nextweek);
        } else {
            if (!initFiles()) {
                printf("Error al abrir archivos de entrenamiento para semanas %d-%d\n", week, nextweek);
                continue;
            }
        }

        initDataframe(numberInputLayer, numberOutputLayer);
        initMetrics(numberOutputLayer);

        ann_general = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer, "tanh");

        for (int i = 0; i < nodes; i++) {
            ann_nodes[i] = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer, "tanh");
            if (useTransferLearning) {
                printf("Loading transfer learning model for node %d...\n", i + 1);
                loadWeightsJson(ann_nodes[i], pathTransferFiles[i]);
                printf("Transfer learning model loaded for node %d.\n", i + 1);
            } else {
                genann_randomize(ann_nodes[i]);
            }
        }

        printf("Entrenamiento para semanas %d y %d iniciado.\n", week, nextweek);

        loop();  // Aquí ejecutas tu loop de entrenamiento para esa pareja de semanas

        // Al terminar loop, el for avanzará a la siguiente pareja automáticamente
    }

    printf("Entrenamiento completado para todas las semanas disponibles.\n");
    return 0;
}
