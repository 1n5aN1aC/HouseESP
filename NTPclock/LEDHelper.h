//
//
//

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
