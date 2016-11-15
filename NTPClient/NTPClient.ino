/*
 Udp NTP Client

 Get the time from a Network Time Protocol (NTP) time server
 Demonstrates use of UDP sendPacket and ReceivePacket
 For more on NTP time servers and the messages needed to communicate with them,
 see http://en.wikipedia.org/wiki/Network_Time_Protocol

 created 4 Sep 2010
 by Michael Margolis
 modified 9 Apr 2012
 by Tom Igoe
 updated for the ESP8266 12 Apr 2015 
 by Ivan Grokhotkov

 This code is in the public domain.
*/

#include <LedControl.h>
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <NtpClientLib.h>

char ssid[] = "joshua"; // your network SSID (name)
char pass[] = "";       // your network password

LedControl lc1=LedControl(D5,D6,D7,1);
String timeDisplay;

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
  delay(1000);
  Serial.println("Done. ");

  lc1.shutdown(0,false);
  //lc1.setIntensity(0, 15);
}

//
void loop()
{
  timeDisplay = NTP.getTimeStr();
  char minDig2 = timeDisplay.charAt(4);
  lc1.setChar(0, 0, minDig2, true);
  char secDig1 = timeDisplay.charAt(6);
  lc1.setChar(0, 1, secDig1, false);
  char secDig2 = timeDisplay.charAt(7);
  lc1.setChar(0, 2, secDig2, false);
  
  Serial.println(NTP.getTimeStr());
  delay(1000);  //   REMOVE DELAY FOR A COUNTER.................................................../////////////////////////
}
