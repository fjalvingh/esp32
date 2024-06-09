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

/*
 * Synth board: the B port is connected to the data lines.
 * The A port is connected to the other lines, as follows:
 */
#define GPA_IRQ         0
#define GPA_IC          1       // When 0 the chip gets reset.
#define GPA_A0          2       // When 0 the data on the data bus is an address, when 1 it is data
#define GPA_WR          3       // When 0 the data gets latched
#define GPA_RD          4       // When 0 the data can be read
#define GPA_LED         7

#define bit(x)    (1 << x)
#define nbit(x)   ( ~bit(x) )

byte xpLedState = 0xff;
byte xpControlState = 0xff;

void d(char* s) {
  Serial.println(s);
}

void rwait() {
  for(int i = 0; i < 10; i++)
    NOP();
}

/**
 * Sends a command to the MCP23017.
 */
void xpSend(uint8_t port, uint8_t val) {
  Wire.beginTransmission(EXPANDER_ADDRESS);
  Wire.write(port);         // IODIR_A
  Wire.write(val);         // All outputs
  Wire.endTransmission(true);
}

/**
 * Read either port A (0) or PortB (1)
 */
int xpRead(uint8_t port) {
  Wire.beginTransmission(EXPANDER_ADDRESS);
  Wire.write(port == 0 ? MCP23017_GPIOA : MCP23017_GPIOB);
  Wire.endTransmission(true);
  Wire.requestFrom(EXPANDER_ADDRESS, 1);    // One byte to read
  return Wire.read() & 0xff;
}

/**
 * Initialize the expander.
 */
void xpInitialize() {
  xpSend(MCP23017_IODIRA, (1 << GPA_IRQ));    // Only IRQ is input, rest is output
  xpSend(MCP23017_IODIRB, 0xff);              // Initialize b as all input
  xpSend(MCP23017_OLATA, 0xFF);               // A all ports high, as most lines are active-low
}

void xpLed(boolean on) {
  byte led = on ? 0x80 : 0x00;
  xpLedState = led;
  xpSend(MCP23017_OLATA, led | xpControlState);
}

int ymReadStatus() {
  xpSend(MCP23017_IODIRB, 0xff);              // All lines READ
  return xpRead(1);                           // Read from the data signals
}

void ymWaitBusy() {
  int val = ymReadStatus();
  if(0 == (val & 0x80)) {
    return;
  }
  d("ym is busy");

  for(;;) {
    val = ymReadStatus();
    if(0 == (val & 0x80)) {
      return;
    }
    rwait();
  }
}

/**
 * Send a byte to the YM control lines (GPIO A).
 */
void ymControl(byte val) {
  xpControlState  = val;
  xpSend(MCP23017_GPIOA, (val & 0x7f) | (xpLedState & 0x80));
}

/**
 * Send a data byte to B (the ym D lines).
 */
void ymData(byte val) {
  xpSend(MCP23017_IODIRB, 0x00);        // All outputs
  xpSend(MCP23017_GPIOB, val);
}

/**
 * Reset the ym. Just keep IC low for a bit, the rest high.
 */
void ymReset() {
  ymControl(~ (1 << GPA_IC));
  rwait();
  ymControl(0xff);
}

/**
 * Write a byte to an YM register.
 */
void ymWriteReg(int reg, byte data) {
  ymWaitBusy();

  //-- Write the address
  ymData(reg);
  ymControl(nbit(GPA_A0) & nbit(GPA_WR));       // A0=0, RD=0
  rwait();
  ymControl(0xff);
  rwait();
  ymData(data);
  ymControl(nbit(GPA_WR));
  rwait();
  ymControl(0xff);
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
    ymWriteReg(list[0], list[1]);
    list += 2;
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin(21, 22);

  xpInitialize();
  ymReset();
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
    xpLed(true);
    // xpSend(MCP23017_OLATA, 0xff);
    digitalWrite(LED, 0);
  } else {
    xpLed(false);
    // xpSend(MCP23017_OLATA, 0x00);
    digitalWrite(LED, 1);
  }

  ymWriteReg(0x28, 0x3a);       // Write low note freq to voice 0 freq reg
  ymWriteReg(0x8, 0);           // Keyup/down register -> release previous note
  ymWriteReg(0x8, 0x40);        // Play note
  delay(200);

  ymWriteReg(0x8, 0);           // Keyup/down register -> release previous note
  delay(200);

  ymWriteReg(0x28, 0x44);       // High note to voice 0 freq reg
  ymWriteReg(0x8, 0);
  ymWriteReg(0x8, 0x40);        // Play note
  
  delay(2000);
  count++;
}