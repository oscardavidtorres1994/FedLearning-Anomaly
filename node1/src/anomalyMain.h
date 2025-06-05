#include <Arduino.h>
#include "anomalyEstimation.h"
#include "mqtt.h"
#include <string>
#include <sstream>
#include <limits> 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp32/clk.h"// Para obtener los límites de float

#include <cctype>  // Para verificar caracteres válidos
// #include "esp_system.h"
#include <esp_heap_caps.h>
#define MAX_SAMPLES 11899
#define MAX_FEATURES 12
#define MAX_THRESHOLDS 50

// Configuración de la red neuronal
const int numberInputLayer = 12;
const int numberHiddenLayer = 64;
// const int numberHiddenLayer = 2;
const int numberOutputLayer = 12;
const int numberEpochs = 30;
const int numberIterations = 1;

float learningRate = 0.1;
const float anneal = 0.995;



float threshold_candidates[MAX_FEATURES][MAX_THRESHOLDS];  // thresholds para cada feature
float best_thresholds[MAX_FEATURES] = {0};
float best_f1s[MAX_FEATURES] = {0};




// Rutas de archivo
String pathTrain = "/sdcard/train.csv";
String pathResult = "/sdcard/result.csv";
String pathAnomalies = "/sdcard/anomaly.csv";
String pathExperimentFile = "/sdcard/experimentNumber.txt";
String pathBest = "/sdcard/best.csv";
String pathTest = "/sdcard/test.csv";
String pathTestGlobal = "/sdcard/testGlobal.csv";
String pathVal = "/sdcard/val.csv";
String pathlabelsVal = "/sdcard/labelsVal.csv";
String pathSaveWeights = "/sdcard/weights.txt";
String pathSaveWeightsFed = "/sdcard/weightsFed.txt";
String pathIterationNumber = "/sdcard/IterationNumber.txt";


// Archivos y variables
FILE* trainSetFile;
FILE* resultFile;
FILE* testSetFile;
FILE* testGlobalSetFile;
FILE* valSetFile;
FILE* labelsValFile;
FILE* testAnomaly;
FILE* testGlobalAnomaly;
FILE* valAnomaly;
FILE* best;
FILE* experimentNumberFile;
FILE* iterationNumberfile;

float input[numberInputLayer];
float output[numberOutputLayer];
float bestValues[numberInputLayer];
genann* ann;
int epoch = 0;
int iterations=0;
int iterationRead=1;
int experimentNumber=0;
bool offlineTraining=true;
bool trainingDisabled = false;
bool testingDisabled = false;
bool useTransferLearning = false;
bool useFedAvg = true;
bool result= false;


const int nodeNumber = 2;
int weightIndex = 0;
bool waitingWeights = false;
bool iterationFinished=false;
bool processFinished=false;

// Inicialización de archivos

// bool initValFiles(){
//     valSetFile = fopen(pathVal.c_str(), "r");
//     if (!valSetFile) {
//         Serial.println("Could not create test file.");
//         // fclose(trainSetFile);
//         // fclose(resultFile);
//         // fclose(testSetFile);
//         return false;
//     }
//     Serial.println("3.1");

//     labelsValFile = fopen(pathlabelsVal.c_str(), "r");
//     if (!valSetFile) {
//         Serial.println("Could not create test file.");
//         // fclose(trainSetFile);
//         // fclose(resultFile);
//         fclose(valSetFile);
//         return false;
//     }
//     Serial.println("3.2");
//     return true;
// }
// bool initEstimationfiles(){

//     testSetFile = fopen(pathTest.c_str(), "r");
//     if (!testSetFile) {
//         Serial.println("Could not create test file.");
//         return false;
//     }

    
    

//     experimentNumberFile = fopen(pathExperimentFile.c_str(), "r");
//     if (!experimentNumberFile) {
//         printf("Could not open experiment number file: %s\n", experimentNumberFile);
//         return false;
//     }else{
//         if (fscanf(experimentNumberFile, "%d", &experimentNumber) != 1) {
//             printf("Error reading experiment number.\n");
//             fclose(experimentNumberFile);
//             return false;
//         }
//         fclose(experimentNumberFile);

