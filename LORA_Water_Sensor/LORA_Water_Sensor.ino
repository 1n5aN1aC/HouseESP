//----------------------------------------------------------------------------------------------------------------
// LORA_Water_Sensor.ino
// 
// This sketch is designed to run on a TTGO LoRa v1 board.  Other controllers may require extensive tweaking.
//
// This uses a UltraSonic Distance sensor to determine the water level in a large tank.
// Then it relays the results back to a receiving node via LoRa.
//
// Author - Joshua Villwock
// Created - 2020-09-14
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>  
#include "SSD1306.h"
#include "esp_deep_sleep.h"

#define echoPin 12 // Echo Pin
#define trigPin 13 // Trigger Pin

/////////////////////////////////////////////////////////////
#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     14   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    433E6
#define LORA_SPREAD     12    //Supported values are between 6 and 12. If a spreading factor of 6 is set, implicit header mode must be used to transmit and receive packets.
#define LORA_BANDWIDTH  125E3 //Supported values are 7.8E3, 10.4E3, 15.6E3, 20.8E3, 31.25E3, 41.7E3, 62.5E3, 125E3, 250E3, and 500E3
#define LORA_CODINGRATE 8     //Supported values are between 5 and 8, these correspond to coding rates of 4/5 and 4/8. The coding rate numerator is fixed at 4.
#define LORA_PRELENGTH  8     //Supported values are between 6 and 65535.
#define LORA_CRC        true  //Default: false
/////////////////////////////////////////////////////////////

String rssi = "RSSI --";
String packSize = "--";
String packet ;

void setup() {
  pinMode(16,      OUTPUT);
  pinMode(2,       OUTPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  //digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  //delay(50); 
  //digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high
  
  Serial.begin(115200);
  while (!Serial);
  
  SPI.begin(SCK,MISO,MOSI,SS);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  LoRa.setSpreadingFactor(LORA_SPREAD);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setCodingRate4(LORA_CODINGRATE);
  LoRa.setPreambleLength(LORA_PRELENGTH);
  if (LORA_CRC)
    LoRa.enableCrc();

  delay(1000);
}

void loop() {
  int distance = checkDistance();
  Serial.println(distance);
  sendPacket("W:" + String(distance));

  //delay(10000);
  // Deep Sleep Instead:
  LoRa.end();
  LoRa.sleep();
  pinMode(19, INPUT);
  pinMode(18, INPUT);
  pinMode(5,  INPUT);
  pinMode(15, INPUT);
  pinMode(2,  INPUT);
  pinMode(4,  INPUT);
  pinMode(16, INPUT);
  pinMode(26, INPUT);
  pinMode(27, INPUT);
  pinMode(14, INPUT);
  delay(100);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);
  esp_deep_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);
  esp_sleep_enable_timer_wakeup(10000000);
  esp_deep_sleep_start();
}

void sendPacket(String contents) {
  // send packet
  LoRa.beginPacket();
  String packet = (contents);
  LoRa.print(packet);
  LoRa.endPacket();
}

// Measure distance 5 times, and return the mode. (Middle result)
int checkDistance() {
  int results[5];
  for (int i=0; i<5; i++) {
    results[i] = sonicMeasure();
    delay(20);
  }

  //Quicksort the results.
  int results_length = sizeof(results) / sizeof(results[0]);
  qsort(results, results_length, sizeof(results[0]), sort_desc);

  return results[2];
}

//Uses a ultrasonic sensor to determine the distance by bouncing soundwaves off of it.
int sonicMeasure() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  //Calculate the distance (in cm) based on the speed of sound.
  int distance = duration/58.2 + 1;
  return distance;
}

// Comparison function used for Quicksort. Taken from here:
// https://arduino.stackexchange.com/questions/38177/how-to-sort-elements-of-array-in-arduino-code
int sort_desc(const void *cmp1, const void *cmp2) {
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}
