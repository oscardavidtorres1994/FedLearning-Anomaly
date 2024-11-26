#include <Arduino.h>
#include "classification.h"
#include "mqtt.h"

// Configuración de la red neuronal
const int numberInputLayer = 11;
const int numberHiddenLayer = 1;
const int numberOutputLayer = 11;
const int numberEpochs = 2;
const int numberIterations = 2;
float learningRate = 0.1;
const float anneal = 0.995;

// Rutas de archivo
String pathTrain = "/sdcard/train.csv";
String pathResult = "/sdcard/result.csv";
String pathAnomalies = "/sdcard/anomaly.csv";
String pathBest = "/sdcard/best.csv";
String pathTest = "/sdcard/test.csv";
String pathSaveWeights = "/sdcard/weights.txt";


// Archivos y variables
FILE* trainSetFile;
FILE* resultFile;
FILE* testSetFile;
FILE* testAnomaly;
FILE* best;

float input[numberInputLayer];
float output[numberOutputLayer];
float bestValues[numberInputLayer];
genann* ann;
int epoch = 0;
int iterations=0;
bool offlineTraining=false;
bool trainingDisabled = false;
bool testingDisabled = false;
bool useTransferLearning = false;
bool useFedAvg = true;

const int nodeNumber = 1;
int weightIndex = 0;
bool waitingWeights = false;
bool iterationFinished=false;
bool processFinished=false;

// Inicialización de archivos
bool initFiles() {
    trainSetFile = fopen(pathTrain.c_str(), "r");
    if (!trainSetFile) {
        Serial.printf("Could not open file: %s\n", pathTrain.c_str());
        return false;
    }
    Serial.println("1");

    resultFile = fopen(pathResult.c_str(), "w");
    if (!resultFile) {
        Serial.println("Could not create result file.");
        fclose(trainSetFile);
        return false;
    }
    Serial.println("2");

    testSetFile = fopen(pathTest.c_str(), "r");
    if (!testSetFile) {
        Serial.println("Could not create test file.");
        fclose(trainSetFile);
        fclose(resultFile);
        return false;
    }
    Serial.println("3");

    testAnomaly = fopen(pathAnomalies.c_str(), "a");
    if (!testAnomaly) {
        Serial.println("Could not create test anomaly file.");
        fclose(trainSetFile);
        fclose(resultFile);
        fclose(testSetFile);
        return false;
    }
    Serial.println("4");

    best = fopen(pathBest.c_str(), "r");
    if (!best) {
        printf("Could not open test anomaly best threshold file.\n");
        fclose(trainSetFile);
        fclose(resultFile);
        fclose(testSetFile);
        fclose(testAnomaly);
        return false;
    } else {
        char buffer[1024];
        if (fgets(buffer, sizeof(buffer), best) == NULL) {
            printf("Error: Could not read from the file.\n");
            fclose(best);
            fclose(trainSetFile);
            fclose(resultFile);
            fclose(testSetFile);
            fclose(testAnomaly);
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
            fclose(trainSetFile);
            fclose(resultFile);
            fclose(testSetFile);
            fclose(testAnomaly);
            return false;
        }

        fclose(best);  // Cerrar el archivo después de procesarlo
    }


    Serial.println("5");


    return true;
}

// Ejecución de una época de entrenamiento y prueba
void execute(int epoch) {
    if (!trainingDisabled) {
        fseek(trainSetFile, 0, SEEK_SET);

        startTrainingTimer();
        while (readData(trainSetFile, input, output)) {
            genann_train(ann, input, output, learningRate);
        }
        printTrainingTimer(resultFile);

        learningRate *= anneal;

        fseek(trainSetFile, 0, SEEK_SET);
        resetMetrics();
        while (readData(trainSetFile, input, output))
            predict(ann, input, output);
        printResult(resultFile);

        Serial.printf("Epoch %d completed.\n", epoch + 1);
    }

    if (!testingDisabled) {
        resetMetrics();
        while (readData(testSetFile, input, output)) {
            predict(ann, input, output);
        }
        printResult(resultFile);
    }

    if (!trainingDisabled) {
        saveWeightsJson(ann, pathSaveWeights, getNumberDataset());
    }
}


