#include "DebugSerial.h"

void DebugSerial::printToSerial(const char* text) {
  if (DEBUG) {
    Serial.println(text);
  }
}
