#include <Arduino.h>
#include "classification.h"
#include "mqtt.h"

// Configuración de la red neuronal
const int numberInputLayer = 11;
const int numberHiddenLayer = 1;
const int numberOutputLayer = 11;
const int numberEpochs = 2;
float learningRate = 0.1;
const float anneal = 0.995;

// Rutas de archivo
String pathTrain = "/sdcard/train.csv";
String pathResult = "/sdcard/result.csv";
String pathTest = "/sdcard/test.csv";
String pathSaveWeights = "/sdcard/weights.txt";

// Archivos y variables
FILE* trainSetFile;
FILE* resultFile;
FILE* testSetFile;
float input[numberInputLayer];
float output[numberOutputLayer];
genann* ann;
int epoch = 0;
bool trainingDisabled = false;
bool testingDisabled = false;
bool useTransferLearning = false;
bool useFedAvg = true;

const int nodeNumber = 1;
int weightIndex = 0;
bool waitingWeights = false;

// Inicialización de archivos
bool initFiles() {
    trainSetFile = fopen(pathTrain.c_str(), "r");
    if (!trainSetFile) {
        Serial.printf("Could not open file: %s\n", pathTrain.c_str());
        return false;
    }

    resultFile = fopen(pathResult.c_str(), "w");
    if (!resultFile) {
        Serial.println("Could not create result file.");
        fclose(trainSetFile);
        return false;
    }

    testSetFile = fopen(pathTest.c_str(), "r");
    if (!testSetFile) {
        Serial.println("Could not create test file.");
        fclose(trainSetFile);
        fclose(resultFile);
        return false;
    }

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
    initMQTT(nodeNumber);

    
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

    ann = genann_init(numberInputLayer, 2, numberHiddenLayer, numberOutputLayer);
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

    Serial.println("Waiting other nodes for 1 minute...");
    delay(1*60*1000);
    Serial.println("starting training:");
       
    awaitWeights(callback, nodeNumber, !trainingDisabled);
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
    if(!trainingDisabled && !waitingWeights && epoch < numberEpochs){
        closeConnection(); //disable MQTT before training to save memory
        execute(epoch);
        epoch++;
        initMQTT(nodeNumber); //enable MQTT
        awaitWeights(callback, nodeNumber, !trainingDisabled);
        if(epoch >= numberEpochs || useFedAvg){
            if(epoch >= numberEpochs){
                Serial.println("Training ended.");
                trainingDisabled = true;
                //resultFile.close();
                fclose(trainSetFile);
                //testSetFile.close();
            }else Serial.println("Waiting weights...");
            waitingWeights = true;
            sendWeights(nodeNumber, ann);
        }
    }
    mqttLoop();
}
