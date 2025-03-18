#include <WiFi.h>
#include <PubSubClient.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT broker details
const char* mqttServer = "MQTT_BROKER_IP";
const int mqttPort = 1883;
const char* mqttUser = "MQTT_USERNAME";
const char* mqttPassword = "MQTT_PASSWORD";

// MQTT topics
const char* mqttCommandTopic = "home/relay/command";

// UART communication pins
#define UART_RX 6  // Connect to TX of Gateway ESP32-C3
#define UART_TX 7  // Connect to RX of Gateway ESP32-C3

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Function to handle incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // Convert payload to String
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

  // Forward the message to Gateway ESP32-C3 via UART
  Serial1.println(message);
  Serial.print("Forwarded to Gateway ESP32-C3: ");
  Serial.println(message);
}

void setup() {
  // Initialize Serial for logging
  Serial.begin(115200);
  Serial.println("WiFi MQTT ESP32-C3 Initializing...");

  // Initialize UART for communication with Gateway ESP32-C3
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  Serial.println("UART initialized for communication with Gateway ESP32-C3");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected!");

  // Connect to MQTT broker
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(mqttCallback);
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT broker...");
    if (mqttClient.connect("WiFiMQTTESP32C3", mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT broker!");
      mqttClient.subscribe(mqttCommandTopic);
    } else {
      Serial.print("Failed to connect to MQTT broker, retrying...");
      delay(2000);
    }
  }

  Serial.println("WiFi MQTT ESP32-C3 Ready!");
}

void loop() {
  // Maintain MQTT connection
  if (!mqttClient.connected()) {
    while (!mqttClient.connected()) {
      Serial.println("Reconnecting to MQTT broker...");
      if (mqttClient.connect("WiFiMQTTESP32C3", mqttUser, mqttPassword)) {
        Serial.println("Reconnected to MQTT broker!");
        mqttClient.subscribe(mqttCommandTopic);
      } else {
        delay(2000);
      }
    }
  }
  mqttClient.loop();
}
