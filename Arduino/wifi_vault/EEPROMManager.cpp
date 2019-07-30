#include "Arduino.h"
#include "EEPROMManager.h"
#include <EEPROM.h>

int EEPROMManager::findEmptyAddress() {
  for (int i = 0; i < EEPROM_SIZE; i += sizeof(PasswordObject)) {
    int status = getEE(i, EMPRY_ADDRESS);
    if (status == EMPRY_ADDRESS) {
      return i;
    }
  }

  return NULL_ADDRESS;
}

int EEPROMManager::getEE(int addr, int def) {
  int val = def;
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, val);
  EEPROM.commit();
  EEPROM.end();
  if (val == 255) return def;
  return val;
}

PasswordObject EEPROMManager::readEEPROMObject(int addr) {
  PasswordObject customVar; //Variable to store custom object read from EEPROM.
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.get(addr, customVar);
  EEPROM.commit();
  EEPROM.end();

  return customVar;
}

void EEPROMManager::clearEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
  // write a 0 to all 512 bytes of the EEPROM
  for (int i = 0; i < EEPROM_SIZE; i++) {
    EEPROM.write(i, 255);
  }

  // turn the LED on when we're done
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);
  EEPROM.end();
}

void EEPROMManager::clearEEPROMAddr(int addr) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(addr, EMPTY_EEPROM_CELL);
  EEPROM.commit();
  EEPROM.end();
}

void EEPROMManager::writeEEPROMObject(int addr, PasswordObject customVar) {
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.put(addr, customVar);
  EEPROM.commit();
  EEPROM.end();
}
