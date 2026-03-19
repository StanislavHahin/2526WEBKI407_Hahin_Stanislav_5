#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>

// ===== Wi-Fi параметри =====
const char* ssid = "WifiLink32";
const char* password = "12353355122";

WebServer httpServer(80);

// ===== Відправка HTML сторінки =====
void sendMainPage() {

  if(!LittleFS.exists("/page.html")){
    httpServer.send(404,"text/plain","HTML file missing");
    return;
  }

  File page = LittleFS.open("/page.html","r");
  httpServer.streamFile(page,"text/html");
  page.close();
}

// ===== Setup =====
void setup() {

  Serial.begin(115200);

  // Ініціалізація файлової системи
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS mount error");
    return;
  }

  // Підключення Wi-Fi
  WiFi.begin(ssid,password);

  Serial.print("Connecting");

  while(WiFi.status() != WL_CONNECTED){
    delay(400);
    Serial.print("*");
  }

  Serial.println();
  Serial.println("Connected");
  Serial.print("Device IP: ");
  Serial.println(WiFi.localIP());

  // Маршрут сервера
  httpServer.on("/", sendMainPage);

  httpServer.begin();
  Serial.println("Web server active");
}

void loop() {

  httpServer.handleClient();

}