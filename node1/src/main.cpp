#include <Arduino.h>
#include "health.h"
#include <esp_heap_caps.h>


void setup() {
    Serial.begin(115200);

    if (psramFound()) {
      size_t psram_size = esp_spiram_get_size();
        Serial.println(psram_size);
        Serial.println("PSRAM está habilitada.");
        Serial.printf("Total PSRAM: %u bytes\n", ESP.getPsramSize());
        Serial.printf("PSRAM libre: %u bytes\n", ESP.getFreePsram());
    } else {
        Serial.println("PSRAM no está habilitada o disponible.");
    }

    Serial.printf("RAM interna libre: %u bytes\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    trainingMode();
}

void loop() {
    // Código principal que se ejecutará repetidamente
    _loop();
}