//     }
    
//     Serial.println("3.5");


    
//     Serial.println("3.8");
    

//     std::ostringstream anomalyFileNameStream;
//     anomalyFileNameStream << "/sdcard/anomaly" << experimentNumber << ".csv";
//     std::string anomalyFileName = anomalyFileNameStream.str();

//     testAnomaly = fopen(anomalyFileName.c_str(), "a");
//     if (!testAnomaly) {
//         Serial.println("Could not create test anomaly file.");
//         fclose(testSetFile);
//         // fclose(valSetFile);
//         return false;
//     }
//     Serial.println("4");

    

//     std::ostringstream anomalyFileNameStream1;
//     anomalyFileNameStream1 << "/sdcard/valanomaly" << experimentNumber << ".csv";
//     std::string anomalyFileName1 = anomalyFileNameStream1.str();

//     valAnomaly = fopen(anomalyFileName1.c_str(), "a");
//     if (!testAnomaly) {
//         Serial.println("Could not create test anomaly file.");
//         fclose(testSetFile);
//         fclose(testAnomaly);
//         fclose(valSetFile);
//         return false;
//     }

//     Serial.println("4.5");

//     best = fopen(pathBest.c_str(), "r");
//     if (!best) {
//         printf("Could not open test anomaly best threshold file.\n");
//         fclose(testSetFile);
//         fclose(testAnomaly);
//         fclose(valAnomaly);
//         // fclose(valSetFile);
//         return false;
//     } else {
//         char buffer[1024];
//         if (fgets(buffer, sizeof(buffer), best) == NULL) {
//             printf("Error: Could not read from the file.\n");
//             fclose(best);
//             fclose(testSetFile);
//             fclose(testAnomaly);
//             fclose(valAnomaly);
//             // fclose(valSetFile);
//             return false;
//         }

//         int count = 0;
//         char *token = strtok(buffer, ",");
//         while (token != NULL && count < numberInputLayer) {
//             bestValues[count] = strtof(token, NULL);  // Convertir el valor a float y almacenarlo
//             // printf("Value %d: %f\n", count, bestValues[count]); // Imprimir el valor leído
//             count++;
//             token = strtok(NULL, ",");
//         }

//         if (count != numberInputLayer) {
//             printf("Error: The number of values in the file does not match 'numberInputLayer'.\n");
//             fclose(best);
//             fclose(testSetFile);
//             fclose(testAnomaly);
//             fclose(valAnomaly);
//             // fclose(valSetFile);
//             return false;
//         }

//         fclose(best);  // Cerrar el archivo después de procesarlo
//     }


//     Serial.println("5");

//     return true;




// }

// bool initestimationGlobalfiles(){

//     testGlobalSetFile = fopen(pathTestGlobal.c_str(), "r");
//     if (!testGlobalSetFile) {
//         Serial.println("Could not create test file.");
//         return false;
//     }

//     experimentNumberFile = fopen(pathExperimentFile.c_str(), "r");
//     if (!experimentNumberFile) {
//         printf("Could not open experiment number file: %s\n", experimentNumberFile);
//         return false;
//     }else{
//         if (fscanf(experimentNumberFile, "%d", &experimentNumber) != 1) {
//             printf("Error reading experiment number.\n");
//             fclose(experimentNumberFile);
//             return false;
//         }
//         fclose(experimentNumberFile);

//     }

//     std::ostringstream anomalyFileNameStreamGlobal;
//     anomalyFileNameStreamGlobal << "/sdcard/anomalyGlobal" << experimentNumber << ".csv";
//     std::string anomalyFileNameGlobal = anomalyFileNameStreamGlobal.str();

//     testGlobalAnomaly = fopen(anomalyFileNameGlobal.c_str(), "a");
//     if (!testGlobalAnomaly) {
//         Serial.println("Could not create test anomaly file.");
//         fclose(testSetFile);
//         fclose(valSetFile);
//         return false;
//     }
//     Serial.println("4");


//     return true;
// }

