//----------------------------------------------------------------------------------------------------------------
// Clock.ino
// 
// Controls a LED clock using an ESP8266, several 7-segment displays, and a bargraph display
// Uses NTP for internet time updating, and supports a Real-Time clock for outages.
// Some extra features include a temperature & humidity sensor, brightness control, time zone control.
// 
// Author - Joshua Villwock
// Created - 2016-11-13
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
#define NETWORK_SSID "joshua"  // your network SSID (name) //
#define NETWORK_PASS ""        // your network password    //
//---------------------------------------------------------//
//            CONFIGURE YOUR MQTT SERVER HERE              //
//---------------------------------------------------------//
#define MQTT_RECONNECT_TIME 10000                          //
//---------------------------------------------------------//
#define DISPLAY_UPDATE_FREQUENCY 1000                      //
#define TEMPERATURE_UPDATE_FREQUENCY 60000                 //
//---------------------------------------------------------//

#include <ESP8266WiFi.h>  // We need to use the wifi for NTP
#include <PubSubClient.h> // MQTT Messaging Library
#include "LEDHelper.h"
#include "TimeManager.h"

unsigned long displayLastUpdated = millis();
unsigned long lastTempSend = millis();
unsigned long lastMQTTReconnect = millis();

WiFiClient espClient;
PubSubClient MQTTClient(espClient);
IPAddress MQTT_SERVER(10, 0, 0, 44);

// Initial set up routines
void setup() {
  Serial.begin(115200);
  Serial.println();

  LED_Helper.LED_Setup();     //Turn on the LEDS, etc.

  Time_Manager.RTCSetup();    //Restore RTC time immediently to current time
  connectWifi();              //Then connect to wifi
  Time_Manager.beginNTP();    //Start up NTP Client & time keeping
  
  MQTTClient.setServer(MQTT_SERVER, 1883);
  MQTTClient.setCallback(MQTTCallback);
}

// Main program loop
void loop() {
  MQTTLoop();
  checkUpdate();
  checkTemp();
  yield();
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(NETWORK_SSID);
  WiFi.begin(NETWORK_SSID, NETWORK_PASS);
  delay(5000);
}

//Update the clock if need be
void checkUpdate() {
  if (millis() > (displayLastUpdated + DISPLAY_UPDATE_FREQUENCY)) {
    LED_Helper.updateDigits(); //Update the time display
    LED_Helper.updateMisc();   //Update the rest of the display
    
    Serial.println(NTP.getTimeDateString() );
    
    displayLastUpdated = millis();
  }
}

//Send temp update if need be
void checkTemp() {
  if (millis() > lastTempSend + TEMPERATURE_UPDATE_FREQUENCY) {
    float celsius = Time_Manager.getTemperature();
    float fahrenheit = celsius * 9.0 / 5.0 + 32.0;
  
    char result[8]; // Buffer big enough for 7-character float
    dtostrf(fahrenheit, 6, 2, result); // Leave room for too large numbers!
    MQTTClient.publish("home/jroom/clock/temp", result);
    lastTempSend = millis();
  }
}

//--------------------------------------------//
//               MQTT Functions               //
//--------------------------------------------//

// Needs to be called by the main program loop frequently.
// Makes sure we are still connected, and check for any new subscription replies.
void MQTTLoop() {
  MQTTClient.loop();
  
  if (!MQTTClient.connected()) {
    //Not connected, so 
    if (millis() > lastMQTTReconnect + MQTT_RECONNECT_TIME) {
      Serial.print("Attempting MQTT connection...");
      // Create a random client ID
      String clientId = "ESP8266Client-";
      clientId += String(random(0xffff), HEX);
      // Attempt to connect
      if (MQTTClient.connect(clientId.c_str())) {
        Serial.println("connected");
        // ... and resubscribe
        MQTTClient.subscribe("home/jroom/clock/brightness", 1);
      } else {
        Serial.print("failed, rc=");
        Serial.print(MQTTClient.state());
        Serial.println(" try again in MQTT_RECONNECT_TIME seconds");
      }
      lastMQTTReconnect = millis();
    }
  }
}

// This is the method that actually handles the MQTT update
void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
