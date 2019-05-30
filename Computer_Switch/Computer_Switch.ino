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

//------------------------------------------------------------
//------------------------------------------------------------
#define RELAY_PIN D1
#define POWER_BUTTON D4
#define POWER_LED D2

// The port to listen for incoming TCP connections
#define LISTEN_PORT 80

// How often to check for power off state
unsigned long lastPowerCheck = millis();
#define POWER_CHECK_FREQUENCY 10000

// WiFi parameters
const char* ssid = "joshua";
const char* password = "";
//------------------------------------------------------------
//------------------------------------------------------------

// Create aREST instance
aREST rest = aREST();
// Create an instance of the server
WiFiServer server(LISTEN_PORT);

// Variables to be exposed to the API
int           relayOn =               0;
int           computerOn =            0;
int           relayOffWhenPowerDown = 0;

// Functions to be exposed to the API
int flashLed(String command);          // Flash the onboard LED as a test
int powerOn(String command);           // Press the power button
int hardPowerOff(String command);      // Hold the power button
int powerOnGracefully(String command); // Turn on relay, then press power.

void setup(void)
{
  Serial.begin(115200);
  initWifi();

  // Expose variables to REST API
  rest.variable("relayOn",&relayOn);
  rest.variable("computerOn",&computerOn);
  rest.variable("relayOffWhenPowerDown",&relayOffWhenPowerDown);

  // Function to be exposed
  rest.function("flash",flashLed);
  rest.function("on",powerOn);
  rest.function("off",hardPowerOff);
  rest.function("powerUp",powerOnGracefully);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("ESP_EIDOLON_IPMI");

  pinMode(LED_BUILTIN,  OUTPUT); //Built in LED for testing
  pinMode(POWER_BUTTON, INPUT);  //Connected to 'hot' wire of on button (switches between in and out)
  pinMode(POWER_LED,    INPUT);  //Connected to 'hot' wire of power LED (resistor?)
}

void loop() {
  // Handle REST calls
  WiFiClient client = server.available();
  if (client && client.available()) {
    rest.handle(client);
  }
  // Check for system power off state
  if (millis() > lastPowerCheck + POWER_CHECK_FREQUENCY) {
    if (relayOn && relayOffWhenPowerDown && !computerOn) {
      //It looks like we're shutdown, but lets make sure.
      delay(100);
      if (digitalRead(POWER_LED) == 0) {
        //OK then, power off Relay.
        delay(2000);
		digitalWrite(RELAY_PIN, LOW);
		relayOn = 0;
		relayOffWhenPowerDown = 0;
      }
    }
    lastPowerCheck = millis();
  }
  // Check status to update variables
  computerOn = digitalRead(POWER_LED);
  delay(1);
}

void initWifi() {
  //Wifi settings
  WiFi.mode(WIFI_STA);
  WiFi.hostname("ESP_EIDOLON_IPMI");
  wifi_station_set_hostname("ESP_EIDOLON_IPMI");
  delay(100);
  
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

// Powers on the computer by pulling the power button low.
int powerOn(String command) {
  pinMode(POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, LOW);
  delay(1000);
  digitalWrite(POWER_BUTTON, HIGH);
  pinMode(POWER_BUTTON, INPUT);
  return 1;
}

// Hard power off computer by holding power button 6 seconds.
int hardPowerOff(String command) {
  pinMode(POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, LOW);
  delay(6000);
  digitalWrite(POWER_BUTTON,HIGH);
  pinMode(POWER_BUTTON, INPUT);
  return 1;
}

// Turn on relay, then press power.
// Parameter 0 = just do that.
// Parameter 1 = also wait until computer shuts down, then automatically flip relay off.
int powerOnGracefully(String command) {
  //Relay on
  digitalWrite(RELAY_PIN, HIGH);
  relayOn = 1;
  delay(5000);
  //Then power computer on as normal
  powerOn("");
  computerOn = 1;
  //Setup auto off if requested
  int autoOff = command.toInt();
  if (autoOff == 1) {
    relayOffWhenPowerDown = 1;
  } else {
    relayOffWhenPowerDown = 0;
  }
  return 1;
}