void callback(const char* topic, byte* payload, unsigned int length) {
    waitingWeights = true;
    Serial.print("Receving weight: ");
    Serial.print(weightIndex+1);
    Serial.print("\\");
    Serial.print(ann->total_weights);

    ann->weight[weightIndex] = atof(reinterpret_cast<const char*>(payload));
    Serial.print("\t");
    Serial.println(ann->weight[weightIndex]);
    weightIndex++;
    if (weightIndex >= ann->total_weights) {
        //closeConnection();
        //initMQTT(nodeNumber);
        //awaitWeights(callback, nodeNumber, !trainingDisabled);
        Serial.println("All weights received");
        //closeConnection();
        //sleep(1000);
        weightIndex = 0;
        
        if(trainingDisabled && !testingDisabled)
          execute(0);
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
        loadWeightsJson(ann, "/sdcard/weights_load.txt");
        Serial.println("Transfer learning model loaded");
    }
    else genann_randomize(ann);
    // Serial.println("Starting training...");

    if(!offlineTraining){
        Serial.println("Traning online");
        Serial.println("Waiting other nodes for 30 seconds ");
        delay(1*30*1000);
    }
    
    Serial.println("starting training:");


    if(!offlineTraining){
        awaitWeights(callback, nodeNumber, !trainingDisabled);
    }   
    
}

// Ciclo de entrenamiento
// void _loop() {
//     delay(1000);
//     if (epoch < numberEpochs) {
//         execute(epoch);
//         epoch++;
//         if (epoch >= numberEpochs) {
//             Serial.println("Training completed.");
//             fclose(trainSetFile);
//             fclose(resultFile);
//         }else Serial.println("Waiting weights...");
//             waitingWeights = true;
//             sendWeights(nodeNumber, ann);
//     }
// }

void _loop() {
    if (!offlineTraining){
        if( !trainingDisabled && !waitingWeights && iterations < numberIterations){
            closeConnection(); //disable MQTT before training to save memory
            for(int i=0; i<numberEpochs; i++ ){
                execute(epoch);
                epoch++;
            }
            Serial.print("Iteration Number: ");
            Serial.println(iterations);
            iterations++;
            epoch=0;
            initMQTT(nodeNumber); //enable MQTT
            awaitWeights(callback, nodeNumber, !trainingDisabled);
            if(iterations >= numberIterations || useFedAvg){
                if(iterations >= numberIterations){
                    Serial.println("Training ended.");
                    trainingDisabled = true;
                    //resultFile.close();
                    fclose(trainSetFile);
                    iterationFinished=true;


                    //testSetFile.close();
                }else Serial.println("Waiting weights...");
                waitingWeights = true;
                sendWeights(nodeNumber, ann);
            }
        }
        if (!testingDisabled && !waitingWeights && iterationFinished  && !processFinished ) {
            Serial.println("Estimating Anomalies...");
            fseek(testSetFile, 0, SEEK_SET);
            while (readData(testSetFile, input, output)){
                predictAnomaly(testAnomaly, ann, input, output, bestValues);

            }
            fclose(testSetFile);
            fclose(testAnomaly);

            Serial.println("Estimating Anomalies finished");
            processFinished=true;
        }
        mqttLoop();
    }else {
        if( !trainingDisabled && !waitingWeights && epoch < numberEpochs) {
            execute(epoch);
            epoch++;
            if(epoch >= numberEpochs ){
                Serial.println("Training ended.");
                
                trainingDisabled = true;
                if (!testingDisabled) {
                    Serial.println("Estimating Anomalies...");
                    fseek(testSetFile, 0, SEEK_SET);
                    while (readData(testSetFile, input, output)){
                        predictAnomaly(testAnomaly, ann, input, output, bestValues);

                    }
                    fclose(testSetFile);
                    fclose(testAnomaly);
                    Serial.println("Estimating Anomalies finished");
                }

                fclose(trainSetFile);
                fclose(trainSetFile);

            }

        }
    }

}
