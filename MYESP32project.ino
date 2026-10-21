#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <Chirale_TensorFlowLite.h>
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include "model_data.h" // Contains g_model and g_model_len

// --- DHT22 Pin & Type Setup ---
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// --- Wi-Fi & MQTT Credentials ---
const char* WIFI_SSID   = "Padmanabhan 2.4G";         // Replace with your Wi-Fi name
const char* WIFI_PASS   = "9566775418";     // Replace with your Wi-Fi password
const char* MQTT_BROKER = "broker.hivemq.com";
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "mohan/edge_ai/dht22_9823"; // Make sure this matches your index.html

WiFiClient espClient;
PubSubClient mqttClient(espClient);

// --- TFLM Variables ---
constexpr int kTensorArenaSize = 30 * 1024;
uint8_t tensor_arena[kTensorArenaSize];

const tflite::Model* model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

void setupWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
}

void setupModel() {
  model = tflite::GetModel(g_model);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    return;
  }

  static tflite::AllOpsResolver resolver;
  static tflite::MicroInterpreter static_interpreter(
      model, resolver, tensor_arena, kTensorArenaSize);
  interpreter = &static_interpreter;

  if (interpreter->AllocateTensors() != kTfLiteOk) {
    Serial.println("AllocateTensors() failed! Increase kTensorArenaSize.");
    return;
  }

  input = interpreter->input(0);
  output = interpreter->output(0);
  Serial.println("INT8 Model Loaded Successfully!");
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    String clientId = "ESP32_DHT22_" + String(random(0xffff), HEX);
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setupWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  setupModel();
}

void loop() {
  if (!mqttClient.connected()) {
    reconnectMQTT();
  }
  mqttClient.loop();

  // 1. Read DHT22 Sensor Data
  float humidity = dht.readHumidity();
  float temp_c = dht.readTemperature();

  // Check if sensor readings are valid
  if (isnan(humidity) || isnan(temp_c)) {
    Serial.println("Failed to read from DHT22 sensor!");
    delay(2000);
    return;
  }

  // 2. Feed Sensor Data to INT8 Model Input
  // NOTE: Adjust indexing if your model expects multiple features (e.g., [temp, humidity])
  float raw_val = temp_c; 
  int8_t quantized_in = (int8_t)(raw_val / input->params.scale + input->params.zero_point);
  input->data.int8[0] = quantized_in;

  // If your model has 2 inputs (e.g., index 0 = Temp, index 1 = Humidity):
  // if (input->bytes >= 2) {
  //   input->data.int8[1] = (int8_t)(humidity / input->params.scale + input->params.zero_point);
  // }

  // 3. Execute Local Edge AI Inference
  float confidence = 0.0f;
  if (interpreter->Invoke() == kTfLiteOk) {
    int8_t raw_out = output->data.int8[0];
    confidence = (raw_out - output->params.zero_point) * output->params.scale;
  }

  // 4. Create Payload JSON
  char payload[180];
  snprintf(payload, sizeof(payload),
           "{\"temp\":%.1f, \"humidity\":%.1f, \"confidence\":%.2f}",
           temp_c, humidity, confidence);

  // 5. Publish to Cloud
  mqttClient.publish(MQTT_TOPIC, payload);
  Serial.print("Published to Cloud: ");
  Serial.println(payload);

  // DHT22 sampling rate requires at least 2-second intervals
  delay(2000); 
}