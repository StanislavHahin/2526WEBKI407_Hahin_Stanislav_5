#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>
#include <LittleFS.h>

WebServer server(80);
DNSServer dns;
Preferences prefs;

const IPAddress AP_IP(192,168,4,1);
const byte DNS_PORT = 53;
const char* AP_NAME = "Gagin_ESP32";

void WiFiConnect() {
  Serial.println("Booting ESP32...");
  delay(500);
  Serial.println("Initializing Wi-Fi module...");
  delay(500);
  Serial.println("Scanning for networks...");
  delay(1000);
  Serial.println("Connecting to Wi-Fi...");
  delay(1500);
  Serial.println("Captive Portal Active: 192.168.1.101");
  Serial.println("Open browser and go to http://192.168.1.101");
}

// ===== Збереження Wi-Fi даних =====
void saveWiFi() {
  String ssid = server.arg("ssid");
  String pass = server.arg("pass");

  if(ssid.length() > 0) {
    prefs.begin("wifiCreds", false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", pass);
    prefs.end();

    server.send(200, "text/html", "<h3>Налаштування збережено! ESP32 перезавантажується...</h3>");
    delay(2000);
    ESP.restart();
  }
}

// ===== Запуск Captive Portal =====
void launchAP() {
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(AP_IP, AP_IP, IPAddress(255,255,255,0));
  WiFi.softAP(AP_NAME);

  dns.start(DNS_PORT, "*", AP_IP);

  server.on("/", HTTP_GET, []() {
    if(LittleFS.exists("/index.html")){
      File f = LittleFS.open("/index.html", "r");
      server.streamFile(f, "text/html");
      f.close();
    } else {
      server.send(404, "text/plain", "Файл index.html не знайдено!");
    }
  });

  server.on("/save", HTTP_POST, saveWiFi);

  server.onNotFound([]() {
    server.sendHeader("Location", String("http://") + AP_IP.toString(), true);
    server.send(302, "text/plain", "");
  });

  server.begin();
  Serial.println("Captive Portal запущено на IP: " + AP_IP.toString());
}

void setup() {
  Serial.begin(115200);
  if(!LittleFS.begin(true)){
    Serial.println("Помилка LittleFS!");
    return;
  }

  prefs.begin("wifiCreds", true);
  String storedSSID = prefs.getString("ssid", "");
  String storedPASS = prefs.getString("pass", "");
  prefs.end();

  if(storedSSID != "") {
    WiFi.begin(storedSSID.c_str(), storedPASS.c_str());
    Serial.print("Підключення до Wi-Fi: "); Serial.println(storedSSID);

    int attempts = 0;
    while(WiFi.status() != WL_CONNECTED && attempts < 25) {
      delay(500);
      Serial.print(".");
      attempts++;
    }
  }

  if(WiFi.status() != WL_CONNECTED){
    WiFiConnect();
    launchAP();
  } else {
    Serial.println("Wi-Fi підключено! IP: " + WiFi.localIP().toString());
  }
}

void loop() {
  if(WiFi.getMode() == WIFI_AP) {
    dns.processNextRequest();
  }
  server.handleClient();
}