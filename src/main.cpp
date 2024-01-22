#include <Arduino.h>
#include <EEPROM.h>

#define STARTING_ADDR 0x0
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
  OR  10000000 (1 << 7)
  ------------
  =   1xxxxxxx

  if changing a bit to 0, 
      xxxxxxxx
  AND 01111111 (!(1 << 7))
  ------------
  =   1xxxxxxx

  Perform modulo 8 by AND of the number and 00000111.

  TLDR: 
    To get a certain bit, first integer divide the hour by 8 to get the nth
    byte, bitwise shift right the nth byte by the modulo of the hour and 8, then
    AND the result with 00000001 to get a 1 or a 0. 
    To switch a bit on, use bitwise OR with a left shift, to switch a bit off, 
  use bitwise AND with the inverse of a left shift. 
  */

typedef struct {
  byte schedule[3];
  short duration; 
} timingconfig;

/*
list of functions:
load the schedule and duration variables from the EEPROM
save the schedule and duration variables to the EEPROM
return the active hours in a day
switch on or off a certain hour in the schedule variables 
set a new duration time
*/
void printTimingConfig(timingconfig tC);
void loadFromEEPROM(unsigned int addr, timingconfig* tC);
void saveToEEPROM(unsigned int addr, timingconfig tC);
bool* getActiveHours(timingconfig tC);
void setHour(timingconfig* tC, int hour, bool newState); 
void clearAllHours(timingconfig* tC); 
void setDuration(timingconfig* tC, int newDuration);

void setup() {
  Serial.begin(115200); 
  // initialize the emulated EEPROM as large as needed
  EEPROM.begin(sizeof(timingconfig));
  // test with some predefined settings 
  /*
  Schedule enabled for 7AM and 7PM and disabled for others
  10000000 00000000 00001000 (128, 0, 8)
  Duration = 20 seconds
  */
  timingconfig tC;

  bool* hours;

  // load previous timing configuration from EEPROM if it exists
  loadFromEEPROM(STARTING_ADDR, &tC);
  Serial.println("previous tC loaded from EEPROM: ");
  printTimingConfig(tC);

  // create timing configuration
  tC.duration = 1;
  tC.schedule[0] = 128;
  tC.schedule[1] = 0;
  tC.schedule[2] = 8;
  Serial.println("tC created: ");
  printTimingConfig(tC);

  // save timing configuration to EEPROM
  Serial.println("Saving tC to EEPROM: ");
  saveToEEPROM(STARTING_ADDR, tC);

  // modify timing configuration in memory
  tC.duration = 40; 
  tC.schedule[1] = 16; // enable 12PM 
  Serial.println("tC modified: ");
  printTimingConfig(tC);

  // load timing configuration from EEPROM
  loadFromEEPROM(STARTING_ADDR, &tC);
  Serial.println("tC reloaded from EEPROM: ");
  printTimingConfig(tC);

  // clear all hours from the timing configuration
  clearAllHours(&tC);
  Serial.println("Cleared all hours from schedule. ");
  printTimingConfig(tC); 

  // set duration of tC to 25
  setDuration(&tC, 25);
  Serial.println("Set tC duration to 25 seconds. ");
  printTimingConfig(tC); 

  // set 00:00, 07:00, 09:00, 13:00, 18:00, 21:00, and 22:00 to enabled
  setHour(&tC, 0, 1);
  setHour(&tC, 7, 1);
  setHour(&tC, 9, 1);
  setHour(&tC, 13, 1);
  setHour(&tC, 18, 1);
  setHour(&tC, 21, 1);
  setHour(&tC, 22, 1);
  Serial.println("set 00:00, 07:00, 09:00, 13:00, 18:00, 21:00, and 22:00 to enabled");
  // expected: 10000001 00100010 01100100 = 129 34 100
  printTimingConfig(tC);

  // print all active hours 
  Serial.print("Active hours = ");
  hours = getActiveHours(tC); 
  for (int h=0;h<24;h++) {
    Serial.print(hours[h]);
    if (h < 23) {
      Serial.print(",");
    }
  }
  Serial.println();

  // set 00:00, 09:00, and 21:00 to disabled
  setHour(&tC, 0, 0);
  setHour(&tC, 9, 0);
  setHour(&tC, 21, 0);
  Serial.println("set 00:00, 09:00, and 21:00 to disabled");
  // expected: 10000000 00100000 01000100 = 128 32 68
  printTimingConfig(tC);

  // save timing configuration to EEPROM
  Serial.println("Saving tC to EEPROM: ");
  saveToEEPROM(STARTING_ADDR, tC); 

  // load timing configuration from EEPROM
  loadFromEEPROM(STARTING_ADDR, &tC);
  Serial.println("tC reloaded from EEPROM: ");
  printTimingConfig(tC);

  // print all active hours 
  Serial.print("Active hours = ");
  hours = getActiveHours(tC); 
  for (int h=0;h<24;h++) {
    Serial.print(hours[h]);
    if (h < 23) {
      Serial.print(",");
    }
  }
  Serial.println();
}

void loop() {
  // put your main code here, to run repeatedly:
}

void loadFromEEPROM(unsigned int addr, timingconfig* tC) {
  EEPROM.get(addr, *tC);
}

void saveToEEPROM(unsigned int addr, timingconfig tC) {
  EEPROM.put(addr, tC);
  EEPROM.commit();
}

void printTimingConfig(timingconfig tC) {
  Serial.print("schedule bytes = ");
  for (int i=0;i<3;i++) {
    Serial.print(tC.schedule[i]);  
    if (i<2)
      Serial.print(", ");
    else Serial.print(" ");
  }
  Serial.print("duration = ");
  Serial.print(tC.duration);
  Serial.println();
}

bool* getActiveHours(timingconfig tC) {
  static bool hours[24];
  // reset the hours array
  for (int i=0;i<24;i++) {
    hours[i] = 0;
  }
  // check each bit in the schedule bytes then load it into the bool hours array 
  for (int i=0;i<24;i++) {
    byte byteIndex = i / 8; 
    byte currByte = tC.schedule[byteIndex]; 
    byte offset = i % 8; 
    currByte = currByte >> offset;
    currByte = currByte & 1; 
    hours[i] = (currByte) ? true: false;
  }
  return hours;
}

/* set the hour in the timing configuration to the specified state. 
args:
  tC - timingConfig object 
  hour - hour of the day from 0-24 
  newState - 0 for disable and 1 for enable 
*/ 
void setHour(timingconfig* tC, int hour, bool newState) {
  int byteIndex = hour / 8;
  int bitIndex = hour % 8; 
  int mask = 1 << bitIndex; 
  Serial.print("mask = ");
  // if enabling the hour
  if (newState) {
    tC->schedule[byteIndex] = tC->schedule[byteIndex] | mask;
  }
  // else disabling the hour
  else {
    mask = ~mask;
    tC->schedule[byteIndex] = tC->schedule[byteIndex] & mask;
  }
  Serial.println(mask);
}

// set the duration of the timing configuration 
void setDuration(timingconfig* tC, int newDuration) {
  tC->duration = newDuration;
}

// macro function to clear all hours in the schedule and reset interval to 0
void clearAllHours(timingconfig* tC) {
  setDuration(tC, 0);
  for (int h=0; h<24; h++) {
    setHour(tC, h, 0);
  }
}


