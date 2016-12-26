//----------------------------------------------------------------------------------------------------------------
// roof_sensor_serial.ino
// 
// This sketch only runs on an Arduino Nano, due to specific AVR functions called for voltage reading.
//
// Handles reading temperature, humidity, rain & wind speed for a weather station.
// Transmits back results via serial for a ESP-based device in the attic to process and send via MQTT.
// 
// Author - Joshua Villwock
// Created - 2016-02-20
// License - Mozilla Public License 2.0 (Do what you want, credit the author, must release under same license)
//----------------------------------------------------------------------------------------------------------------

#include <DHT.h>

#define LED_PIN    13 // LED connected to digital pin 13
#define WIND_PIN   2  // Anemometer connected to digital (interrupt) pin 2
#define RAIN_PIN   3  // Rain Meter connected to digital (interrupt) pin 3

#define TEMP_PIN_1 12         // Temp Sensor connected to digital pin 12
#define TEMP_PIN_1_TYPE DHT22 // Temp Sensor type
#define TEMP_PIN_1_DELAY 30   // How often to update sensor
#define BATTERY_DELAY 120     // How often to update voltage

#define TEXT_RAIN_FLIP "RAIN FLIP" // serial text to print on rain flip
#define TEXT_WIND_UPDATE "W:"      // serial text to print on wind update
#define TEXT_OUTSIDE     "T:"      // serial text to print for temperature
#define TEXT_HUMIDITY    "H:"      // serial text to print for humidity
#define TEXT_BATTERY     "B:"      // serial text to print for voltage

#define WIND_DEBOUNCE_TIME 200     // MICROseconds before accepting more wind
#define RAIN_DEBOUNCE_TIME 2000    // MILLIseconds before accepting new rain

volatile unsigned long windDebounce = 0; // last wind debounce
volatile unsigned long rainDebounce = 0; // last rain debounce
unsigned long lastmillis  = 0;           // last time main loop reported
volatile byte half_revolutions = 0;      // number of wind (half) revolutions since reset
int tempCounter    = 0;                  // counter for how many seconds since last temp reading
int batteryCounter = 0;                  // counter for how many seconds since last battery reading

DHT dht(TEMP_PIN_1, TEMP_PIN_1_TYPE);    // Declare the DHT sensor

// setup code, runs once upon boot:
void setup() {
  pinMode(LED_PIN, OUTPUT);        // set led pin as output
  pinMode(WIND_PIN, INPUT_PULLUP); // enable pullup for wind sensor
  pinMode(RAIN_PIN, INPUT_PULLUP); // enable pullup for rain sensor

  Serial.begin(9600);             // Start serial
  dht.begin();                    // Initialize the DHT sensor
  
  attachInterrupt(digitalPinToInterrupt(WIND_PIN), windInterrupt, FALLING); // attach interrupt handler
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainInterrupt, FALLING); // attach interrupt handler
}

// main code, runs forever:
void loop() {
  if (millis() - lastmillis >= 1000) {    // if it's been more than 1000ms:
    noInterrupts();                         // disable interrupts when calculating
    Serial.print(TEXT_WIND_UPDATE);         // print header
    Serial.println(getWind(), DEC);         // print the rpm to serial

    if (tempCounter > TEMP_PIN_1_DELAY) { // if we've ran this loop 10 times
      updateTemp();                         // then get us temp data
    } else {
      tempCounter++;                        // otherwise, increment loop count
    }

    if (batteryCounter > BATTERY_DELAY) { // if we've ran this loop 10 times
      updateVoltage();                      // then get us temp data
    } else {
      batteryCounter++;                     // otherwise, increment loop count
    }

    lastmillis = millis();                  // update lastmillis
    interrupts();                           // re-enable interrupts
  }
}

// when wind gauge is triggered
void windInterrupt() {
  if (micros() - windDebounce > WIND_DEBOUNCE_TIME) { // if we're not within the debounce time
    half_revolutions++;              // increment the half revolutions
    windDebounce = micros();         // update the last debounce time
  }
}

// when rain gauge is triggered
void rainInterrupt() {
  if (millis() - rainDebounce > RAIN_DEBOUNCE_TIME) { // if we're not within the debounce time
    Serial.println(TEXT_RAIN_FLIP);     // then output that we flipped.
    rainDebounce = millis();            // update the last debounce time
  }
}

// Called to update & send temp data
void updateTemp() {
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  if ( !isnan(f) && !isnan(h) ) {
    Serial.print(TEXT_OUTSIDE);
    Serial.println(f);
    Serial.print(TEXT_HUMIDITY);
    Serial.println(h);
  }
  tempCounter = 1;
}

//Called to update & send voltage data
void updateVoltage() {
  Serial.print(TEXT_BATTERY);
  Serial.println( readVcc(), DEC );
  batteryCounter = 1;
}

// gets the number of windTurns
// also resets it and handles led blinking
int getWind() {
  if (half_revolutions == 0) { // if no revolutions,
    ledOnFor(5);                 // then flash the led
  }
  int rpm = half_revolutions;
  half_revolutions = 0;          // reset revolutions
  return rpm;                    // return result
}

// turns LED on, waits 'time' ms, turns LED off
void ledOnFor(int time) {
  digitalWrite(13, HIGH); // set the LED on
  delay(time);            // wait the desired amount of time
  digitalWrite(13, LOW);  // set the LED off
}

// Measures the 5v power rail using the internal 1.1v vref backwards.
// http://provideyourown.com/2012/secret-arduino-voltmeter-measure-battery-voltage/
long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(2);                        // Wait for Vref to settle
  ADCSRA |= _BV(ADSC);             // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL;             // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH;             // unlocks both

  long result = (high<<8) | low;   // combine result

  result = 1125300L / result;      // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result;                   // Vcc in millivolts
}