// bool initFiles() {
//     trainSetFile = fopen(pathTrain.c_str(), "r");
//     if (!trainSetFile) {
//         Serial.printf("Could not open file: %s\n", pathTrain.c_str());
//         return false;
//     }
//     Serial.println("1");

//     // resultFile = fopen(pathResult.c_str(), "a");
//     // if (!resultFile) {
//     //     Serial.println("Could not create result file.");
//     //     fclose(trainSetFile);
//     //     return false;
//     // }
//     // Serial.println("2");

//     iterationNumberfile = fopen(pathIterationNumber.c_str(), "r");
//     if (!iterationNumberfile) {
//         printf("Could not open experiment number file: %s\n", iterationNumberfile);
//         return false;
//     }else{
//         if (fscanf(iterationNumberfile, "%d", &iterationRead) != 1) {
//             printf("Error reading experiment number.\n");
//             fclose(iterationNumberfile);
//             return false;
//         }
//         fclose(iterationNumberfile);

//     }

//     // printf("iteration read: %s\n", iterationRead);
//     // printf("Number iterations: %s\n", numberIterations);
//     iterations = 0;
//     // printf("Number iterations calculed: %s\n", numberIterations);
//     if(iterations>0){
//         useTransferLearning=true;
//     }

//     Serial.println("3");
   

//     return true;
// }

bool initValFiles(){
    valSetFile = fopen(pathVal.c_str(), "r");
    if (!valSetFile) {
        Serial.println("Could not open validation set file.");
        return false;
    }
    Serial.println("3.1");

    labelsValFile = fopen(pathlabelsVal.c_str(), "r");
    if (!labelsValFile) {
        Serial.println("Could not open labels validation file.");
        fclose(valSetFile);
        return false;
    }
    Serial.println("3.2");

    return true;
}

bool initEstimationfiles(){
    testSetFile = fopen(pathTest.c_str(), "r");
    if (!testSetFile) {
        Serial.println("Could not open test set file.");
        return false;
    }

    experimentNumberFile = fopen(pathExperimentFile.c_str(), "r");
    if (!experimentNumberFile) {
        Serial.println("Could not open experiment number file.");
        fclose(testSetFile);
        return false;
    }

    if (fscanf(experimentNumberFile, "%d", &experimentNumber) != 1) {
        Serial.println("Error reading experiment number.");
        fclose(experimentNumberFile);
        fclose(testSetFile);
        return false;
    }
    fclose(experimentNumberFile);

    Serial.println("3.5");

    std::ostringstream anomalyFileNameStream;
    anomalyFileNameStream << "/sdcard/anomaly" << experimentNumber << ".csv";
    testAnomaly = fopen(anomalyFileNameStream.str().c_str(), "a");
    if (!testAnomaly) {
        Serial.println("Could not create test anomaly file.");
        fclose(testSetFile);
        return false;
    }

    std::ostringstream anomalyFileNameStream1;
    anomalyFileNameStream1 << "/sdcard/valanomaly" << experimentNumber << ".csv";
    valAnomaly = fopen(anomalyFileNameStream1.str().c_str(), "a");
    if (!valAnomaly) {
        Serial.println("Could not create validation anomaly file.");
        fclose(testAnomaly);
        fclose(testSetFile);
        return false;
    }

    Serial.println("4.5");

    best = fopen(pathBest.c_str(), "r");
    if (!best) {
        Serial.println("Could not open best threshold file.");
        fclose(testSetFile);
        fclose(testAnomaly);
        fclose(valAnomaly);
        return false;
    }

    char buffer[1024];
    if (!fgets(buffer, sizeof(buffer), best)) {
        Serial.println("Error reading best threshold values.");
        fclose(best);
        fclose(testSetFile);
        fclose(testAnomaly);
        fclose(valAnomaly);
        return false;
    }

    int count = 0;
    char* token = strtok(buffer, ",");
    while (token && count < numberInputLayer) {
        bestValues[count++] = strtof(token, nullptr);
        token = strtok(nullptr, ",");
    }

    if (count != numberInputLayer) {
        Serial.println("Best threshold values do not match input layer size.");
        fclose(best);
        fclose(testSetFile);
        fclose(testAnomaly);
        fclose(valAnomaly);
        return false;
    }

    fclose(best);
    Serial.println("5");
    return true;
}

