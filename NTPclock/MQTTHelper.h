//----------------------------------------------------------------------------------------------------------------
// MQTTHelper.h
// 
// Manages controlling all MQTT communication & subscriptions.
// For ease, we define a global object that can be used for all MQTT-related functions
// 
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------
//            CONFIGURE YOUR MQTT SERVER HERE              //
//---------------------------------------------------------//
#define MQTT_SERVER "10.0.0.44"                            //
#define MQTT_USER ""                                       //
#define MQTT_PASSWORD ""                                   //
#define MQTT_RECONNECT_TIME 10000                          //
//---------------------------------------------------------//

#ifndef __MQTTHelper_H__
#define __MQTTHelper_H__

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT Messaging Library

class MQTTHelper
{
  WiFiClient espClient;
  PubSubClient mqttClient;
  unsigned long lastMQTTReconnect;
  
public:
  void mqttLoop();
  void connect();
  void reconnect();
  boolean publishMQTT(const char* channel, const char* data);
};

extern MQTTHelper MQTT_Helper;

#endif //__MQTTHelper_H__
