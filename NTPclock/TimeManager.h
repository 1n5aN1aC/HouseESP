//----------------------------------------------------------------------------------------------------------------
// TimeManager.h
// 
// Manages all time-related functions for the clock.  (Time, NTP, RTC)
// For ease, we define a global object that should be used for all time-related functions
// 
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#ifndef __TimeManager_H__
#define __TimeManager_H__

#include <TimeLib.h>      // This library helps with easy time-keeping
#include <NtpClientLib.h> // The NTP library

class TimeManager
{
  bool militaryTime = false;
  
public:
  void beginNTP();
  void RTCSetup();
  int getHours();
};

extern TimeManager Time_Manager;

#endif //__TimeManager_H__
