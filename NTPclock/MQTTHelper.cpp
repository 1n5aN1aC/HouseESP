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

// Needs to be called by the main program loop frequently.
// Makes sure we are still connected, and check for any new subscription replies.
void MQTTHelper::mqttLoop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

// Technically, just configures the connection.
// reconnect() technically does the connecting
void MQTTHelper::connect() {
  mqttClient = PubSubClient(espClient);
  mqttClient.setServer(MQTT_SERVER, 1883);
}

// Tries to reconnect MQTT, but only if it hasn't tried in the last MQTT_RECONNECT_TIME seconds...
void MQTTHelper::reconnect() {
  if (millis() > lastMQTTReconnect + MQTT_RECONNECT_TIME) {
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
      Serial.println(" try again in MQTT_RECONNECT_TIME seconds");
    }
    lastMQTTReconnect = millis();
  }
}

boolean MQTTHelper::publishMQTT(const char* channel, const char* data) {
  mqttClient.publish(channel, data);
}
