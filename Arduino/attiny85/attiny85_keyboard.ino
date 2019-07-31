/****************************************************************************************************************/
/* BluetoothKeyboard - Control the keyboard across a bluetooth connection                                       */
/****************************************************************************************************************/

#include <SoftSerial_INT0.h>
#include "DigiKeyboard.h"

#define P_RX 2                        // Reception PIN (SoftSerial)
#define P_TX 1                        // Transmition PIN (SoftSerial)
#define KEY_ENTER 40                  // Keyboard usage values (ENTER Key)
#define KEY_ESC 41                    // Keyboard usage values (ESCAPE Key)

SoftSerial BLE(P_RX, P_TX);           // Software serial port for control the BLE module
char inData[40];
byte index = 0; // Index into array; where to store the character
/****************************************************************************************************************/

void setup()
{
  BLE.begin(9600); // Initialize the serial port
}

void clearArray() {
  for (int index = 0; index < 40; index++) {
    inData[index] = '\0';
  }
  index = 0;
}

void printDataToKeyBoard() {
  DigiKeyboard.sendKeyStroke(0);
  DigiKeyboard.print(inData);
  DigiKeyboard.sendKeyStroke(0x2B);
  clearArray();
}

void loop()
{
  static char cmd; // Get Command variable

  //read chars from the serial
  // If there is any data incoming from the serial port
  while (BLE.available() > 0) {
    delay(1);  //delay to allow byte to arrive in input buffer
    cmd = BLE.read(); // Get command
    inData[index] = cmd; // Store it
    index++; // Increment where to write next
  }

  //call keyboard only if data is available
  if (index != 0) {
    printDataToKeyBoard();
  }

  DigiKeyboard.update(); // Update the USB connection (maintain alive the connection)
}

