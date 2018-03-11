//----------------------------------------------------------------------------------------------------------------
// Micro_Temp.ino
// 
// This sketch is designed to run on a WeMos D1 mini.  Other controllers will require extensive tweaking.
//
// Handles deep sleep, waking, reading temp & humidity via DHT22, and sending results via MQTT
//
// Author - Joshua Villwock
// Created - 2018-03-11
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

unsigned long lastTempHumidSend = millis();
#define TEMP_HUMID_UPDATE_FREQUENCY 60000

// Initial set up routines
void setup() {
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

    MQTT_Helper.publishMQTT("home/living/micro/temp",  temp,  false);
    MQTT_Helper.publishMQTT("home/living/micro/humid", humid, false);
    lastTempHumidSend = millis();
  }
}

float getTemperature() {
  return dht.readTemperature(fahrenheit);
}

float getHumidity() {
  return dht.readHumidity();
}
