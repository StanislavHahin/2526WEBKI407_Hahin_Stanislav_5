#include <WiFi.h>
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

const char* ssid = "WifiLink32";
const char* password = "325431235";

WebServer server(80);

const int LED = 2;
bool ledState = false;


// -------- API статусу ----------
void apiStatus(){

  StaticJsonDocument<100> doc;
  doc["state"] = ledState;

  String json;
  serializeJson(doc, json);

  server.send(200,"application/json",json);

}


// -------- API керування ----------
void apiControl(){

  if(!server.hasArg("plain")){
    server.send(400,"text/plain","No data");
    return;
  }

  String body = server.arg("plain");

  StaticJsonDocument<200> doc;
  DeserializationError err = deserializeJson(doc, body);

  if(err){
    server.send(400,"text/plain","JSON error");
    return;
  }

  String cmd = doc["cmd"];

  if(cmd == "on"){
    ledState = true;
  }

  if(cmd == "off"){
    ledState = false;
  }

  digitalWrite(LED, ledState);

  server.send(200,"application/json","{\"ok\":true}");

}


// -------- головна сторінка ----------
void handleRoot(){
  File file = LittleFS.open("/index.html","r");
  if(!file){
    server.send(404,"text/plain","index.html missing");
    return;
  }
  server.streamFile(file,"text/html");
  file.close();
}
void setup(){
  Serial.begin(115200);
  pinMode(LED,OUTPUT);
  digitalWrite(LED,LOW);
  // LittleFS
  if(!LittleFS.begin(true)){
    Serial.println("LittleFS error");
    return;
  }
  Serial.println("LittleFS mounted");
  // WiFi
  WiFi.begin(ssid,password);
  Serial.print("Connecting");
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  // routes
  server.on("/",handleRoot);

  server.on("/api/status",HTTP_GET,apiStatus);

  server.on("/api/control",HTTP_POST,apiControl);

  server.begin();

  Serial.println("Server started");

}


void loop(){

  server.handleClient();

}