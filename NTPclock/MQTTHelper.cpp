//----------------------------------------------------------------------------------------------------------------
// MQTTHelper.cpp
// 
// Manages controlling all MQTT communication & subscriptions.
// For ease, we define a global object that can be used for all MQTT-related functions
// 
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include "MQTTHelper.h"

MQTTHelper MQTT_Helper = MQTTHelper();

void MQTTHelper::mqttLoop() {
 if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

void MQTTHelper::connect() {
  mqttClient = PubSubClient(espClient);
  mqttClient.setServer(MQTT_SERVER, 1883);
}

// Loop until we're reconnected
void MQTTHelper::reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
