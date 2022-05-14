#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ShiftRegister74HC595.h>
#include <ArduinoJson.h>

WiFiManager wifiManager;
ESP8266WebServer server(80);

// numberOfShiftRegisters
// serialDataPin
// clockPin
// latchPin
ShiftRegister74HC595<1> sr(5, 4, 14);

int relayState = LOW;

int sensorPin = A0;
int sensorValue;

void setup() {
  Serial.begin(115200);

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
  if(server.hasArg("plain")) { // body will be json
    String json = server.arg("plain");
    DynamicJsonDocument req(1024);
    DynamicJsonDocument res(1024);
    String response;

    deserializeJson(req, json);

    if (req["relayNumber"] == 0 ||
        req["relayNumber"] &&
        req["relayNumber"] < 8 &&
        req["relayNumber"] >= 0) { // check if body is valid
      int relayNumber = req["relayNumber"];
      int currentState = sr.get(relayNumber); // check current state of relay
      int newState = !currentState; // set new state to opposite of current state

      res["relayNumber"] = relayNumber;
      res["currentState"] = newState;
      serializeJson(res, response);

      sr.set(relayNumber, newState);
      server.send(200, "application/json", response); // send back json with new state
      return;
    } else {
      server.send(400, "text/plain", "Invalid Request");
    }
  } else {
    server.send(400, "text/plain", "Invalid Request");
  }
}

void handleReadSensor() {
  sensorValue = analogRead(sensorPin);

  String msg = "Sensor value: ";
  msg = msg + String(sensorValue);
  Serial.println(msg);

  String json = "{ \"sensor reading\": \"";
  json = json + String(sensorValue);
  json = json + "\" }";

  server.send(200, "application/json", json);
}