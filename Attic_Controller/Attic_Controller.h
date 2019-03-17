//----------------------------------------------------------------------------------------------------------------
// Attic_Controller.h
// 
// Header file for main file.
// 
// Author - Joshua Villwock
// Created - 2019-03-16
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------


void setup();
void loop();

void connectWifi();
float getTemperature();
float getHumidity();
void checkTempHumid();
void handleSerial();
void checkSerial();
