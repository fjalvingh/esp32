#include <Arduino.h>
#include <Wire.h>
#include "ym.h"

/*
 * Info links:
 * https://github.com/electrified/rc2014-ym2151/blob/main/software/test_programs/beep.bas
 * https://cx5m.file-hunter.com/fmunit.htm
 * 
 */

#define EXPANDER_ADDRESS 32
#define LED 2


Ym ym(21, 22, EXPANDER_ADDRESS);

void d(char* s) {
  Serial.println(s);
}

byte voice0[] = {
  0x20, 0xc0,
  0x58, 0x01,
  0x98, 0x1f,
  0xb8, 0x0d,
  0xf8, 0xf6
};

void writeList(byte* list, int count) {
  while(count-- > 0) {
    ym.writeReg(list[0], list[1]);
    list += 2;
  }
}

void setup() {
  Serial.begin(9600);
  ym.begin();
  // Wire.begin(21, 22);

  pinMode(LED, OUTPUT);

  // xpSend(MCP23017_IODIRA, 0x00);  // Port A all outputs
  // xpSend(MCP23017_IODIRB, 0x00);  // Port B all outputs
  // xpSend(MCP23017_OLATA, 0x00);


  // put your setup code here, to run once:
  writeList(voice0, sizeof(voice0) / 2);
}

int count = 0;
void loop() {
  // delay(200);
  // digitalWrite(LED, HIGH);
  // delay(200);
  // digitalWrite(LED, LOW);

  if(count & 0x1) {
    ym.led(true);
    // xpSend(MCP23017_OLATA, 0xff);
    digitalWrite(LED, 0);
  } else {
    ym.led(false);
    // xpSend(MCP23017_OLATA, 0x00);
    digitalWrite(LED, 1);
  }

  ym.writeReg(0x28, 0x3a);       // Write low note freq to voice 0 freq reg
  ym.writeReg(0x8, 0);           // Keyup/down register -> release previous note
  ym.writeReg(0x8, 0x40);        // Play note
  delay(200);

  ym.writeReg(0x8, 0);           // Keyup/down register -> release previous note
  delay(200);

  ym.writeReg(0x28, 0x44);       // High note to voice 0 freq reg
  ym.writeReg(0x8, 0);
  ym.writeReg(0x8, 0x40);        // Play note
  
  delay(2000);
  count++;
}