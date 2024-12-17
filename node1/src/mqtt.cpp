#include "mqtt.h"
#include <WiFi.h>

//#define MQTT_USER  "smart_lock"
//#define MQTT_PASSWORD "123456789"

// WiFi
const char* ssid = "DavidPhone2";     // Enter your WiFi name
const char* password = "D@vid3284";  // Enter WiFi password
char path[10];
char char_array[30];
const bool MQTT_DISABLED = false;

// MQTT Broker
// const char* mqtt_broker = "192.168.154.203";
const char* mqtt_broker = "192.168.1.101";
//const int mqtt_port = 1606;


WiFiClient espClient;
PubSubClient client(espClient);

const int uniqueId = rand();

void connectBroker(int node) {
  //Serial.println("Connecting MQTT broker:");
  //Serial.print(mqtt_broker);
  //Serial.println(" ...")
  if(MQTT_DISABLED) return;

  client.setServer(mqtt_broker, 1883);  //, mqtt_port);
  // Loop until we're reconnected
  while (!client.connected()) {
    // Generate a unique id
    String id = "node-"+ String(node)+"-"+String(uniqueId);
    if (client.connect(id.c_str())) {
    } else {
      Serial.print("MQTT connection failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 1 second");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
  Serial.println("Connected.");
}

void closeConnection() {
  //espClient.stop();
  //espClient.flush();
  if(MQTT_DISABLED) return;
  client.disconnect();
  //client.unsubscribe("/aggregate");
}


void initMQTT(int node) {
  if(MQTT_DISABLED) return;
  delay(1000);

  //WiFi.mode(WIFI_STA);  //Optional
  WiFi.begin(ssid, password);
  //Serial.println("\nConnecting");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }

  Serial.println("\nConnected to the WiFi network");
  //Serial.print("Local ESP32 IP: ");
  //Serial.println(WiFi.localIP());
  connectBroker(node);
  //client.setCallback(callback);
  //client.subscribe(topic);
}

// void initMQTT(int node) {
//   if(MQTT_DISABLED) return;
//   delay(1000);

//   WiFi.begin(ssid, password);
//   Serial.println("\nConectando a la red WiFi");

//   unsigned long startAttemptTime = millis();

//   // Espera hasta que se conecte al WiFi o pase un tiempo
//   while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
//     Serial.print(".");
//     delay(100);
//   }

//   if (WiFi.status() != WL_CONNECTED) {
//     Serial.println("\nNo se pudo conectar a la red WiFi.");
//     return;
//   }

//   Serial.println("\nConectado a la red WiFi");
//   Serial.print("Dirección IP local del ESP32: ");
//   Serial.println(WiFi.localIP());

//   connectBroker(node);
// }

void mqttLoop() {
  client.loop();
}

// void sendWeights(int node, genann const* ann) {
//   if(MQTT_DISABLED) return;

//   sprintf(path, "/%d", node);
//   Serial.println("Sending weights...");

//   for (int i = 0; i < ann->total_weights; ++i) {
//     Serial.print("Sending weight: ");
//     Serial.print(i+1);
//     Serial.print("\\");
//     Serial.println(ann->total_weights);
//     //sprintf(char_array, "%d,%f", i+2,ann->weight[i]);
//     sprintf(char_array, "%f", ann->weight[i]);
//     //Serial.printf("wei-%d\n",i+2);
//     client.publish("weights/iiottest", char_array);
//     delay(200);
//   }

//   client.publish("weights/iiottest", "END");
//   Serial.println("Weights sent");
//   //free(path);
//   //free(char_array);
// }

void sendWeights(int node, genann const* ann) {
  if(MQTT_DISABLED) return;

  sprintf(path, "/%d", node);
  Serial.println("Sending weights...");

  for (int i = 0; i < ann->total_weights; ++i) {
    Serial.print("Sending weight: ");
    Serial.print(path);
    Serial.print(": ");
    Serial.print(i+1);
    Serial.print("\\");
    Serial.println(ann->total_weights);
    //sprintf(char_array, "%d,%f", i+2,ann->weight[i]);
    sprintf(char_array, "%f", ann->weight[i]);
    //Serial.printf("wei-%d\n",i+2);
    client.publish(path, char_array);
    delay(20);
  }

  client.publish(path, "END");
  Serial.println("Weights sent");
  //free(path);
  //free(char_array);
}



void awaitWeights(void (*callback)(const char*, uint8_t*, unsigned int), int node, bool train) {
  if(MQTT_DISABLED) return;
  if(train){
    sprintf(path, "%d", node);
    Serial.println("Registering as training node");
    client.publish("/add-participant", path);
  }else client.publish("/get-weights", "0");

  client.subscribe("/receive-weights");
  client.setCallback(callback);
  //client.setKeepAlive(65535);
  //free(topic);
}


