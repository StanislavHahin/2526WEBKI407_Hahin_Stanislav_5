#include <Arduino.h>
#include <WiFi.h>
#include <HTTPSServer.hpp>
#include <SSLCert.hpp>
#include <HTTPRequest.hpp>
#include <HTTPResponse.hpp>

using namespace httpsserver;

// Wi-Fi дані
const char* ssid = "WiFiLink32";
const char* password = "987654321d";

// Сертифікат і сервер
SSLCert cert;
HTTPSServer httpsServer(&cert);

// Обробка кореневого запиту
void handleRoot(HTTPRequest* req, HTTPResponse* res) {
    // Простий Basic Auth: admin:1234 -> YWRtaW46MTIzNA==
    if (req->getHeader("Authorization") != "Basic YWRtaW46MTIzNA==") {
        res->setStatusCode(401);
        res->setHeader("WWW-Authenticate", "Basic realm=\"ESP32-Secure\"");
        res->print("Доступ заборонено!");
        return;
    }

    res->setHeader("Content-Type", "text/html; charset=utf-8");
    res->print("<!DOCTYPE html><html lang='uk'><head><meta charset='UTF-8'><title>ESP32 HTTPS Dashboard</title></head>");
    res->print("<body style='font-family:sans-serif; background:#0a0a0a; color:#00ffcc; display:flex; justify-content:center; align-items:center; height:100vh;'>");
    res->print("<div style='text-align:center; border:2px solid #00ffcc; padding:30px; border-radius:20px;'>");
    res->print("<h1>Вітаємо, Адмін!</h1>");
    res->print("<p>Сервер ESP32 працює через HTTPS</p>");
    res->print("<p>Студент: Гагін С.Я.</p>");
    res->print("</div></body></html>");
}

void setup() {
    Serial.begin(115200);

    WiFi.begin(ssid, password);
    Serial.print("Підключення до Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi підключено!");
    Serial.println("IP: " + WiFi.localIP().toString());

    // Генерація самопідписаного сертифіката
    if (createSelfSignedCert(cert, KEYSIZE_2048, "CN=esp32.local,O=Lab,C=UA") != 0) {
        Serial.println("Помилка сертифіката!");
        return;
    }

    ResourceNode* rootNode = new ResourceNode("/", "GET", &handleRoot);
    httpsServer.registerNode(rootNode);

    httpsServer.start();
    if (httpsServer.isRunning()) {
        Serial.println("HTTPS сервер запущено на порті 443");
    }
}

void loop() {
    httpsServer.loop();
}