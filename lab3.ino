#include <Arduino.h>
#include <Wire.h>
#include <BH1750.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>

// ===== Налаштування Wi-Fi =====
const char* ssid = "WifiLink32";
const char* password = "2039120392d";

// ===== Ініціалізація BH1750 =====
BH1750 lightMeter;

// ===== WebSocket Server =====
WebSocketsServer webSocket = WebSocketsServer(81);

// ===== Період відправки даних =====
unsigned long lastSend = 0;
const unsigned long sendInterval = 1000; // 1 секунда

void handleWebSocketMessage(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
}

void setup() {
  Serial.begin(115200);

  // Ініціалізація LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("Помилка LittleFS!");
    return;
  }

  WiFi.begin(ssid, password);
  Serial.print("Підключення до Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi підключено! IP: 192.168.1.101");

  // Ініціалізація BH1750
  Wire.begin();
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println("Помилка ініціалізації BH1750!");
  } else {
    Serial.println("BH1750 готовий до роботи");
  }

  webSocket.begin();
  webSocket.onEvent(handleWebSocketMessage);

  Serial.println("WebSocket сервер запущено на порту 80");
}

void loop() {
  webSocket.loop();

  unsigned long now = millis();
  if (now - lastSend > sendInterval) {
    lastSend = now;

    // Зчитування освітленості
    float lux = lightMeter.readLightLevel();
    if (lux < 0) lux = 0; // BH1750 повертає -1 при помилці

    // Формування JSON
    String json = "{\"lux\":" + String(lux, 2) + "}";

    // Відправка всім клієнтам
    webSocket.broadcastTXT(json);

    Serial.println("Відправлено: " + json);
  }
}