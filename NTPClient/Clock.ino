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

LedControl lc1=LedControl(D5,D6,D7,1); //Initialize MAX7219
char timeDigits[6] = "----00";           //Most recent 

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  // We start by connecting to a WiFi network
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
  
  Serial.print("Starting NTP client...  ");
  NTP.begin("pool.ntp.org", 4, true);
  NTP.setInterval(60);
  delay(2000);
  Serial.println("Done.");

  lc1.shutdown(0,false);
  lc1.setIntensity(0, 10);
}

//
void loop()
{
  String timeDisplay = NTP.getTimeStr();
  char minDig2 = timeDisplay.charAt(4);
  lc1.setChar(0, 0, minDig2, true);
  char secDig1 = timeDisplay.charAt(6);
  lc1.setChar(0, 1, secDig1, false);
  char secDig2 = timeDisplay.charAt(7);
  lc1.setChar(0, 2, secDig2, false);
  
  time_t t = now(); // store the current time in time variable t
  hour(t);          // returns the hour for the given time t
  minute(t);        // returns the minute for the given time t
  second(t);        // returns the second for the given time t
  day(t);           // the day for the given time t
  weekday(t);       // day of the week for the given time t
  month(t);         // the month for the given time t
  year(t);          // the year for the given time t
  
  Serial.println(NTP.getTimeStr());
  delay(1000);  //   REMOVE DELAY FOR A COUNTER.................................................../////////////////////////
}

void () {
	
}