bool initestimationGlobalfiles(){
    testGlobalSetFile = fopen(pathTestGlobal.c_str(), "r");
    if (!testGlobalSetFile) {
        Serial.println("Could not open global test set file.");
        return false;
    }

    experimentNumberFile = fopen(pathExperimentFile.c_str(), "r");
    if (!experimentNumberFile) {
        Serial.println("Could not open experiment number file.");
        fclose(testGlobalSetFile);
        return false;
    }

    if (fscanf(experimentNumberFile, "%d", &experimentNumber) != 1) {
        Serial.println("Error reading experiment number.");
        fclose(experimentNumberFile);
        fclose(testGlobalSetFile);
        return false;
    }
    fclose(experimentNumberFile);

    std::ostringstream anomalyFileNameStreamGlobal;
    anomalyFileNameStreamGlobal << "/sdcard/anomalyGlobal" << experimentNumber << ".csv";
    testGlobalAnomaly = fopen(anomalyFileNameStreamGlobal.str().c_str(), "a");
    if (!testGlobalAnomaly) {
        Serial.println("Could not create global anomaly file.");
        fclose(testGlobalSetFile);
        return false;
    }

    Serial.println("4");
    return true;
}

bool initFiles(){
    trainSetFile = fopen(pathTrain.c_str(), "r");
    if (!trainSetFile) {
        Serial.printf("Could not open training set file: %s\n", pathTrain.c_str());
        return false;
    }
    Serial.println("1");

    iterationNumberfile = fopen(pathIterationNumber.c_str(), "r");
    if (!iterationNumberfile) {
        Serial.println("Could not open iteration number file.");
        fclose(trainSetFile);
        return false;
    }

    if (fscanf(iterationNumberfile, "%d", &iterationRead) != 1) {
        Serial.println("Error reading iteration number.");
        fclose(iterationNumberfile);
        fclose(trainSetFile);
        return false;
    }
    fclose(iterationNumberfile);

    iterations = 0;
    if (iterations > 0) {
        useTransferLearning = true;
    }

    Serial.println("3");
    return true;
}


