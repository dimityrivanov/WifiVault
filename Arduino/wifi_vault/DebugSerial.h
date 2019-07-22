#ifndef DebugSerial_h
#define DebugSerial_h

#include "Arduino.h"
class DebugSerial {
  public:
    void printToSerial(const char* text);
  private:
    const bool DEBUG = false;
};
#endif
