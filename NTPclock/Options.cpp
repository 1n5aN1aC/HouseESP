//----------------------------------------------------------------------------------------------------------------
// Options.cpp
//
// This file exists to contain potentially-sesitive configuration options,
// such as network passwords and the like.
// It does, however, contain other options as well.
//
// Author - Joshua Villwock
// Created - 2016-12-30
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <ESP8266WiFi.h> //Needed for IPAddress type

// WiFi settings
const char WIFI_SSID[] = "joshua";  // your network SSID (name)
const char WIFI_PASS[] = "";        // your network password

//---------------------------------------------------------//
//            CONFIGURE YOUR MQTT SERVER HERE              //
//---------------------------------------------------------//
const IPAddress MQTT_SERVER(10, 0, 0, 21);                 //
const int       MQTT_RECONNECT_TIME = 10000;               //
//---------------------------------------------------------//

// OTA settings
const char OTA_PASS[] = "21232f297a57a5a743894a0e4a801fc3";