// Ejecución de una época de entrenamiento y prueba
void execute(int epoch) {
    // if (!trainingDisabled) {
    //     resultFile = fopen(pathResult.c_str(), "a");
    //     if (!resultFile) {
    //         Serial.println("Failed to open result file.");
    //         return; // Salir si no se puede abrir el archivo
    //     }
    //     fseek(trainSetFile, 0, SEEK_SET);
    //     size_t freeMemBefore = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    //     size_t minFreeMemBefore = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);

    //     startTrainingTimer();
    //     while (readData(trainSetFile, input, output)) {
    //         genann_train(ann, input, output, learningRate);
    //     }
    //     if(resultFile){
    //         printTrainingTimer(epoch+1, resultFile);
    //     }
        
    //     size_t freeMemAfter = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    //     size_t minFreeMemAfter = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);

    //     learningRate *= anneal;

    //     fseek(trainSetFile, 0, SEEK_SET);
    //     resetMetrics();
    //     while (readData(trainSetFile, input, output))
    //         predict(ann, input, output);
    //     // printResult(resultFile);
    //     if(resultFile){
    //         // Serial.println("llego");
    //         fprintf(resultFile, "Epoch %d Memory Report:\n", epoch + 1);
    //         fprintf(resultFile, "Free Memory Before Training: %d bytes\n", freeMemBefore);
    //         fprintf(resultFile, "Minimum Free Memory Before Training: %d bytes\n", minFreeMemBefore);
    //         fprintf(resultFile, "Free Memory After Training: %d bytes\n", freeMemAfter);
    //         fprintf(resultFile, "Minimum Free Memory After Training: %d bytes\n", minFreeMemAfter);

            

            
    //     }
    //     fclose(resultFile);
        
    //     Serial.printf("Epoch %d completed.\n", epoch + 1);
    // }
   
    // if (!testingDisabled) {
    //     resetMetrics();
    //     while (readData(testSetFile, input, output)) {
    //         predict(ann, input, output);
    //     }
    //     // printResult(resultFile);
    // }
        if (!trainingDisabled) {
            resultFile = fopen(pathResult.c_str(), "a");
            if (!resultFile) {
                Serial.println("Failed to open result file.");
                return;
            }

            fseek(trainSetFile, 0, SEEK_SET);
            Serial.printf("Total PSRAM: %d bytes\n", heap_caps_get_total_size(MALLOC_CAP_SPIRAM));

            // ✅ Measure memory before training
            size_t freeMemBefore = heap_caps_get_free_size(MALLOC_CAP_8BIT);
            size_t minFreeMemBefore = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
            size_t largestFreeBlockBefore = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
            UBaseType_t stackBefore = uxTaskGetStackHighWaterMark(NULL);

            // ✅ Get CPU core & frequency
            int coreID = xPortGetCoreID();
            int cpuFreq = esp_clk_cpu_freq() / 1000000;  // Convert Hz to MHz

            // ✅ Start CPU measurement
            int64_t startCPUTime = esp_timer_get_time();

            startTrainingTimer();
            while (readData(trainSetFile, input, output)) {
                genann_train(ann, input, output, learningRate);
            }

            int64_t elapsedCPUTime = esp_timer_get_time() - startCPUTime;  // Time in µs

            // ✅ Measure memory after training
            size_t freeMemAfter = heap_caps_get_free_size(MALLOC_CAP_8BIT);
            size_t minFreeMemAfter = heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT);
            size_t largestFreeBlockAfter = heap_caps_get_largest_free_block(MALLOC_CAP_8BIT);
            UBaseType_t stackAfter = uxTaskGetStackHighWaterMark(NULL);

            learningRate *= anneal;

   
            Serial.printf("Free PSRAM: %d bytes\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM));
            if (resultFile) {
                fprintf(resultFile, "Epoch %d Memory & CPU Report:\n", epoch + 1);
                fprintf(resultFile, "Free Memory Before Training: %d bytes\n", freeMemBefore);
                fprintf(resultFile, "Minimum Free Memory Before Training: %d bytes\n", minFreeMemBefore);
                fprintf(resultFile, "Largest Free Block Before: %d bytes\n", largestFreeBlockBefore);
                fprintf(resultFile, "Stack High Water Before: %d bytes\n", stackBefore);

                fprintf(resultFile, "Free Memory After Training: %d bytes\n", freeMemAfter);
                fprintf(resultFile, "Minimum Free Memory After Training: %d bytes\n", minFreeMemAfter);
                fprintf(resultFile, "Largest Free Block After: %d bytes\n", largestFreeBlockAfter);
                fprintf(resultFile, "Stack High Water After: %d bytes\n", stackAfter);

                fprintf(resultFile, "CPU Core Used: %d\n", coreID);
                fprintf(resultFile, "CPU Frequency: %d MHz\n", cpuFreq);
                fprintf(resultFile, "CPU Time Used: %.2f ms\n", elapsedCPUTime / 1000.0);

            }

            fclose(resultFile);
            Serial.printf("Epoch %d completed.\n", epoch + 1);
        }

    if (!trainingDisabled) {
        saveWeightsJson(ann, pathSaveWeights, getNumberDataset());
    }
}



