//----------------------------------------------------------------------------------------------------------------
// LEDHelper.cpp
//
// Manages controlling the MAX7219 chip & any other LEDs.
// For ease, we define a global helper object, that then is used for all operations
//
// Author - Joshua Villwock
// Created - 2016-12-09
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include "LEDHelper.h"
#include "TimeManager.h"
#include "MQTTHelper.h"

LEDHelper LED_Helper = LEDHelper();

//Called once to turn on the LEDs
void LEDHelper::LED_Setup() {
  LEDs.shutdown(0, false);    //Turn the display on
  set_brightness(brightness); //Just temporary brightness.  Set over MQTT
}

//Set the new brightness
void LEDHelper::set_brightness(int new_brightness) {
  brightness = new_brightness;
  LEDs.setIntensity(0, brightness);
  if (new_brightness != brightness) {
    char str[10];
    sprintf(str, "%d", new_brightness);
    MQTT_Helper.publishMQTT("home/jroom/clock/brightness", str);
  }
}

// Handles updating the primary 7-segment display
void LEDHelper::updateDigits() {
  int theHour = Time_Manager.getHours();
  //If the hour is less than 10, display an empty far left digit
  if ((theHour / 10) == 0) {
    LEDs.setChar(0, 6, ' ', false);
  }
  //Otherwise, normal hour digit
  else {
    LEDs.setChar(0, 6, (theHour / 10), false);
  }
  LEDs.setChar(0, 0, (theHour % 10), false);
  LEDs.setChar(0, 1, (minute() / 10), false);
  LEDs.setChar(0, 5, (minute() % 10), false);
}

// This is misc stuff that may need updated.  Called once per second
void LEDHelper::updateMisc() {
  int portion = (second() / 6) + 1; //get the number of bars that should be lit
  setBar(portion);                  //and then set them

  //Every other second....
  if (second() % 2 == 1) {
    //Turn second flasher on
    LEDs.setLed(0,0,0, true);
  } else {
    //Turn second flasher off
    LEDs.setLed(0,0,0, false);
  }
}

// Controlls a 10-segment LED bargraph
// int bars: 0-10 number of bars that should be on
void LEDHelper::setBar(int bars) {
  bool segment[10] = {false, false, false, false, false, false, false, false, false, false};
  for (int i=0; i<bars; i++) { //set true up to the number of bars
    segment[i] = true;
  }

  for (int i = 0; i<5; i++) {           //Set the LEDs
    LEDs.setLed(0, 7, i, segment[i]);     //0-4
    LEDs.setLed(0, 3, i, segment[i + 5]); //5-9
  }
}
