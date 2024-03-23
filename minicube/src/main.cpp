#include "Arduino.h"

#define PIN8 8
#define PIN12 12
#define PIN11 11

#define CLOCKPIN PIN12
#define LATCHPIN PIN8
#define DATAPIN PIN11

#define L0      PIN4
#define L1      PIN5
#define L2      PIN6
#define L3      PIN7

static int LAYERPIN[] {
  L0, L1, L2, L3
};

static int layerData[4];

void setup() {
  pinMode(PIN8, OUTPUT);
  pinMode(PIN11, OUTPUT);
  pinMode(PIN12, OUTPUT);
  pinMode(PIN8, OUTPUT);
  pinMode(PIN4, OUTPUT);
  pinMode(PIN5, OUTPUT);
  pinMode(PIN6, OUTPUT);
  pinMode(PIN7, OUTPUT);

  digitalWrite(CLOCKPIN, HIGH);
  digitalWrite(LATCHPIN, LOW);
  digitalWrite(DATAPIN, HIGH);

  digitalWrite(L0, LOW);
  digitalWrite(L1, LOW);
  digitalWrite(L2, LOW);
  digitalWrite(L3, LOW);
}

/*
 * Send the specified 16 bits to the shift registers, then latch
 */
void sendToSR(uint16_t val) {
  digitalWrite(LATCHPIN, LOW);
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, (val >> 8));
  shiftOut(DATAPIN, CLOCKPIN, MSBFIRST, (val & 0xff));
  digitalWrite(LATCHPIN, HIGH);
}

void switchOn(int layer, int led) {
  for(int i = 0; i < 4; i++) {
    digitalWrite(LAYERPIN[i], LOW);
  }
  digitalWrite(LAYERPIN[layer], HIGH);

  sendToSR(1 << led);
}

/*
 * Show all layers.
 */
void showLayers() {
  int prevLayer = 3;
  for(int layer = 0; layer < 4; layer++) {
    //-- Switch on plane
    digitalWrite(LAYERPIN[prevLayer], LOW);   // Switch off previous plane
    digitalWrite(LAYERPIN[layer], HIGH);
    prevLayer = layer;

    sendToSR(layerData[layer]);
    delay(5);
  }
}

static int alterCount;
static int ledCount;

void setLed(int led, bool on) {
  int layer = led / 16;
  int bit = (led % 16);

  if(on) {
    layerData[layer] |= (1 << bit);
  } else {
    layerData[layer] &= ~ (1 << bit);
  }
}

void alterUI() {
  if(alterCount++ < 2)
    return;
  alterCount = 0;

  if(ledCount >= 128)
    ledCount = 0;

  if(ledCount < 64) {
    setLed(ledCount, true);
  } else if(ledCount < 128) {
    setLed(ledCount - 64, false);
  }
  ledCount++;
}

void loop() {
  for(;;) {
    alterUI();
    showLayers();
  }
}
