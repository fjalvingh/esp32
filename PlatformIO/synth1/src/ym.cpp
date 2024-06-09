
#include <Arduino.h>
#include <Wire.h>
#include "ym.h"

/*
 * Info links:
 * https://github.com/electrified/rc2014-ym2151/blob/main/software/test_programs/beep.bas
 * https://cx5m.file-hunter.com/fmunit.htm
 * Also, see halfway on this page for a reasonably good explanation of the registers,
 * at least a lot better than the crap from Yamaha itself: https://github.com/X16Community/x16-docs/blob/master/X16%20Reference%20-%2011%20-%20Sound%20Programming.md
 * 
 * 
 * The ym2151 is an 8 channel, 4 operator sound chip.
 * 
 */

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


/**
 * Get a register address
 */
uint8_t getAddr(uint8_t channel, uint8_t op) {
    return op * 8 + channel;
}

void checkOpAndChannel(uint8_t channel, uint8_t op) {
    if(channel >= 8)
        throw "Bad channel";
    if(op >= 4)
        throw "Bad op";
}

Ym::Ym(int sda, int scl, int expander) {
    m_expander = expander;
    m_sda = sda;
    m_scl = scl;
}

void Ym::begin() {
    Wire.begin(m_sda, m_scl);
    xpInitialize();                     // Initialize the MCP23017
    ymReset();                          // Reset the YM2151

    //-- Set all channels to use operator C2
    for(int i = 0; i < 8; i++)
        m_operatorOnOffState[i] = 0x40; // Default C2 to ON

}

void Ym::rwait() {
  for(int i = 0; i < 10; i++)
    NOP();
}

/**
 * Sends a command to the MCP23017.
 */
void Ym::xpSend(uint8_t port, uint8_t val) {
  Wire.beginTransmission(m_expander);
  Wire.write(port);         // IODIR_A
  Wire.write(val);         // All outputs
  Wire.endTransmission(true);
}

/**
 * Read either port A (0) or PortB (1)
 */
int Ym::xpRead(uint8_t port) {
  Wire.beginTransmission(m_expander);
  Wire.write(port == 0 ? MCP23017_GPIOA : MCP23017_GPIOB);
  Wire.endTransmission(true);
  Wire.requestFrom(m_expander, 1);    // One byte to read
  return Wire.read() & 0xff;
}

/**
 * Initialize the expander.
 */
void Ym::xpInitialize() {
  xpSend(MCP23017_IODIRA, (1 << GPA_IRQ));    // Only IRQ is input, rest is output
  xpSend(MCP23017_IODIRB, 0xff);              // Initialize b as all input
  xpSend(MCP23017_OLATA, 0xFF);               // A all ports high, as most lines are active-low
}

/**
 * Switch the on-board LED on or off.
 */
void Ym::led(boolean on) {
  byte led = on ? 0x80 : 0x00;
  xpLedState = led;
  xpSend(MCP23017_OLATA, led | xpControlState);
}

int Ym::ymReadStatus() {
  xpSend(MCP23017_IODIRB, 0xff);              // All lines READ
  return xpRead(1);                           // Read from the data signals
}

/**
 * Wait until the BUSY flag in the status register clears.
 */
void Ym::ymWaitBusy() {
  int val = ymReadStatus();
  if(0 == (val & 0x80)) {
    return;
  }

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
void Ym::ymControl(byte val) {
  xpControlState  = val;
  xpSend(MCP23017_GPIOA, (val & 0x7f) | (xpLedState & 0x80));
}

/**
 * Send a data byte to B (the ym D lines).
 */
void Ym::ymData(byte val) {
  xpSend(MCP23017_IODIRB, 0x00);        // All outputs
  xpSend(MCP23017_GPIOB, val);
}

/**
 * Reset the ym. Just keep IC low for a bit, the rest high.
 */
void Ym::ymReset() {
  ymControl(~ (1 << GPA_IC));
  rwait();
  ymControl(0xff);
}

/**
 * Write a byte to an YM register.
 */
void Ym::writeReg(uint8_t reg, byte data) {
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
  m_regState[reg] = data;
}

void Ym::writeRegM(uint8_t reg, uint8_t value, uint8_t mask) {
    byte val = (m_regState[reg] & ~mask) | (value & mask);
    writeReg(reg, val);
}


//-------- Global -------------
void Ym::lfo(boolean on) {
    writeRegM(0x01, on ? 0x00 : 0x02, 0x02); 
}

void Ym::operatorOn(uint8_t channel, uint8_t op, boolean on) {
    checkOpAndChannel(channel, op);

    //-- Get operators in same order as for all other calls
   	if(op == 1) {
		op = 2;
	} else if(op == 2) {
		op = 1;
	}

    //-- Set or reset the appropriate bit
    op += 3;                                    // Appropriate bit in register 8
	if(on) {
		m_operatorOnOffState[channel] = m_operatorOnOffState[channel] | (1 << op);
	} else {
		m_operatorOnOffState[channel] = m_operatorOnOffState[channel] & (~(1 << op));
	}
}

void Ym::noteOn(uint8_t channel) {
    uint8_t val = (channel & 0x7) 
        | m_operatorOnOffState[channel]
        ;
    writeReg(0x8, val);
}

void Ym::noteOff(uint8_t channel) {
    uint8_t val = (channel & 0x7);                      // All operator bits off
    writeReg(0x8, val);
}

void Ym::note(uint8_t channel, boolean on) {
    if(on)
        noteOn(channel);
    else
        noteOff(channel);
}






