#include <Arduino.h>
#include <Wire.h>

#define EXPANDER_ADDRESS 32

#define MCP23017_IODIRA 0x00
#define MCP23017_IODIRB 0x01
#define MCP23017_GPIOA  0x12
#define MCP23017_GPIOB  0x13
#define MCP23017_GPPUA  0x0C
#define MCP23017_GPPUB  0x0D
#define MCP23017_OLATA  0x14
#define MCP23017_OLATB  0x15
#define MCP23008_GPIOA  0x09
#define MCP23008_GPPUA  0x06
#define MCP23008_OLATA  0x0A

#define LED 2

void xpSend(uint8_t port, uint8_t val) {
  Wire.beginTransmission(EXPANDER_ADDRESS);
  Wire.write(port);         // IODIR_A
  Wire.write(val);         // All outputs
  Wire.endTransmission(true);
}


void setup() {
  pinMode(LED, OUTPUT);

  Wire.begin(21, 22);

  xpSend(MCP23017_IODIRA, 0x00);  // Port A all outputs
  xpSend(MCP23017_IODIRB, 0x00);  // Port B all outputs
  xpSend(MCP23017_OLATA, 0x00);


  // put your setup code here, to run once:
}


int count = 0;
void loop() {
  // delay(200);
  // digitalWrite(LED, HIGH);
  // delay(200);
  // digitalWrite(LED, LOW);

  xpSend(MCP23017_OLATA, count);
  xpSend(MCP23017_OLATB, 1);
  delay(2);
  count++;
  if(count >= 256)
    count = 0;
  xpSend(MCP23017_OLATB, 0);
  delay(2);
}