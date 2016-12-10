#include "MQTTHelper.h"

//---------------------------------------------------------//
#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_USER ""
#define MQTT_PASSWORD ""
//---------------------------------------------------------//

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