void callback(const char* topic, byte* payload, unsigned int length) {
    waitingWeights = true;

    Serial.print("Receiving weight: ");
    Serial.print(weightIndex + 1);
    Serial.print("\\");
    Serial.print(ann->total_weights);

    // Convertir el payload a un string seguro
    char buffer[length + 1];
    strncpy(buffer, reinterpret_cast<const char*>(payload), length);
    buffer[length] = '\0';  // Asegurarse de que el string esté terminado

    Serial.print("\tPayload: ");
    Serial.println(buffer);

    // Intentar convertir el payload a float
    float weight = atof(buffer);

    ann->weight[weightIndex] = weight;
    Serial.print("\tWeight: ");
    printf("%f\n", ann->weight[weightIndex]);

    weightIndex++;
    if (weightIndex >= ann->total_weights) {
        Serial.println("All weights received");
        weightIndex = 0;
        Serial.println("saving weights..");
        saveWeightsJson(ann, pathSaveWeightsFed, getNumberDataset());
        Serial.println("setting new iteration number");


        iterationNumberfile = fopen(pathIterationNumber.c_str(), "w");
        if (iterationNumberfile == NULL) {
        printf("Error: Could not open file %s for reading.\n", pathIterationNumber);
        
        }else{
            int newIterations = numberIterations - iterations;
            Serial.println("Iterations number");
            Serial.println(numberIterations);
            Serial.println(newIterations);
            if(newIterations==0){
                newIterations=16;
            }
            fprintf(iterationNumberfile, "%d\n", newIterations); // Guardar el nuevo valor en el archivo
            fclose(iterationNumberfile);
        }
        


        if (trainingDisabled && !testingDisabled) {
            
            execute(0);
        }
        waitingWeights = false;
    }
}




// Modo de entrenamiento inicial
void trainingMode() {
    if (!offlineTraining){
        initMQTT(nodeNumber);
    } else{
        Serial.println("nodes Trainning offline");

    }
    

    
    if(!trainingDisabled || !testingDisabled){
      init_sd_card();
      if(!initFiles()){
        Serial.println("Initialization failed. Exit");
        while(1);
        return;
      }
    }

    initDataframe(numberInputLayer, numberOutputLayer);
    initMetrics(numberOutputLayer);

    ann = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer,"tanh");
    if (ann == nullptr) {
        Serial.println("Out of memory");
        while (1);
    }

    if(trainingDisabled){
        awaitWeights(callback, nodeNumber, !trainingDisabled);
        return;
    }

    srand(time(0));
    if (useTransferLearning){
        Serial.println("Loading transfer learning model...");
        loadWeightsJson(ann, "/sdcard/weightsFed.txt");
        Serial.println("Transfer learning model loaded");
    }
    else genann_randomize(ann);
    // Serial.println("Starting training...");

    if(!offlineTraining){
        Serial.println("Traning online");
        Serial.println("Waiting other nodes for 10 seconds ");
        delay(1*10*1000);
    }
    
    Serial.println("starting training:");


    if(!offlineTraining){
        awaitWeights(callback, nodeNumber, !trainingDisabled);
    }   
    
}


// void _loop() {
//     if (!offlineTraining){
//         if( !trainingDisabled && !waitingWeights && iterations < numberIterations){
//             closeConnection(); //disable MQTT before training to save memory
//             for(int i=0; i<numberEpochs; i++ ){
//                 execute(epoch);
//                 epoch++;
//             }
//             Serial.print("Iteration Number: ");
//             Serial.println(iterations);
//             iterations++;
//             epoch=0;
//             initMQTT(nodeNumber); //enable MQTT
//             awaitWeights(callback, nodeNumber, !trainingDisabled);
//             if(iterations >= numberIterations || useFedAvg){
//                 if(iterations >= numberIterations){
//                     Serial.println("Training ended.");
//                     trainingDisabled = true;

//                     fclose(trainSetFile);

//                     iterationFinished=true;

//                 }else Serial.println("Waiting weights...");
//                 waitingWeights = true;
//                 sendWeights(nodeNumber, ann);
//             }
//         }
//         if (!testingDisabled && !waitingWeights && iterationFinished  && !processFinished ) {
//             fclose(trainSetFile);
//             initEstimationfiles();

//             Serial.println("Estimating Anomalies...");
//             fseek(testSetFile, 0, SEEK_SET);
//             while (readData(testSetFile, input, output)){
//                 predictAnomaly(testAnomaly, ann, input, output, bestValues);

//             }
//             fclose(testSetFile);
//             fclose(testAnomaly);

//             initValFiles();
//             Serial.println("Calculating best Thresholds");
//             fseek(valSetFile, 0, SEEK_SET);
//             while (readData(valSetFile, input, output)){
//                 predictAnomalyVal(valAnomaly, ann, input, output, bestValues);

