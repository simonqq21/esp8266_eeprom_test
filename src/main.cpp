#include <Arduino.h>
#include <EEPROM.h>

#define SCHEDULE_ADDR 0x00001000
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
void loadFromEEPROM(timingconfig* tC);
void saveToEEPROM(timingconfig tC);
bool* getActiveHours(timingconfig tC);
void setHour(timingconfig* tC, int hour, bool newState); 

void setup() {
  Serial.begin(115200); 
  // initialize the emulated EEPROM as large as needed
  EEPROM.begin(sizeof(timingconfig));
  // test with some predefined settings 
  timingconfig tC;
  tC.duration = 20;
  Serial.print("Duration=");
  Serial.println(tC.duration);
}

void loop() {
  // put your main code here, to run repeatedly:
}

void loadFromEEPROM(timingconfig* tC) {
  unsigned int addr = 0;
  EEPROM.get(addr, *tC);
}

void saveToEEPROM(timingconfig tC) {
  unsigned int addr = 0;
  EEPROM.put(addr, tC);
  EEPROM.commit();
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
    Serial.println(currByte);
    hours[i] = (currByte) ? true: false;
  }
  return hours;
}

void setHour(timingconfig tC, int hour, bool newState) {

}
