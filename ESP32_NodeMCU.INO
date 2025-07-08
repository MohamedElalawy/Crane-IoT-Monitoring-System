#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <EEPROM.h>

// Wi-Fi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Google Apps Script Web App URL
const char* serverName = "YOUR_WEBAPP_URL";

// Crane number
const char* craneID = "Crane-001";

// Digital input pins
const int pinCraneRunning = D1;
const int pinCraneWorking = D2;
const int pinFailureFlag = D3;
const int pinMaintenanceFlag = D4;
const int pinManualSave = D5;

// Simulated analog sensors
const int pinTempSensor = A0;
const int pinPressureSensor = A0; // Replace with real pins

// Timers
unsigned long lastUpdate = 0;
unsigned long lastEEPROMWrite = 0;
const unsigned long updateInterval = 5000;     // 5 sec
const unsigned long EEPROMInterval = 3600000;  // 1 hr

// Flags
bool saveNowRemote = false;
bool saveNowLocal = false;

// Time accumulators
unsigned long onTime = 0;
unsigned long workingTime = 0;
unsigned long failureTime = 0;
unsigned long maintenanceTime = 0;

unsigned long lastLoopTime = 0;

// Maintenance thresholds
const unsigned long oilFilterInterval = 250 * 3600;   // 250 hours in seconds
const unsigned long airFilterInterval = 500 * 3600;   // 500 hours
const unsigned long fuelFilterInterval = 1000 * 3600; // 1000 hours

void setup() {
  Serial.begin(115200);
  pinMode(pinCraneRunning, INPUT);
  pinMode(pinCraneWorking, INPUT);
  pinMode(pinFailureFlag, INPUT);
  pinMode(pinMaintenanceFlag, INPUT);
  pinMode(pinManualSave, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected.");

  EEPROM.begin(64);
  loadEEPROM();

  lastLoopTime = millis();
}

void loop() {
  unsigned long now = millis();
  unsigned long dt = (now - lastLoopTime) / 1000; // seconds
  lastLoopTime = now;

  // Read inputs
  bool craneOn = digitalRead(pinCraneRunning);
  bool craneWorking = digitalRead(pinCraneWorking);
  bool failureFlag = digitalRead(pinFailureFlag);
  bool maintenanceFlag = digitalRead(pinMaintenanceFlag);

  if (craneOn) onTime += dt;
  if (craneWorking) workingTime += dt;
  if (failureFlag) failureTime += dt;
  if (maintenanceFlag) maintenanceTime += dt;

  // Local manual save input
  if (digitalRead(pinManualSave) == HIGH) {
    saveNowLocal = true;
  }

  if (now - lastUpdate > updateInterval) {
    sendData(craneOn, craneWorking, failureFlag, maintenanceFlag);
    lastUpdate = now;
  }

  if (now - lastEEPROMWrite > EEPROMInterval || saveNowLocal || saveNowRemote) {
    saveToEEPROM();
    saveNowLocal = false;
    saveNowRemote = false;
    lastEEPROMWrite = now;
  }
}

void sendData(bool craneOn, bool craneWorking, bool failureFlag, bool maintenanceFlag) {
  if (WiFi.status() != WL_CONNECTED) return;

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");

  float temp = analogRead(pinTempSensor); // Replace with real sensor code
  float pressure = analogRead(pinPressureSensor); // Replace with real sensor code

  String json = "{";
  json += "\"craneID\":\"" + String(craneID) + "\",";
  json += "\"onTime\":\"" + String(onTime) + "\",";
  json += "\"workingTime\":\"" + String(workingTime) + "\",";
  json += "\"failureTime\":\"" + String(failureTime) + "\",";
  json += "\"maintenanceTime\":\"" + String(maintenanceTime) + "\",";
  json += "\"temperature\":\"" + String(temp) + "\",";
  json += "\"pressure\":\"" + String(pressure) + "\",";
  json += "\"oilFilterInterval\":\"" + String(oilFilterInterval) + "\",";
  json += "\"airFilterInterval\":\"" + String(airFilterInterval) + "\",";
  json += "\"fuelFilterInterval\":\"" + String(fuelFilterInterval) + "\"";
  json += "}";

  int responseCode = http.POST(json);
  if (responseCode > 0) {
    String response = http.getString();
    Serial.println(response);

    if (response.indexOf("SAVE_NOW") >= 0) {
      saveNowRemote = true;
    }
  } else {
    Serial.println("Error sending data");
  }
  http.end();
}

void saveToEEPROM() {
  EEPROM.put(0, onTime);
  EEPROM.put(4, workingTime);
  EEPROM.put(8, failureTime);
  EEPROM.put(12, maintenanceTime);
  EEPROM.commit();
  Serial.println("EEPROM updated");
}

void loadEEPROM() {
  EEPROM.get(0, onTime);
  EEPROM.get(4, workingTime);
  EEPROM.get(8, failureTime);
  EEPROM.get(12, maintenanceTime);
  Serial.println("EEPROM loaded");
}