//             }
//             fclose(valSetFile);
//             fclose(valAnomaly);



//             initestimationGlobalfiles();

//             Serial.println("Estimating Anomalies Global...");
//             fseek(testGlobalSetFile, 0, SEEK_SET);
//             while (readData(testGlobalSetFile, input, output)){
//                 predictAnomaly(testGlobalAnomaly, ann, input, output, bestValues);

//             }
//             fclose(testGlobalSetFile);
//             fclose(testGlobalAnomaly);

          
//             Serial.println("Estimating Anomalies finished");

//             processFinished=true;
            
//             experimentNumberFile = fopen(pathExperimentFile.c_str(), "w");
//             if (!experimentNumberFile) {
//                 printf("Could not open experiment number file: %s\n", pathExperimentFile.c_str());
//             } else {
//                 Serial.print("Experiment Number finished: ");
//                 Serial.println(experimentNumber);

//                 experimentNumber++; // Incrementar el número del experimento

//                 // Guardar el número como un entero
//                 fprintf(experimentNumberFile, "%d\n", experimentNumber);

//                 Serial.println("Saving new experimentNumber");
//                 fclose(experimentNumberFile);

//             }


//         }
//         mqttLoop();
//     }else {
//         if( !trainingDisabled && !waitingWeights && epoch < numberEpochs) {
//             Serial.println("trainning offline");
//             execute(epoch);
//             epoch++;
//             if(epoch >= numberEpochs ){
//                 Serial.println("Training ended.");
//                 fclose(trainSetFile);
//                 // fclose(resultFile);
//                 initEstimationfiles();
                
//                 trainingDisabled = true;
//                 if (!testingDisabled) {
//                     Serial.println("Estimating Anomalies...");
//                     for(int i=0; i<5; i++ ){
//                         int64_t startCPUTime = esp_timer_get_time();

//                         fseek(testSetFile, 0, SEEK_SET);
//                         while (readData(testSetFile, input, output)){
//                             predictAnomaly(testAnomaly, ann, input, output, bestValues);

//                         }
//                         int64_t elapsedCPUTime = esp_timer_get_time() - startCPUTime;  

//                         Serial.printf("CPU Time Used: %.3f ms\n", elapsedCPUTime / 1000.0);

//                     }
                    


//                     fclose(testSetFile);
//                     fclose(testAnomaly);
//                     initValFiles();

//                     Serial.println("Calculating Thresholds...");
//                     fseek(valSetFile, 0, SEEK_SET);
//                     while (readData(valSetFile, input, output)){
//                         predictAnomalyVal(valAnomaly, ann, input, output, bestValues);

//                     }
//                     fseek(valAnomaly, 0, SEEK_SET);
//                     fseek(labelsValFile, 0, SEEK_SET);

//                     Serial.println("Calculating Thresholds... 2");

//                     fclose(valSetFile);
//                     fclose(valAnomaly);
//                     fclose(labelsValFile);

//                     Serial.println("Estimating Anomalies finished");
//                 }

//                 fclose(trainSetFile);
//                 processFinished=true;

//                 experimentNumberFile = fopen(pathExperimentFile.c_str(), "w");
//                 if (!experimentNumberFile) {
//                     printf("Could not open experiment number file: %s\n", pathExperimentFile.c_str());
//                 } else {
//                     Serial.print("Experiment Number finished: ");
//                     Serial.println(experimentNumber);

//                     experimentNumber++; // Incrementar el número del experimento

//                     // Guardar el número como un entero
//                     fprintf(experimentNumberFile, "%d\n", experimentNumber);

//                     Serial.println("Saving new experimentNumber");
//                     fclose(experimentNumberFile);
//                     // fclose(resultFile);
//                 }

//             }

//         }
//     }

// }

