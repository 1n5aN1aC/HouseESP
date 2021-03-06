//----------------------------------------------------------------------------------------------------------------
// TimeManager.cpp
//
// Manages all time-related functions for the clock.  (Time, NTP, RTC)
// For ease, we define a global object that should be used for all time-related functions
//
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include "TimeManager.h"
#include "LEDHelper.h"

#include <DS3232RTC.h>    // RTC Library
#include <TimeLib.h>      // Must be included AFTER DS3232RTC!!!
#include <Wire.h>         // Included here so that Arduino library object file references work

TimeManager Time_Manager = TimeManager();

// Set up the RTC.  Also loads RTC time to get time faster
void TimeManager::RTCSetup() {
  time_t RTCTime = RTC.get();   //Get RTC time
  setTime(RTCTime);             //Set it as current time
  RTC.squareWave(SQWAVE_NONE);  //Never assume the RTC was last configured by you, so just clear them to your needed state
  LED_Helper.updateDigits();    //Force update of display while loop() not running
}

// Initial set up of NTP
void TimeManager::beginNTP() {
  Serial.print("Starting NTP client...");
  NTP.begin("pool.ntp.org", -8, true);    //-8 is pacific time, true means dst
  NTP.setInterval(86400);                 //every 24 hours

  //Called on NTP update.
  NTP.onNTPSyncEvent([](NTPSyncEvent_t ntpEvent) {
    if (ntpEvent == 0) {
      Serial.print(NTP.getTimeDateString(NTP.getLastNTPSync()));
      Serial.println(" NTP UPDATE!");
      RTC.set(NTP.getLastNTPSync());
    }
  });

  Serial.println("Done.");
}

// Enables or Disables military time
void TimeManager::setMilitary(boolean military) {
  if (military != militaryTime) {
    //update MQTT
  }
  militaryTime = military;
}

// Return the current hour, depending on the military time setting
int TimeManager::getHours() {
  //If non-military time & 13-23 time, subtract 12 to get real hours...
  if (!militaryTime && hour() > 12)
    return hour() - 12;
  //If non-military time & 0, set hour to 12...
  else if (!militaryTime && hour() == 0)
    return 12;
  else
    return hour();
}

// gets the temperature from the RTC.  Also does unit conversion
float TimeManager::getTemperature() {
  int temp = RTC.temperature();
  float tempC = temp / 4.0;
  float tempF = tempC * 9.0 / 5.0 + 32.0;

  if (fahrenheit)
    return tempF;
  else
    return tempC;
}
