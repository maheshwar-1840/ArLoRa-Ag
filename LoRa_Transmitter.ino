#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "RTClib.h"

#define ONE_WIRE_BUS1 6
#define ONE_WIRE_BUS2 5

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const int analogInPin1 = A0;
const int analogInPin2 = A1;
bool is1V1Output = false;
float refVoltage = 5;
int rawValue1;
int rawValue2;
float voltage1,vwcValue1,voltage2,vwcValue2;

OneWire oneWire(ONE_WIRE_BUS1);
OneWire twoWire(ONE_WIRE_BUS2);
DallasTemperature sensor1(&oneWire);
DallasTemperature sensor2(&twoWire);
float Celcius1=0;
float Celcius2=0;

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  #ifndef ESP8266 //RTC Configuration..........present date and time.
    while (!Serial); // for Leonardo/Micro/Zero
  #endif
  if (! rtc.begin()) {
      Serial.println("Couldn't find RTC");
      while (1);
  }
  if (rtc.lostPower()) {
      Serial.println("RTC lost power, lets set the time!");
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  Serial.println("LoRa Sender");
  sensor1.begin();//sensor initiation
  sensor2.begin();
   if (is1V1Output == true)
   {
    analogReference(INTERNAL); //set ADC reference to internal 1.1V
    refVoltage = 1.1;
  } 
  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  
  
}

void loop() 
{
  rawValue1 = analogRead(analogInPin1);
  rawValue2 = analogRead(analogInPin2);
  
  voltage1 = rawValue1 * refVoltage / 1023.0f;
  voltage2 = rawValue2 * refVoltage / 1023.0f;
  
  if (is1V1Output == true)
  {
    //multiply the voltage to mach 3V version VWC equation of the sensor
    voltage1 = voltage1 * 2.94;
    voltage2 = voltage2 * 2.94;

  }
  
  vwcValue1 = (5.5667f * voltage1 * voltage1 * voltage1) - (19.6738f * voltage1 * voltage1) + (31.0893f * voltage1) - 6.7511f;
  vwcValue2 = (5.5667f * voltage2 * voltage2 * voltage2) - (19.6738f * voltage2 * voltage2) + (31.0893f * voltage2) - 6.7511f;
  
  DateTime now = rtc.now();

  sensor1.requestTemperatures(); 
  Celcius1=sensor1.getTempCByIndex(0);
  sensor2.requestTemperatures(); 
  Celcius2=sensor2.getTempCByIndex(0);
  
  Serial.print("Sending packet: ");
  Serial.println(counter);
  // send packet
  LoRa.beginPacket();
  LoRa.print(now.year(), DEC);
  LoRa.print('/');
  LoRa.print(now.month(), DEC);
  LoRa.print('/');
  LoRa.print(now.day(), DEC);
  LoRa.print(',');
  LoRa.print(now.hour(), DEC);
  LoRa.print(':');
  LoRa.print(now.minute(), DEC);
  LoRa.print(':');
  LoRa.print(now.second(), DEC);
  LoRa.print(',');
  LoRa.print("Sender 2:");
  LoRa.print(",");
  LoRa.print(counter);
  LoRa.print(", ");
  LoRa.print("S2M1:");
  LoRa.print(",");
  LoRa.print(vwcValue1);
  LoRa.print(",");
  LoRa.print("S2M2: ");
  LoRa.print(",");
  LoRa.print(vwcValue2);
  LoRa.print(",");
  LoRa.print("S2T1: ");
  LoRa.print(",");
  LoRa.print(Celcius1);
  LoRa.print(",");
  LoRa.print("S2T2: ");
  LoRa.print(",");
  LoRa.print(Celcius2);
  LoRa.endPacket();

  counter++;

  delay(5000);
}