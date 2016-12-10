//
//
//

#include "TimeManager.h"
#include "LEDHelper.h"

TimeManager Time_Manager = TimeManager();

// Set up the RTC.  Also loads RTC time to get time faster
void TimeManager::RTCSetup() {
  time_t RTCTime = RTC.get();  //Get RTC time
  setTime(RTCTime);            //Set it as current time
  RTC.squareWave(SQWAVE_NONE); //Never assume the Rtc was last configured by you, so just clear them to your needed state
  LED_Helper.updateDigits();              //Force update of display while loop() not running
}

// Initial set up of NTP
void TimeManager::beginNTP() {
  Serial.print("Starting NTP client...");
  NTP.begin("pool.ntp.org", -8, true);    //-8 is pacific time, true means dst
  NTP.setInterval(21600);                 //every 6 hours

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

int TimeManager::getHours() {
  //If non-military time & 13-23 time, subtract 12 to get real hours...
  if (militaryTime && hour() > 12)
    return hour() - 12;
  //If non-military time & 0, set hour to 12...
  else if (militaryTime && hour() == 0)
    return 12;
  else
    return hour();
}

int TimeManager::getMinutes() {
  return minute();
}

int TimeManager::getSeconds() {
  return second();
}
