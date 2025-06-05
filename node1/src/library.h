#ifndef LIBRARY_H
#define LIBRARY_H

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "genann.h"
#include <SPI.h>
#include <SD.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "esp_err.h"

// Declaración de funciones para el entrenamiento y manejo de datos
void initDataframe(int inputLayers, int outputLayers);
bool readData(FILE* in, float* input, float* output);
bool readDataVal(FILE* in, FILE* labels, float* input, float* output);
void saveWeightsJson(genann const* ann, String path, int items);
void loadWeightsJson(genann* ann, const String& path);
void init_sd_card(); 
void listDir(const char *dirname); // Cambiado de initSdCard() a init_sd_card() para usar el nuevo método

#endif /* LIBRARY_H */
