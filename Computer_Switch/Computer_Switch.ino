//----------------------------------------------------------------------------------------------------------------
// Computer_Switch.ino
// 
// This sketch is designed to run on a WeMos D1 mini.  Other controllers may require extensive tweaking.
//
// This runs a small webserver hosting a tiny API to remotely power on or off a computer
// Also supports a rely to electrically isolate the server.
// Includes functionality to report temperature back to influxDB
//
// Author - Joshua Villwock
// Created - 2019-05-28
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

// Import required libraries
#include <DHT.h>               //Temp Sensor
#include <ESP8266WiFi.h>       //WiFi
#include <ESP8266mDNS.h>       //OTA
#include <WiFiUdp.h>           //OTA
#include <ArduinoOTA.h>        //OTA
#include <ESP8266HTTPClient.h> //Server
#include <ESP8266WebServer.h>  //Server
#include <lwip/netif.h>        //GratuitousARP
#include <lwip/etharp.h>       //GratuitousARP
#include "Options.cpp"         //User Options
extern "C" {
  #include "user_interface.h"
}

//Setup DHT
DHT dht(DHT_PIN, DHTTYPE);

// The port to listen for incoming TCP connections
ESP8266WebServer server(80);

//Init Starting Times
unsigned long lastTempCheck = millis();
unsigned long lastPowerCheck = millis();
unsigned long lastGratuitousARP = millis();

// Variables to be exposed to the API
int relayOn =               0;
int computerOn =            0;
int relayOffWhenPowerDown = 0;
char temp[8] =             "";

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  initWifi();
  dht.begin();

  server.on("/", getStatus);
  server.on("/on", powerOn);
  server.on("/off", hardPowerOff);
  server.on("/relayon", powerOnGracefully);

  pinMode(LED_BUILTIN,  OUTPUT); //Built in LED for testing
  pinMode(POWER_BUTTON, INPUT);  //Connected to 'hot' wire of on button (switches between in and out)
  pinMode(POWER_LED,    INPUT);  //Connected to 'hot' wire of power LED (resistor?)

  server.begin(); //Start the server
  Serial.println("Server listening");
}

void loop() {
  //Handling of incoming requests
  server.handleClient();

  // Check for system power off state
  if (millis() > lastPowerCheck + POWER_CHECK_FREQUENCY) {
    if (relayOn && relayOffWhenPowerDown && !computerOn) {
      //It looks like we're shutdown, but lets make sure.
      delay(250);
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
  // Check the temperature and update the server
  if (millis() > lastTempCheck + TEMP_CHECK_FREQUENCY) {
    checkTempHumid();
    lastTempCheck = millis();
  }
  
  if (millis() > lastGratuitousARP + GRATUITOUS_ARP_FREQUENCY) {
    SendGratuitousARP();
    lastGratuitousARP = millis();
  }
  // Check status to update variables
  computerOn = digitalRead(POWER_LED);
  delay(1);
}

// Set up WiFi
void initWifi() {
  //Wifi settings
  WiFi.mode(WIFI_STA);
  WiFi.hostname(DEVICE_NAME);
  wifi_station_set_hostname(DEVICE_NAME);
  delay(100);
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //Start OTA
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.setPassword(OTA_PASS);

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void SendGratuitousARP() {
  netif *n = netif_list;
  while (n) {
    etharp_gratuitous(n);
    n = n->next;
  }
}

void getStatus() {
  String preString = "{\"relayOn\":\"" + String(relayOn) + "\",\"computerOn\":\"" + String(computerOn) + "\",\"relayOffWhenPowerDown\":\"" + String(relayOffWhenPowerDown) + "\",\"temp\":\"" + String(temp) + "\"}";
  const char * result = preString.c_str();
  server.send(200, "application/json", result);
}

// Powers on the computer by pulling the power button low.
void powerOn() {
  pinMode(POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, LOW);
  delay(1000);
  digitalWrite(POWER_BUTTON, HIGH);
  pinMode(POWER_BUTTON, INPUT);
  server.send(200, "application/json", "{\"status\":\"complete\"}");
}

// Hard power off computer by holding power button 6 seconds.
void hardPowerOff() {
  pinMode(POWER_BUTTON, OUTPUT);
  digitalWrite(POWER_BUTTON, LOW);
  delay(6000);
  digitalWrite(POWER_BUTTON,HIGH);
  pinMode(POWER_BUTTON, INPUT);
  server.send(200, "application/json", "{\"status\":\"complete\"}");
}

// Turn on relay, then press power.
// Parameter 0 = just do that.
// Parameter 1 = also wait until computer shuts down, then automatically flip relay off.
void powerOnGracefully() {
  //Relay on
  digitalWrite(RELAY_PIN, HIGH);
  relayOn = 1;
  delay(5000);
  //Then power computer on as normal
  powerOn();
  computerOn = 1;
  //Setup auto off if requested
  //int autoOff = command.toInt()
  //if (autoOff == 1) {
  if (1) {
    relayOffWhenPowerDown = 1;
  } else {
    relayOffWhenPowerDown = 0;
  }
  server.send(200, "application/json", "{\"status\":\"complete\"}");
}

// Literally returns the temperature
float getTemperature() {
  return dht.readTemperature(true);
}

// Send temp / humidity update
void checkTempHumid() {
  float bob = getTemperature();
  dtostrf(getTemperature(), -6, 2, temp); // Leave room for too large numbers!

  Serial.print("temp: ");
  Serial.println(temp);
  sendTempUpdate();
}

void sendTempUpdate() {
  HTTPClient http;
  http.begin("http://10.0.0.21:8086/write?db=sensors");
  http.POST("weather,location=PLAYHOUSE Temperature=" + String(temp));
  http.writeToStream(&Serial);
  http.end();
}
