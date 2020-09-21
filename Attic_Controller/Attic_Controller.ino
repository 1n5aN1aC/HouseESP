//----------------------------------------------------------------------------------------------------------------
// Attic_Controller.ino
// 
// This sketch is designed to run on a WeMos D1 mini.  Other controllers will require extensive tweaking.
//
// Handles communicating with the roof sensors, as well as reading attic temperature / humidity
// Processes that information, and passes it on via MQTT
//
// Author - Joshua Villwock
// Created - 2016-11-13
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <Arduino.h>
#include <DHT.h>               //Temperature
#include <ESP8266WiFi.h>       //WiFi
#include <ESP8266HTTPClient.h> //Reporting
#include <ESP8266mDNS.h>       //OTA
#include <WiFiUdp.h>           //OTA
#include <ArduinoOTA.h>        //OTA
#include "Attic_Controller.h"  //Functions
#include "Options.cpp"         //User Options
extern "C" {
  #include "user_interface.h"
}

//DHT Variables
#define DHTTYPE DHT22     // DHT 22  (AM2302), AM2320, AM2321
bool fahrenheit = true;
DHT dht(DHTPin, DHTTYPE);

unsigned long lastTempHumidSend = millis();
#define TEMP_HUMID_UPDATE_FREQUENCY 60000

//Serial Receive settings
#define MAXCHARS  32        // Max chars before a line break
#define ENDMARKER '\n'      // Character that defines the end of a line
char serialChars[MAXCHARS]; // Array to store received data
static byte next_loc = 0;   // Array location for next char


// Initial set up routines
void setup() {
  Serial.begin(9600);
  
  WiFi.mode(WIFI_STA);
  initWifi();
  initOTA();
  
  dht.begin();
}

// Main program loop
void loop() {
  checkTempHumid();
  checkSerial();
  yield();
  delay(100); //saves considerable power & heat
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
}

//Start OTA
void initOTA() {
  ArduinoOTA.setHostname(DEVICE_NAME);
  ArduinoOTA.setPassword(OTA_PASS);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

// Send temp / humidity update if need be
void checkTempHumid() {
  if (millis() > lastTempHumidSend + TEMP_HUMID_UPDATE_FREQUENCY) {

    char temp[8]; // Buffer big enough for 7-character float
    dtostrf(getTemperature(), -6, 2, temp); // Leave room for too large numbers!

    char humid[8];
    dtostrf(getHumidity(), -6, 2, humid);

    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ATTIC Temperature=" + String(temp) + ",Humidity=" + String(humid));
    http.writeToStream(&Serial);
    http.end();
    lastTempHumidSend = millis();
  }
}

float getTemperature() {
  return dht.readTemperature(fahrenheit);
}

float getHumidity() {
  return dht.readHumidity();
}

// Check for (and handle) any serial data sent to us from the roof sensor
// Adapted from:  http://forum.arduino.cc/index.php?topic=288234.0
void checkSerial() {
  boolean newData = false;    // If we have a finished data packet

  //Get all the serial bytes we can until there is no more, or there is a newline.
  while (Serial.available() > 0 && newData == false) {
    char rc = Serial.read();

    //If a regular character, append to the list
    if (rc != ENDMARKER) {
      serialChars[next_loc] = rc;
      next_loc++;
      //If it's more than the max length, just overight the last char
      if (next_loc >= MAXCHARS) {
        next_loc = MAXCHARS - 1;
      }
    }
    //If a finishing character, then mark as complete
    else {
      serialChars[next_loc] = '\0'; // terminate the string
      next_loc = 0;
      newData = true;
    }
  }

  //Once a full line has been found, send to processing.
  if (newData == true) {
    handleSerial();
    newData = false;
  }
}

// Process a complete serial line, and publish results to MQTT
void handleSerial() {
  char cleanChars[MAXCHARS];  //serialChars, with first 2 chars 'removed'
  strncpy(cleanChars, serialChars, MAXCHARS);
  for(int i=2; i < MAXCHARS; i++) {
    cleanChars[i-2] = cleanChars[i];
  }
  String stringChars = String(cleanChars);
  stringChars.replace("\r", "");

  // Check the first character, and process the packet based on that.
  //Wind Update
  if (serialChars[0] == 'W') {
    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ROOF WindSpeed=" + String(stringChars));
    http.writeToStream(&Serial);
    http.end();
  }
  //Temperature Update
  if (serialChars[0] == 'T') {
    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ROOF Temperature=" + String(stringChars));
    http.writeToStream(&Serial);
    http.end();
  }
  //Humidity Update
  if (serialChars[0] == 'H') {
    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ROOF Humidity=" + String(stringChars));
    http.writeToStream(&Serial);
    http.end();
  }
  //'Battery' Update
  if (serialChars[0] == 'B') {
    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ROOF Battery=" + String(stringChars));
    http.writeToStream(&Serial);
    http.end();
  }
  //Rain Flip
  if (serialChars[0] == 'R') {
    HTTPClient http;
    http.begin("http://10.0.0.21:8086/write?db=sensors");
    http.POST("weather,location=ROOF RainFlip=" + String(1));
    http.writeToStream(&Serial);
    http.end();
  }
}
