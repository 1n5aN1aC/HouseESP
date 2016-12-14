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
#include "LEDHelper.h"

MQTTHelper MQTT_Helper = MQTTHelper();

// Needs to be called by the main program loop frequently.
// Makes sure we are still connected, and check for any new subscription replies.
void MQTTHelper::mqttLoop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Technically, just configures the connection.
// reconnect() technically does the connecting
void MQTTHelper::connect() {
  mqttClient = PubSubClient(espClient);
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(callback);
  mqttClient.subscribe("home/jroom/clock/brightness", 1);
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

// 
boolean MQTTHelper::publishMQTT(const char* channel, const char* data) {
  mqttClient.publish(channel, data);
}

// Register to recieve updates on a topic
boolean MQTTHelper::subscribeTopic(char* topic, int qos) {
  mqttClient.subscribe(topic, qos);
}

// This is a horrible hack we have to do because of library limitations
void MQTTCallbackShim(char* topic, byte* payload, unsigned int length) {
  Serial.println("callback called.  :)");
  MQTT_Helper.MQTTCallback(topic, payload, length);
}

// This is the method that actually handles the MQTT update
void MQTTHelper::MQTTCallback(char* topic, byte* payload, unsigned int length) {
  if (strcmp(topic, "home/jroom/clock/brightness") == 0) {
    char array2[length];
    strncpy(array2, reinterpret_cast<const char*>(payload), length);
    Serial.println(array2);
  }
}
