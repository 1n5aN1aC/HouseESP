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
  lc1.setIntensity(0, 10);
}

// Main program loop
void loop() {
  String timeDisplay = NTP.getTimeStr();
  char minDig1 = timeDisplay.charAt(3);
  char minDig2 = timeDisplay.charAt(4);
  char secDig1 = timeDisplay.charAt(6);
  char secDig2 = timeDisplay.charAt(7);
  lc1.setChar(0, 0, minDig1, false);
  lc1.setChar(0, 1, minDig2, false);
  lc1.setChar(0, 2, secDig1, false);
  lc1.setChar(0, 3, secDig2, false);
  
  lc1.setChar(0, 4, secDig2, false); //test for progress bar
  
  Serial.println(timeDisplay);
  
  time_t time = now(); // store the current time in time variable t
  hour(t);             // returns the hour for the given time t
  minute(t);           // returns the minute for the given time t
  second(t);           // returns the second for the given time t
  day(t);              // the day for the given time t
  weekday(t);          // day of the week for the given time t
  month(t);            // the month for the given time t
  year(t);             // the year for the given time t
  
  delay(1000);         //   REMOVE DELAY FOR A COUNTER..............................PLEASE!!!!......../////////////////////////
}

// Inition connection to WiFi
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
  Serial.print("Starting NTP client.");
  NTP.begin("pool.ntp.org", 4, true);
  NTP.setInterval(60);
  while (NTP.getLastNTPSync() == 0) {     //returns a time_t, so not sure this will work or not.
    delay(500);
    Serial.print(".");
  }
  Serial.println("Done.");
}
