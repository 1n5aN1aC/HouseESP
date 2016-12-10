//
//
//

#ifndef __MQTTHelper_H__
#define __MQTTHelper_H__

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT Messaging Library

class MQTTHelper
{
  WiFiClient espClient;
  PubSubClient mqttClient;
  
public:
  void mqttLoop();
  void connect();
  void reconnect();
};

extern MQTTHelper MQTT_Helper;

#endif //__MQTTHelper_H__
