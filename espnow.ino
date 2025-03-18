#include <esp_now.h>
#include <WiFi.h>

// Define relay pins (not used in this code, but kept for reference)
#define RELAY_1_PIN 3
#define RELAY_2_PIN 10
#define RELAY_3_PIN 19
#define RELAY_4_PIN 5

// Hardcoded MAC address of the relay controller
uint8_t relayControllerMacAddress[] = {0x34, 0x85, 0x18, 0x17, 0xF6, 0x68}; // MAC address of the relay controller

// Structure to send data
typedef struct {
  uint8_t relayNumber; // Relay number (1, 2, 3, or 4)
  uint8_t state;       // 0 = OFF, 1 = ON
} RelayCommand;

// UART communication pins
#define UART_RX 4  // Connect to TX of WiFi MQTT ESP32-C3
#define UART_TX 5  // Connect to RX of WiFi MQTT ESP32-C3

// Callback when data is sent via ESP-NOW
void OnDataSent(const uint8_t *mac, esp_now_send_status_t status) {
  char logMessage[50];
  snprintf(logMessage, sizeof(logMessage), "Data sent to %02X:%02X:%02X:%02X:%02X:%02X, Status: %s",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failed");
  Serial.println(logMessage);
}

void setup() {
  // Initialize Serial for logging
  Serial.begin(115200);
  Serial.println("Gateway ESP32-C3 (ESP-NOW) Initializing...");

  // Initialize WiFi in STA mode
  WiFi.mode(WIFI_STA);
  WiFi.channel(1); // Set WiFi channel to 1
  Serial.print("Gateway ESP32-C3 MAC: ");
  Serial.println(WiFi.macAddress());

  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Register callback for sending data
  esp_now_register_send_cb(OnDataSent);

  // Add peer (relay controller)
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, relayControllerMacAddress, 6);
  peerInfo.channel = 1; // Match the WiFi channel
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }

  // Initialize UART for communication with WiFi MQTT ESP32-C3
  Serial1.begin(115200, SERIAL_8N1, UART_RX, UART_TX);
  Serial.println("UART initialized for communication with WiFi MQTT ESP32-C3");

  Serial.println("Gateway ESP32-C3 (ESP-NOW) Ready!");
}

void loop() {
  // Check for incoming UART data from WiFi MQTT ESP32-C3
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();

    // Parse the command (format: <relayNumber> <state>)
    int relayNumber = command.substring(0, command.indexOf(' ')).toInt();
    int state = command.substring(command.indexOf(' ') + 1).toInt();

    if (relayNumber >= 1 && relayNumber <= 4 && (state == 0 || state == 1)) {
      RelayCommand relayCommand;
      relayCommand.relayNumber = relayNumber;
      relayCommand.state = state;

      // Send command to relay controller via ESP-NOW
      esp_err_t result = esp_now_send(relayControllerMacAddress, (uint8_t *)&relayCommand, sizeof(relayCommand));

      if (result == ESP_OK) {
        Serial.println("Command sent to relay controller via ESP-NOW!");
      } else {
        Serial.println("Error sending command via ESP-NOW!");
      }
    } else {
      Serial.println("Invalid UART command received!");
    }
  }
}
