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
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Options.cpp"
#include "MQTTHelper.h"
extern "C" {
  #include "user_interface.h"
}

void connectWifi();
void checkTempHumid();

#define UPDATE_FREQUENCY 20 // Time to sleep (in seconds)
#define DHTTYPE DHT11       // DHT 11
const int DHTPin = 2;       // Should be D4 on the Wemos D1 Mini
bool fahrenheit = true;     // Yes, report fahrenheit
DHT dht(DHTPin, DHTTYPE);

//Wifi settings
extern const char WIFI_SSID[];
extern const char WIFI_PASS[];
WiFiClientSecure wifiClient;

//Main progrom upon return from Deep Sleep
void setup() {
  Serial.begin(9600);
  while(!Serial) { }         // Wait for serial to initialize.
  Serial.println("I AM WOKE.");
  
  dht.begin();
  connectWifi();
  MQTT_Helper.setup();
  yield();
  checkTempHumid();
}

void loop() {
  delay(2000);  //Sometimes the MQTT doesn't seem to go through before sleep begins....
  Serial.println("Going into deep sleep for 20 seconds");
  ESP.deepSleep(UPDATE_FREQUENCY * 1000000);
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  wifi_station_set_hostname("ESP_Attic");
  unsigned long wifiConnectStart = millis();
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(10000);
    }

    delay(500);
    Serial.print("Connecting wifi");
    Serial.print(".");
    // Only try for 5 seconds.
    if (millis() - wifiConnectStart > 15000) {
      Serial.println("Failed to connect to WiFi");
      return;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


float getTemperature() {
  return dht.readTemperature(fahrenheit);
}

float getHumidity() {
  return dht.readHumidity();
}

// Send temp / humidity update if need be
void checkTempHumid() {
  char temp[8]; // Buffer big enough for 7-character float
  dtostrf(getTemperature(), -6, 2, temp); // Leave room for too large numbers!

  char humid[8];
  dtostrf(getHumidity(), -6, 2, humid);

  MQTT_Helper.publishMQTT("home/living/micro/temp",  temp,  false);
  MQTT_Helper.publishMQTT("home/living/micro/humid", humid, false);
}
