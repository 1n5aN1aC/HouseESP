//----------------------------------------------------------------------------------------------------------------
// Attic_Controller.ino
// 
// This sketch is designed to run on a WeMos D1 mini.  Other controllers will require extensive tweaking.
//
// Handles communicating with the roof sensors, as well as reading attic temperature / humidity
// Processes that information, and passes it on via MQTT
//
// Author - Joshua Villwock
// Created - 2016-11-13
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <DHT.h>
#include "MQTTHelper.h"
#include <ESP8266WiFi.h>
#include "Options.cpp"
extern "C" {
  #include "user_interface.h"
}

extern const char WIFI_SSID[];
extern const char WIFI_PASS[];

#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2320, AM2321
const int DHTPin = 2;
bool fahrenheit = true;
DHT dht(DHTPin, DHTTYPE);

#define MAXCHARS  32        // Max chars before a line break
#define ENDMARKER '\n'      // Character that defines the end of a line
char serialChars[MAXCHARS]; // Array to store received data
static byte next_loc = 0;   // Array location for next char

unsigned long lastTempHumidSend = millis();
#define TEMP_HUMID_UPDATE_FREQUENCY 60000

// Initial set up routines
void setup() {
  Serial.begin(9600);
  dht.begin();
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname("ESP_Attic");
  connectWifi();
  MQTT_Helper.setup();
}

// Main program loop
void loop() {
  MQTT_Helper.mqttLoop();
  checkTempHumid();
  yield();
  checkSerial();
  delay(100); //saves considerable power & heat
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(5000);
}

// Send temp / humidity update if need be
void checkTempHumid() {
  if (millis() > lastTempHumidSend + TEMP_HUMID_UPDATE_FREQUENCY) {

    char temp[8]; // Buffer big enough for 7-character float
    dtostrf(getTemperature(), -6, 2, temp); // Leave room for too large numbers!

    char humid[8];
    dtostrf(getHumidity(), -6, 2, humid);

    MQTT_Helper.publishMQTT("home/attic/controller/temp",  temp,  false);
    MQTT_Helper.publishMQTT("home/attic/controller/humid", humid, false);
    lastTempHumidSend = millis();
  }
}

// Check for (and handle) any serial data sent to us from the roof sensor
// Adapted from:  http://forum.arduino.cc/index.php?topic=288234.0
void checkSerial() {
  boolean newData = false;    // If we have a finished data packet

  //Get all the serial bytes we can until there is no more, or there is a newline.
  while (Serial.available() > 0 && newData == false) {
    char rc = Serial.read();

    //If a regular character, append to the list
    if (rc != ENDMARKER) {
      serialChars[next_loc] = rc;
      next_loc++;
      //If it's more than the max length, just overight the last char
      if (next_loc >= MAXCHARS) {
        next_loc = MAXCHARS - 1;
      }
    }
    //If a finishing character, then mark as complete
    else {
      serialChars[next_loc] = '\0'; // terminate the string
      next_loc = 0;
      newData = true;
    }
  }

  //Once a full line has been found, send to processing.
  if (newData == true) {
    handleSerial();
    newData = false;
  }
}

// Process a complete serial line, and publish results to MQTT
void handleSerial() {
  char cleanChars[MAXCHARS];  //serialChars, with first 2 chars 'removed'
  strncpy(cleanChars, serialChars, MAXCHARS);
  for(int i=2; i < MAXCHARS; i++) {
    cleanChars[i-2] = cleanChars[i];
  }

  // Check the first character, and process the packet based on that.
  //Wind Update
  if (serialChars[0] == 'W') {
    MQTT_Helper.publishMQTT("home/roof/weather/wind", cleanChars, false);
  }
  //Temperature Update
  if (serialChars[0] == 'T') {
    MQTT_Helper.publishMQTT("home/roof/weather/temp", cleanChars, false);
  }
  //Humidity Update
  if (serialChars[0] == 'H') {
    MQTT_Helper.publishMQTT("home/roof/weather/humid", cleanChars, false);
  }
  //'Battery' Update
  if (serialChars[0] == 'B') {
    MQTT_Helper.publishMQTT("home/roof/weather/battery", cleanChars, true);
  }
  //Rain Flip
  if (serialChars[0] == 'R') {
    MQTT_Helper.publishMQTT("home/roof/weather/rain", serialChars, false);
  }
}

float getTemperature() {
  return dht.readTemperature(fahrenheit);
}

float getHumidity() {
  return dht.readHumidity();
}
