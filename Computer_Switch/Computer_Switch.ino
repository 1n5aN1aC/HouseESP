//----------------------------------------------------------------------------------------------------------------
// Computer_Switch.ino
// 
// This sketch is designed to run on a WeMos D1 mini.  Other controllers may require extensive tweaking.
//
// This runs a small webserver hosting a tiny API to remotely power on or off a computer
// Also supports a rely to electrically isolate the server.
//
// Author - Joshua Villwock
// Created - 2019-05-28
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
extern "C" {
  #include "user_interface.h"
}

// Create aREST instance
aREST rest = aREST();

// WiFi parameters
const char* ssid = "joshua";
const char* password = "";

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int relayOn;

// Declare functions to be exposed to the API
int powerOn(String command);
int flashLed(String command);
int hardReset(String command);

void setup(void)
{
  // Start Serial
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP_EIDOLON_IPMI");
  wifi_station_set_hostname("ESP_EIDOLON_IPMI");
  delay(100);

  // Init variables and expose them to REST API
  relayOn = 0;
  rest.variable("relayOn",&relayOn);

  // Function to be exposed
  rest.function("power",powerOn);
  rest.function("led",flashLed);
  rest.function("reset",hardReset);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("ESP_EIDOLON_IPMI");

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());

  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(D4, INPUT);
}

void loop() {
  // Handle REST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  while(!client.available()){
    delay(1);
  }
  rest.handle(client);
}

// Custom function accessible by the API
int flashLed(String command) {
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
  return 1;
}

// Custom function accessible by the API
int powerOn(String command) {
  pinMode(D4, OUTPUT);
  digitalWrite(D4,LOW);
  delay(1500);
  digitalWrite(D4,HIGH);
  pinMode(D4, INPUT);
  return 1;
}

// Custom function accessible by the API
int hardReset(String command) {
  pinMode(D4, OUTPUT);
  digitalWrite(D4,LOW);
  delay(6000);
  digitalWrite(D4,HIGH);
  pinMode(D4, INPUT);
  return 1;
}
