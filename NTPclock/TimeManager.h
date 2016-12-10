//
//
//

#ifndef __TimeManager_H__
#define __TimeManager_H__

#include <DS3232RTC.h>    // RTC Library
#include <TimeLib.h>      // This library helps with easy time-keeping
#include <NtpClientLib.h> // The NTP library

class TimeManager
{
  bool militaryTime = false;
  
public:
  void beginNTP();
  void RTCSetup();
  int getHours();
  int getMinutes();
  int getSeconds();
};

extern TimeManager Time_Manager;

#endif //__TimeManager_H__
