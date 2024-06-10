
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

void raise(char* msg) {
  throw *msg;
}

void checkOpAndChannel(uint8_t channel, uint8_t op) {
    if(channel >= 8)
        raise("Bad channel");
    if(op >= 4)
        raise("Bad op");
}

void checkChannel(uint8_t channel) {
  if(channel >= 8)
    raise("Bad channel");
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

void Ym::setLfoFreq(uint8_t freq) {
  writeRegM(0x18, 0xff, freq);
}

void Ym::setLfoWaveForm(uint8_t code) {
  if(code > 0x3)
    raise("LFO Waveform > 3");
  writeRegM(0x1b, code, 0x03);
}

void Ym::setLfoWaveForm(LfoWaveform form) {
  setLfoWaveForm((uint8_t) form);
}

void Ym::setLfoPhaseDepth(uint8_t depth) {
  if(depth > 127)
    raise("Depth > 127");
  writeReg(0x19, 0x80 | depth);
}

void Ym::setLfoAmplitudeDepth(uint8_t depth) {
  if(depth > 127)
    raise("Depth > 127");
  writeReg(0x19, depth);
}

void Ym::setNoise(boolean on, uint8_t freq) {
  if(freq > 0x1f)
    raise("Noise frequency > 31");
  byte val = (on ? 0x80 : 0x00) | freq;
  writeRegM(0x0f, val, 0xff);
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

PROGMEM const unsigned char KeyCodeTable[] = {
	0x00, 0x01, 0x02, 0x04, 0x05, 0x06, 0x08, 0x09,
	0x0a, 0x0c, 0x0d, 0x0e, 0x10, 0x11, 0x12, 0x14,
	0x15, 0x16, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e,
	0x20, 0x21, 0x22, 0x24, 0x25, 0x26, 0x28, 0x29,
	0x2a, 0x2c, 0x2d, 0x2e, 0x30, 0x31, 0x32, 0x34,
	0x35, 0x36, 0x38, 0x39, 0x3a, 0x3c, 0x3d, 0x3e,
	0x40, 0x41, 0x42, 0x44, 0x45, 0x46, 0x48, 0x49,
	0x4a, 0x4c, 0x4d, 0x4e, 0x50, 0x51, 0x52, 0x54,
	0x55, 0x56, 0x58, 0x59, 0x5a, 0x5c, 0x5d, 0x5e,
	0x60, 0x61, 0x62, 0x64, 0x65, 0x66, 0x68, 0x69,
	0x6a, 0x6c, 0x6d, 0x6e, 0x70, 0x71, 0x72, 0x74,
	0x75, 0x76, 0x78, 0x79, 0x7a, 0x7c, 0x7d, 0x7e,
};

void Ym::setTone(uint8_t channel, uint8_t keyCode, uint8_t keyFraction) {
  checkChannel(channel);
 	int16_t	kfOffset = (keyFraction & 0x3f);
	int16_t	noteOffset = keyCode + (keyFraction >> 6);
	if(noteOffset < 0) 
    noteOffset = 0;
	else if(noteOffset > 0xbf) 
    noteOffset = 0xbf;
  writeReg(0x30 + channel, kfOffset << 2);
  writeReg(0x28 + channel, pgm_read_byte_near(KeyCodeTable + noteOffset));
}


void Ym::setAlgorithm(uint8_t channel, uint8_t algo) {
  checkChannel(channel);
  writeRegM(0x20 + channel, algo, 0x07);
}

void Ym::setFeedback(uint8_t channel, uint8_t level) {
  checkChannel(channel);
  writeRegM(0x20 + channel, level << 3, 0x38);
}

void Ym::setOutputChannels(uint8_t channel, uint8_t lr) {
  checkChannel(channel);
  if(lr > 3)
    raise("Left-right channel must be 2 bits only");
  writeRegM(0x20 + channel, lr << 6, 0xc0);
}

void Ym::setPhaseModulationSensitivity(uint8_t channel, uint8_t level) {
  checkChannel(channel);
  if(level > 7)
    raise("phase modulation sensitivity > 7");
  writeRegM(0x38 + channel, level << 4, 0x70);
}

void Ym::setAmplitudeModulationSensitivity(uint8_t channel, uint8_t level) {
  checkChannel(channel);
  if(level > 3)
    raise("amplitude modulation sensitivity > 3");
  writeRegM(0x38 + channel, level, 0x03);
}






