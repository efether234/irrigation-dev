#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

WiFiManager wifiManager;
ESP8266WebServer server(80);

int relayPin = D1; //GPIO5
int relayState = LOW;

int sensorPin = A0;
int sensorValue;

void setup() {
  Serial.begin(115200);

  pinMode(relayPin, OUTPUT);

  wifiManager.autoConnect("IrrigationSetup", "password");

  Serial.println("Wifi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/toggleRelay", handleToggleRelay);
  server.on("/readSensor", handleReadSensor);

  server.begin();
}

void loop () {
  server.handleClient();
}

void handleToggleRelay() {
  relayState = !relayState;
  digitalWrite(relayPin, relayState);

  String msg = "Relay state: ";
  if(relayState == 1) {
    msg = msg + "open";
  } else {
    msg = msg + "closed";
  }
  Serial.println(msg);

  String json = "{ \"relay status\": \"";
  json = json + String(relayState);
  json = json + "\" }";

  server.send(200, "text/json", json);
}

void handleReadSensor() {
  sensorValue = analogRead(sensorPin);

  String msg = "Sensor value: ";
  msg = msg + String(sensorValue);
  Serial.println(msg);

  String json = "{ \"sensor reading\": \"";
  json = json + String(sensorValue);
  json = json + "\" }";

  server.send(200, "text/json", json);
}