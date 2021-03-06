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

#include <ESP8266WiFi.h>  // We need to use the wifi for NTP
#include "LEDHelper.h"
#include "TimeManager.h"
#include "MQTTHelper.h"
#include "Options.cpp"
extern "C" {
  #include "user_interface.h"
}

extern const char WIFI_SSID[];
extern const char WIFI_PASS[];

unsigned long displayLastUpdated = millis();
unsigned long lastTempSend = millis();

#define DISPLAY_UPDATE_FREQUENCY 1000
#define TEMPERATURE_UPDATE_FREQUENCY 60000

// Initial set up routines
void setup() {
  Serial.begin(9600);
  Serial.println();

  LED_Helper.LED_Setup();     //Turn on the LEDS, etc.

  Time_Manager.RTCSetup();    //Restore RTC time immediately to current time
  wifi_station_set_hostname("ESP_Clock");
  connectWifi();              //Then connect to wifi
  Time_Manager.beginNTP();    //Start up NTP Client & time keeping
  MQTT_Helper.connect();
}

// Main program loop
void loop() {
  MQTT_Helper.mqttLoop();
  yield();
  checkTimeUpdate();
  checkTempUpdate();
  delay(50); //saves considerable power & heat
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  delay(5000);
}

//Update the clock if need be
void checkTimeUpdate() {
  if (millis() > (displayLastUpdated + DISPLAY_UPDATE_FREQUENCY)) {
    LED_Helper.updateDigits(); //Update the time display
    LED_Helper.updateMisc();   //Update the rest of the display

    Serial.println(NTP.getTimeDateString() );

    displayLastUpdated = millis();
  }
}

//Send temp update if need be
void checkTempUpdate() {
  if (millis() > lastTempSend + TEMPERATURE_UPDATE_FREQUENCY) {
    float temp = Time_Manager.getTemperature();

    char result[8]; // Buffer big enough for 7-character float
    dtostrf(temp, 6, 2, result); // Leave room for too large numbers!
    MQTT_Helper.publishMQTT("home/jroom/clock/temp", result, false);
    lastTempSend = millis();
  }
}
