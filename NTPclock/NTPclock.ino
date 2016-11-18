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


#include <LedControl.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

//---------------------------------------------------------//
//              CONFIGURE YOUR NETWORK HERE                //
//---------------------------------------------------------//
char ssid[] = "joshua"; // your network SSID (name)        //
char pass[] = "";       // your network password           //
//---------------------------------------------------------//

LedControl lc1 = LedControl(D5,D6,D7,1); //Initialize MAX7219
char timeDigits[] = "----";              //String represntation of current time

// Initial set up routines
void setup() {
  Serial.begin(115200);
  Serial.println();

  connectWifi(); //First, connect to wifi
  beginNTP();    //Start up NTP Client & time keeping

  lc1.shutdown(0,false);
  lc1.setIntensity(0, 10);     //TODO: Needs moved out to seperate function.  Plus controls to raise / lower.
}

// Main program loop
void loop() {
  String timeDisplay = NTP.getTimeStr();
  timeDigits[0] = timeDisplay.charAt(3);
  timeDigits[1] = timeDisplay.charAt(4);
  timeDigits[2] = timeDisplay.charAt(6);
  timeDigits[3] = timeDisplay.charAt(7);
  for (int i=0; i<4; i++) {
    Serial.print(i);
    lc1.setChar(0, i, timeDigits[i], false);
  }
  lc1.setChar(0, 4, timeDigits[3], false); //test for progress bar
  lc1.setRow(0, 5, 31);
  
  Serial.println(timeDisplay);
  
  time_t nowTime = now(); // store the current time in time variable t
  hour(nowTime);             // returns the hour for the given time t
  minute(nowTime);           // returns the minute for the given time t
  second(nowTime);           // returns the second for the given time t
  day(nowTime);              // the day for the given time t
  weekday(nowTime);          // day of the week for the given time t
  month(nowTime);            // the month for the given time t
  year(nowTime);             // the year for the given time t
  
  delay(1000);         //TODO:   REMOVE DELAY FOR A COUNTER..............................PLEASE!!!!......../////////////////////////
}

// Controlls a 10-segment LED bargraph
// int bars: 0-10
void setBar(int bars) {
  
  
  for (int i=0; i<10; i++) {
    
  }

  switch(c) {
  case 0:
    lc.setLed(0,r,e,state); 
    break;

  case 1:
    lc.setLed(0,r+4,e,state); 
    break;

  case 2:
    lc.setLed(0,r,e,state); 
    lc.setLed(0,r+4,e,state);     
    break;
  }
}

// Initial connection to WiFi
void connectWifi() {
  Serial.print("Connecting to ");
  Serial.print(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("  Done.");
  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Initial set up of NTP
void beginNTP() {
  Serial.print("Starting NTP client...");
  NTP.begin("pool.ntp.org", 4, true);
  NTP.setInterval(60);
  Serial.println("Done.");
}
