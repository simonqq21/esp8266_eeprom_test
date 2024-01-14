#include <Arduino.h>
#include <EEPROM.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h> 
#include <WiFiUdp.h>
#include <TimeLib.h>

#define LOCAL_SSID "QUE-STARLINK"
#define LOCAL_PASS "Quefamily01259"

// configuration variables 
/*  The configuration variables are the ff:
- a list of three bytes where the 24 hours per day are represented. The data is 
  stored in little endian order, so the organization of 24 hours into three bytes
  will be arranged as below:

  address |       00                    01                       02
  hours   | 7 6 5 4 3 2 1 0 | 15 14 13 12 11 10 9 8 | 23 22 21 20 19 18 17 16
  
  so if a bit is high, the relay will close contacts for the set duration at the start
  of that hour before opening contacts for the rest of the hour, but if that bit is 
  low, then the relay will remain open contact for that hour.

  eg. if the relay must close at 7AM and remain open the rest of the time,
  7 / 8 = 0, byte 0.  0000 0111 >> 3 = 0
  7 % 8 = 7, 7th bit. 0000 0111 & 
  change byte 0, 7th bit to 1. 
      xxxxxxxx
  or  10000000 (1 << 7)
  ------------
  =   1xxxxxxx

  if changing a bit to 0, 
      xxxxxxxx
  and 01111111 (!(1 << 7))
  ------------
  =   1xxxxxxx

  TLDR: To switch a bit on, use bitwise or with a left shift, to switch a bit off, 
  use bitwise and with the inverse of a left shift. */

/*
list of functions:
load the schedule variables from the EEPROM
print the active hours 
switch on a certain hour in the schedule variables 
switch off a certain hour in the schedule variables 
set all hours in the schedule variables to OFF
save the schedule variables to the EEPROM
load the ON duration from the EEPROM 
save the ON duration to the EEPROM
*/

void setup() {
  Serial.begin(115200); 
}

void loop() {
  // put your main code here, to run repeatedly:
}
