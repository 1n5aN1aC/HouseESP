//----------------------------------------------------------------------------------------------------------------
// Clock.ino
// 
// Controls a LED clock using an ESP8266, several 7-segment displays, and a bargraph display
// Uses NTP for internet time updating, and supports a Real-Time clock for outages.
// Some extra features include a temperature & humidity sensor, brightness control, time zone control.
// 
// Author - Joshua Villwock
// Created - 2016-11-13
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <pgmspace.h>     // Used by RTC Library.  Specific for ESP8266
#include <DS3232RTC.h>    // RTC Library
#include <Wire.h>         // Incuded here so that Arduino library object file references work
#include <LedControl.h>   // MAX7219 display control library
#include <ESP8266WiFi.h>  // We need to use the wifi for NTP
#include <TimeLib.h>      // This library helps with easy time-keeping
#include <NtpClientLib.h> // The NTP library

//---------------------------------------------------------//
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
char ssid[] = "joshua"; // your network SSID (name)        //
char pass[] = "";       // your network password           //
//---------------------------------------------------------//

LedControl lc1 = LedControl(D5,D6,D7,1); // Initialize MAX7219
unsigned long timeLastBlinked = millis();
unsigned long timeLastUpdated = millis();

bool militaryTime = false;

// Initial set up routines
void setup() {
  Serial.begin(115200);
  Serial.println();

  lc1.shutdown(0,false);
  lc1.setIntensity(0, 10);     //TODO: Needs moved out to seperate function.  Plus controls to raise / lower.

  RTCSetup();    //Restore RTC time immediently to current time
  connectWifi(); //Then connect to wifi
  beginNTP();    //Start up NTP Client & time keeping

  //Called on NTP update.
  NTP.onNTPSyncEvent([](NTPSyncEvent_t ntpEvent) {
    if (ntpEvent == 0) {
      Serial.print(NTP.getTimeDateString(NTP.getLastNTPSync()));
      Serial.println(" NTP UPDATE!");
      RTC.set(NTP.getLastNTPSync());
    }
  });
}

// Main program loop
void loop() {
  if (millis() > (timeLastUpdated + 1000)) {
    updateDigits(); //Update the time display
    updateMisc();   //Update the rest of the display
    
    Serial.println(NTP.getTimeDateString(now() ) );
    
    timeLastUpdated = millis();
  }
}

// Handles updating the primary 7-segment display
void updateDigits() {
  //If non-military time & 13-23 time, subtract 12 to get real hours...
  int theHour = hour();
  if (!militaryTime && theHour > 12) {
    theHour = hour() - 12;
  }
  //If the hour is less than 10, display an empty far left digit
  if ((theHour / 10) == 0) {
    lc1.setChar(0, 6, ' ', false);
  }
  //Otherwise, normal hour digit
  else {
    lc1.setChar(0, 6, (theHour / 10), false);
  }
  lc1.setChar(0, 0, (theHour % 10), false);
  lc1.setChar(0, 1, (minute() / 10), false);
  lc1.setChar(0, 5, (minute() % 10), false);
}

// 
void updateMisc() {
  int portion = (second() / 6) + 1; //get the number of bars that should be lit
  setBar(portion);                  //and then set them

  //Every other second....
  if (second() % 2 == 1) {
    lc1.setLed(0,0,0,true);
  } else {
    lc1.setLed(0,0,0,false);
  }
}

// Controlls a 10-segment LED bargraph
// int bars: 0-10 number of bars that should be on
void setBar(int bars) {
  bool segment[10] = {false, false, false, false, false, false, false, false, false, false};
  for (int i = 0; i<bars; i++) {   //set true up to the number of bars
    segment[i] = true;
  }
  
  for (int i=0; i<5; i++) {         //Set the LEDs
    lc1.setLed(0,7,i,segment[i]);   //0-4
    lc1.setLed(0,3,i,segment[i+5]); //5-9
  }
}

// Initial connection to WiFi
// We wait for 5 seconds to connect, but do not block on the connection.
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  delay(5000);
}

// Set up the RTC.  Also loads RTC time to get time faster
void RTCSetup() {
  time_t RTCTime = RTC.get();  //Get RTC time
  setTime(RTCTime);            //Set it as current time
  RTC.squareWave(SQWAVE_NONE); //Never assume the Rtc was last configured by you, so just clear them to your needed state
  updateDigits();              //Force update of display while loop() not running
}

// Initial set up of NTP
void beginNTP() {
  Serial.print("Starting NTP client...");
  NTP.begin("pool.ntp.org", -8, true);    //-8 is pacific time, true means dst
  NTP.setInterval(21600);                 //every 6 hours
  Serial.println("Done.");
}
