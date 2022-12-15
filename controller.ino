#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ShiftRegister74HC595.h>
#include <ArduinoJson.h>

/* Mux control pins */
int s0 = 0;  // D3
int s1 = 2;  // D4
int s2 = 12; // D6
int s3 = 13; // D7

/* Sensor read pin */
int sensorPin = A0;

WiFiManager wifiManager;
ESP8266WebServer server(80);
//ShiftRegister74HC595<1> sr(4, 5, 14); // D2, D1, D5
ShiftRegister74HC595<1> sr(15, 5, 4); // D8, D1, D2

void setup() {
  /* Setup mux pins */
  pinMode(s0, OUTPUT);
  pinMode(s1, OUTPUT);
  pinMode(s2, OUTPUT);
  pinMode(s3, OUTPUT);

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

void setupMux() {
  
}

/* This route handler toggles the relays on and off. */
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

/* This route handler sends sensor data back to the client */
void handleReadSensor() {
  if(server.arg("sensor") == "" ||
     server.arg("sensor").toInt() < 0 ||
     server.arg("sensor").toInt() > 15) { // invalid parameter
    server.send(400, "text/plain", "Invalid request");
    return;
  }
  String channel = server.arg("sensor");
  int sensorValue = readMux(channel.toInt());
  server.send(200, "text/plain", String(sensorValue));
}

int readMux(int channel) {
  /* Add control pins to an array so we can loop through them */
  int controlPin[] = {s0, s1, s2, s3};

  /* Array of decimal to binary values to look up mux channels */
  int muxChannel[16][4]={
    {0,0,0,0}, //channel 0
    {1,0,0,0}, //channel 1
    {0,1,0,0}, //channel 2
    {1,1,0,0}, //channel 3
    {0,0,1,0}, //channel 4
    {1,0,1,0}, //channel 5
    {0,1,1,0}, //channel 6
    {1,1,1,0}, //channel 7
    {0,0,0,1}, //channel 8
    {1,0,0,1}, //channel 9
    {0,1,0,1}, //channel 10
    {1,1,0,1}, //channel 11
    {0,0,1,1}, //channel 12
    {1,0,1,1}, //channel 13
    {0,1,1,1}, //channel 14
    {1,1,1,1}  //channel 15
  };

  /* Loop through the four s-pins */
  for(int i = 0; i < 4; i++) {
    digitalWrite(controlPin[i], muxChannel[channel][i]);
  }

  /* Read the value at the signal pin*/
  int val = analogRead(sensorPin);

  /* Return the value */
  return val;
}
