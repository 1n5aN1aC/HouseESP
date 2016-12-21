//----------------------------------------------------------------------------------------------------------------
// Attic_Controller.ino
// 
// Controls a LED clock using an ESP8266, several 7-segment displays, and a bargraph display
// Uses NTP for internet time updating, and supports a Real-Time clock for outages.
// Some extra features include a temperature & humidity sensor, brightness control, time zone control.
// 
// Author - Joshua Villwock
// Created - 2016-11-13
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <DHT.h>
#include "MQTTHelper.h"
#include <ESP8266WiFi.h>  // We need to use the wifi for NTP

//---------------------------------------------------------//
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
#define SSID "joshua"  // your network SSID (name)         //
#define PASS ""        // your network password            //
//---------------------------------------------------------//

#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2320, AM2321
const int DHTPin = 4;
DHT dht(DHTPin, DHTTYPE);
bool fahrenheit = true;

unsigned long lastTempHumidSend = millis();
#define TEMP_HUMID_UPDATE_FREQUENCY 60000

// Initial set up routines
void setup() {
  Serial.begin(115200);
  Serial.println();
  dht.begin();
  connectWifi();        //Connect to wifi
  MQTT_Helper.setup();
}

// Main program loop
void loop() {
  MQTT_Helper.mqttLoop();
  
  checkTempHumid();
  delay(1000); //saves power; we don't need to check often anyways...
  yield();
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASS);
  delay(5000);
  Serial.println(getTemperature() );
}

//Send temp / humidity update if need be
void checkTempHumid() {
  if (millis() > lastTempHumidSend + TEMP_HUMID_UPDATE_FREQUENCY) {
    
    char temp[8]; // Buffer big enough for 7-character float
    dtostrf(getTemperature(), 6, 2, temp); // Leave room for too large numbers!

    char humid[8];
    dtostrf(getHumidity(), 6, 2, humid);
    
    MQTT_Helper.publishMQTT("home/attic/controller/temp",  temp,  false);
    MQTT_Helper.publishMQTT("home/attic/controller/humid", humid, false);
    lastTempHumidSend = millis();
  }
}

float getTemperature() {
  float tempC = dht.readTemperature();
  float tempF = tempC * 9.0 / 5.0 + 32.0;

  if (fahrenheit)
    return tempF;
  else
    return tempC;
}

float getHumidity() {
  return dht.readHumidity();
}

