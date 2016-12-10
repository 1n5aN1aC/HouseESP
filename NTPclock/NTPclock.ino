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

#include <Wire.h>         // Incuded here so that Arduino library object file references work
#ifndef _ESPwifi_
  #include <ESP8266WiFi.h>  // We need to use the wifi for NTP
#endif

#include "LEDHelper.h"
#include "TimeManager.h"
#include "MQTTHelper.h"

//---------------------------------------------------------//
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
#define ssid "joshua"  // your network SSID (name)         //
#define pass ""        // your network password            //
//---------------------------------------------------------//

unsigned long timeLastUpdated = millis();

// Initial set up routines
void setup() {
  Serial.begin(115200);

  LED_Helper.LED_Setup();     //Turn on the LEDS, etc.

  Time_Manager.RTCSetup();    //Restore RTC time immediently to current time
  connectWifi();              //Then connect to wifi
  Time_Manager.beginNTP();    //Start up NTP Client & time keeping
  MQTT_Helper.connect();
}

// Main program loop
void loop() {
  MQTT_Helper.mqttLoop();
  
  if (millis() > (timeLastUpdated + 1000)) {
    LED_Helper.updateDigits(); //Update the time display
    LED_Helper.updateMisc();   //Update the rest of the display
    
    Serial.println(NTP.getTimeDateString(now() ) );
    
    timeLastUpdated = millis();

    //int t = RTC.temperature();
    //float celsius = t / 4.0;
    //float fahrenheit = celsius * 9.0 / 5.0 + 32.0;

    //char result[8]; // Buffer big enough for 7-character float
    //dtostrf(celsius, 6, 2, result); // Leave room for too large numbers!
    //client.publish("temp/random", result);
  }
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  delay(5000);
}
