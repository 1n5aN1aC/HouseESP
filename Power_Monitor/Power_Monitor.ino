//----------------------------------------------------------------------------------------------------------------
// Power_Monitor.ino
// 
// ESP8266 node that monitors power usage via non-invasive current sensors
// Reports back power usage via MQTT.  Also does temp & humidity.
// This is designed to run on an ESP board with built-in relays, though that is not used yet.
// 
// Author - Joshua Villwock
// Created - 2016-12-22
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <DHT.h>
#include "MQTTHelper.h"
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
extern "C" {
  #include "user_interface.h"
}

//---------------------------------------------------------//
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
#define SSID "joshua"  // your network SSID (name)         //
#define PASS ""        // your network password            //
//---------------------------------------------------------//

#define DHTTYPE DHT22     // I use DHT 22  (AM2302), AM2320, AM2321
const int DHTPin = 4;     // pin the DHT is connected to
bool fahrenheit = true;   // use fahrenheit?  Future versions will allow changing via MQTT
DHT dht(DHTPin, DHTTYPE); // Set up the sensor...

unsigned long lastTempHumidSend = 0;
unsigned long lastPowerRead     = 0;
unsigned long lastPowerSend     = 0;

#define TEMP_HUMID_UPDATE_FREQUENCY 60000 //temp every minute
#define POWER_UPDATE_FREQUENCY      500   //Read power 2x per second
#define POWER_SEND_FREQUENCY        10000 //Send power every 10 seconds

// array for power readings
const int UPDATES_PER_SEND = ceil(POWER_SEND_FREQUENCY / POWER_UPDATE_FREQUENCY);
String powerReadings[UPDATES_PER_SEND];

// Initial set up routines
void setup() {
  Serial.begin(9600);
  dht.begin();
  wifi_station_set_hostname("ESP_Power");
  connectWifi();
  MQTT_Helper.setup();
  otaInit();
  ArduinoOTA.setPassword((const char *)"123");
}

// Main program loop
void loop() {
  MQTT_Helper.mqttLoop();
  yield();
  checkPower();
  checkTempHumid();
  delay(100); //saves considerable power
  ArduinoOTA.handle();
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  WiFi.begin(SSID, PASS);
  delay(5000);
}

// Read Power usage
void checkPower() {
  if (millis() > lastPowerRead + POWER_UPDATE_FREQUENCY) {
    //

    lastPowerRead = millis();
    //If it's been long enough, also send the power update via MQTT
    if (millis() > lastPowerSend + POWER_SEND_FREQUENCY) {
      sendPower();
    }
  }
}

// Send power
void sendPower() {
  //
  lastPowerSend = millis();
}

// Send temp / humidity update
void checkTempHumid() {
  if (millis() > lastTempHumidSend + TEMP_HUMID_UPDATE_FREQUENCY) {
    
    char temp[8]; // Buffer big enough for 7-character float
    dtostrf(getTemperature(), 6, 2, temp); // Leave room for too large numbers!

    char humid[8];
    dtostrf(getHumidity(), 6, 2, humid);
    
    MQTT_Helper.publishMQTT("home/garage/power/temp",  temp,  false);
    MQTT_Helper.publishMQTT("home/garage/power/humid", humid, false);
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

void otaInit() {
  ArduinoOTA.onStart([]() {
  Serial.println("Starting OTA");
  });
  ArduinoOTA.onEnd([]() {
  Serial.println("\nEnd of OTA");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
  Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
  Serial.printf("Error[%u]: ", error);
  if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
  else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
  else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
  else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
  else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