void estimateAndSaveResults() {
    Serial.println("Estimating Anomalies...");
    fseek(testSetFile, 0, SEEK_SET);
    while (readData(testSetFile, input, output)) {
        predictAnomaly(testAnomaly, ann, input, output, bestValues);
    }
    fclose(testSetFile);
    fclose(testAnomaly);

    initValFiles();
    Serial.println("Calculating best Thresholds");
    fseek(valSetFile, 0, SEEK_SET);
    while (readData(valSetFile, input, output)) {
        predictAnomalyVal(valAnomaly, ann, input, output, bestValues);
    }
    fclose(valSetFile);
    fclose(valAnomaly);

    initestimationGlobalfiles();
    Serial.println("Estimating Anomalies Global...");
    fseek(testGlobalSetFile, 0, SEEK_SET);
    while (readData(testGlobalSetFile, input, output)) {
        predictAnomaly(testGlobalAnomaly, ann, input, output, bestValues);
    }
    fclose(testGlobalSetFile);
    fclose(testGlobalAnomaly);

    Serial.println("Estimating Anomalies finished");
}

void saveExperimentNumber() {
    experimentNumberFile = fopen(pathExperimentFile.c_str(), "w");
    if (!experimentNumberFile) {
        printf("Could not open experiment number file: %s\n", pathExperimentFile.c_str());
        return;
    }

    Serial.print("Experiment Number finished: ");
    Serial.println(experimentNumber);
    experimentNumber++;
    fprintf(experimentNumberFile, "%d\n", experimentNumber);
    fclose(experimentNumberFile);

    Serial.println("Saving new experimentNumber");
}


void _loop() {
    if (!offlineTraining) {
        if (!trainingDisabled && !waitingWeights && iterations < numberIterations) {
            closeConnection();
            for (int i = 0; i < numberEpochs; i++) {
                execute(epoch++);
            }

            Serial.print("Iteration Number: ");
            Serial.println(iterations++);
            epoch = 0;

            initMQTT(nodeNumber);
            awaitWeights(callback, nodeNumber, !trainingDisabled);

            if (iterations >= numberIterations || useFedAvg) {
                if (iterations >= numberIterations) {
                    Serial.println("Training ended.");
                    trainingDisabled = true;
                    fclose(trainSetFile);
                    iterationFinished = true;
                } else {
                    Serial.println("Waiting weights...");
                }
                waitingWeights = true;
                sendWeights(nodeNumber, ann);
            }
        }

        if (!testingDisabled && !waitingWeights && iterationFinished && !processFinished) {
            fclose(trainSetFile);
            initEstimationfiles();
            estimateAndSaveResults();
            processFinished = true;
            saveExperimentNumber();
        }

        mqttLoop();
    } else {
        if (!trainingDisabled && !waitingWeights && epoch < numberEpochs) {
            Serial.println("Training offline");
            execute(epoch++);

            if (epoch >= numberEpochs) {
                Serial.println("Training ended.");
                fclose(trainSetFile);
                initEstimationfiles();
                trainingDisabled = true;

                if (!testingDisabled) {
                    for (int i = 0; i < 30; i++) {
                        int64_t startCPUTime = esp_timer_get_time();
                        fseek(testSetFile, 0, SEEK_SET);
                        while (readData(testSetFile, input, output)) {
                            predictAnomaly(testAnomaly, ann, input, output, bestValues);
                        }
                        int64_t elapsedCPUTime = esp_timer_get_time() - startCPUTime;
                        Serial.printf("CPU Time Used: %.3f ms\n", elapsedCPUTime / 1000.0);
                    }

                    fclose(testSetFile);
                    fclose(testAnomaly);
                    initValFiles();
                    Serial.println("Calculating Thresholds...");

                    fseek(valSetFile, 0, SEEK_SET);
                    while (readData(valSetFile, input, output)) {
                        predictAnomalyVal(valAnomaly, ann, input, output, bestValues);
                    }

                    fseek(valAnomaly, 0, SEEK_SET);
                    fseek(labelsValFile, 0, SEEK_SET);

                    fclose(valSetFile);
                    fclose(valAnomaly);
                    fclose(labelsValFile);

                    Serial.println("Estimating Anomalies finished");
                }

                fclose(trainSetFile);
                processFinished = true;
                saveExperimentNumber();
            }
        }
    }
}

