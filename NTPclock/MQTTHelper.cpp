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

//---------------------------------------------------------//
//            CONFIGURE YOUR MQTT SERVER HERE              //
//---------------------------------------------------------//
const IPAddress MQTT_SERVER(10, 0, 0, 44);                 //
const int       MQTT_RECONNECT_TIME = 10000;               //
//---------------------------------------------------------//

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
  mqttClient.setCallback(MQTTCallbackShim);
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
      // ... and resubscribe
      mqttClient.subscribe("home/jroom/clock/brightness", 1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in MQTT_RECONNECT_TIME seconds");
    }
    lastMQTTReconnect = millis();
  }
}

// Just a shim to allow you to call it via the MQTTHelper object
boolean MQTTHelper::publishMQTT(const char* channel, const char* data, bool retained) {
  mqttClient.publish(channel, data, retained);
}

// This is a horrible hack we have to do because of library limitations
void MQTTCallbackShim(char* topic, byte* payload, unsigned int length) {
  MQTT_Helper.MQTTCallback(topic, payload, length);
}

// This is the method that actually handles the MQTT update
// Here is what i have been using to handle subscriptions. I took it as a snippet from elsewhere but i cannot credit author as i dont have reference!
void MQTTHelper::MQTTCallback(char* topic, byte* payload, unsigned int length) {
  char message_buff[128];   // initialize storage buffer (I haven't tested to this capacity.)
  // create character buffer with ending null terminator (string)
  int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';

  String msgString   = String(message_buff);
  String topicString = String(topic);     //the topic is null-terminated, so we can easily convert it.
  int    msgInt      = msgString.toInt(); //the int version of the message, if conversion is possible.

  Serial.println("Topic:" + topicString);
  Serial.println("Payload: " + msgString);

  if (topicString.equalsIgnoreCase("home/jroom/clock/brightness")) {
    LED_Helper.set_brightness(msgInt);
  }
}
