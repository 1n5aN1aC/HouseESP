//----------------------------------------------------------------------------------------------------------------
// LEDHelper.h
// 
// Manages controlling the MAX7219 chip & any other LEDs.
// For ease, we define a global helper object, that then is used for all operations
// 
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#ifndef __LEDHelper_H__
#define __LEDHelper_H__

#include <LedControl.h>   // MAX7219 display control library

class LEDHelper
{
  int brightness = 7;
  LedControl LEDs = LedControl(D5,D6,D7,1); // Initialize MAX7219
  void setBar(int bars);
  
public:
  void LED_Setup();
  void set_brightness(int brightness);
  void updateDigits();
  void updateMisc();
};

extern LEDHelper LED_Helper;

#endif //__LEDHelper_H__
