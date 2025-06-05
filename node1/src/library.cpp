#include "library.h"
#include <Arduino.h>
#include <dirent.h>

int _inputLayers = 0;
int _outputLayers = 0;

const int lineCharLength = 30;
char line[lineCharLength];

void init_sd_card() {
    Serial.println("Initializing SD card...");
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = true,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_config.width = 4;
    slot_config.clk = GPIO_NUM_39;  // Pines configurados para el ESP32-S3
    slot_config.cmd = GPIO_NUM_40;
    slot_config.d0 = GPIO_NUM_47;
    slot_config.d1 = GPIO_NUM_21;
    slot_config.d2 = GPIO_NUM_42;
    slot_config.d3 = GPIO_NUM_41;

    sdmmc_card_t *card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            Serial.println("Failed to mount filesystem. If you want the card to be formatted, set .format_if_mount_failed = true.");
        } else {
            Serial.printf("Failed to initialize the card (%s).\n", esp_err_to_name(ret));
        }
        return;
    }

    sdmmc_card_print_info(stdout, card);
    Serial.println("SD card mounted successfully.");
    listDir("/sdcard");
}



void listDir(const char *dirname) {
    Serial.print("Listing directory: ");
    Serial.println(dirname);

    DIR *dir = opendir(dirname);
    if (dir == nullptr) {
        Serial.print("Failed to open directory: ");
        Serial.println(dirname);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            Serial.print("DIR : ");
            Serial.println(entry->d_name);
        } else {
            Serial.print("FILE: ");
            Serial.println(entry->d_name);
        }
    }
    closedir(dir);
}


bool readData(FILE* in, float* input, float* output) {
    // Variables temporales
    char line[256]; // Buffer para leer una línea completa
    int i = 0;

    // Verificar si el archivo tiene datos
    if (fgets(line, sizeof(line), in) == nullptr) {
        return false; // Fin de archivo o error
    }

    // Tokenizar la línea leída usando comas
    char* token = strtok(line, ",");
    while (token != nullptr && i < _inputLayers) {
        // Serial.println(token);
        // Convertir cada token en un valor flotante y asignarlo a input
        input[i] = atof(token);
        i++;
        token = strtok(nullptr, ",");
    }

    // Si el número de valores leídos es menor que el tamaño de entrada esperado
    if (i != _inputLayers) {
        Serial.println("Error: número inesperado de columnas en una línea del CSV");
        return false;
    }

    // Copiar el input a output para el autoencoder
    for (int j = 0; j < _inputLayers; j++) {
        output[j] = input[j];
    }

    return true; // Lectura de línea completa y copia a output
}




bool readDataVal(FILE* in, FILE* labels, float* input, float* output) {
    char line[256];   // Buffer para leer una línea completa del archivo de MSE
    char line2[256];  // Buffer para leer una línea completa del archivo de etiquetas
    int i = 0, j = 0;

    // Verificar si hay datos disponibles en ambos archivos
    if (fgets(line, sizeof(line), in) == nullptr) {
        return false; // Fin de archivo o error
    }

    if (fgets(line2, sizeof(line2), labels) == nullptr) {
        return false; // Fin de archivo o error
    }

    // Leer valores MSE
    char* token = strtok(line, ",");
    while (token != nullptr && i < _inputLayers) {
        input[i] = atof(token);  // Convertir a float
        i++;
        token = strtok(nullptr, ",");
    }

    // Leer etiquetas y convertir "True"/"False" a 1.0/0.0
    char* token2 = strtok(line2, ",");
    while (token2 != nullptr && j < _inputLayers) {
        // Eliminar saltos de línea o espacios extra
        while (*token2 == ' ' || *token2 == '\n' || *token2 == '\r') token2++;

        if (strncmp(token2, "True", 4) == 0) {
            output[j] = 1.0f;
        } else if (strncmp(token2, "False", 5) == 0) {
            output[j] = 0.0f;
        } else {
            Serial.println("Error: etiqueta no válida en archivo de etiquetas");
            return false;
        }

        j++;
        token2 = strtok(nullptr, ",");
    }

    if (i != _inputLayers) {
        Serial.println("Error: número inesperado de columnas en archivo de MSE");
        return false;
    }

    if (j != _inputLayers) {
        Serial.println("Error: número inesperado de columnas en archivo de etiquetas");
        return false;
    }

    return true;
}




void initDataframe(int inputLayers, int outputLayers) {

  _inputLayers = inputLayers;
  _outputLayers = outputLayers;


}


void saveWeightsJson(genann const* ann, String path, int items) {
    // Convertir el String a un const char* para fopen
    const char* filePath = path.c_str();

    // Abrir el archivo en modo escritura
    FILE* file = fopen(filePath, "w");
    if (!file) {
        Serial.println("Could not open file: weights");
        return;
    }

    // Escribir la cantidad de items
    fprintf(file, "%d\n", items);

    // Escribir cada peso en una línea
    for (int i = 0; i < ann->total_weights; ++i) {
        fprintf(file, "%.20f\n", ann->weight[i]);
    }

    // Cerrar el archivo
    fclose(file);
}




void loadWeightsJson(genann* ann, const String& path) {
    // Convertir el String a un const char* para fopen
    const char* filePath = path.c_str();

    // Abrir el archivo en modo lectura
    FILE* file = fopen(filePath, "r");
    if (!file) {
        Serial.println("Could not open weights file");
        return;
    }

    // Variables para manejar los datos
    char line[64]; // Buffer para leer cada línea
    int items;
    float accuracy;

    // Leer y omitir las dos primeras líneas (items y accuracy)
    if (fgets(line, sizeof(line), file)) {
        items = atoi(line); // Leer items (aunque no se usa, solo se salta)
    }

    // Leer cada peso en una línea y asignarlo a ann->weight
    int i = 0;
    while (fgets(line, sizeof(line), file) && i < ann->total_weights) {
        ann->weight[i++] = atof(line); // Convertir la línea a float y asignar a weight
    }

    // Cerrar el archivo
    fclose(file);
    Serial.println("Weights loaded successfully");
}

