#include <Arduino.h>
#include <NeoHWSerial.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#define PIN PORTD2
#define RXBUFSZ   32

Adafruit_NeoPixel strip = Adafruit_NeoPixel(64, PIN, NEO_GRB + NEO_KHZ800);

int gDel = 100;
volatile int gLc = 0;
int gCount = 0;

uint8_t gRxBuf[RXBUFSZ];
volatile int gPut = 0;
volatile int gGet = 0;
volatile int gSz = 0;


static int rxGet() {
  if(gSz == 0)
    return -1;
  int ix = gGet++;
  if(ix >= RXBUFSZ)
    gGet = 0;
  int res = gRxBuf[ix] & 0xff;
  gSz--;
  return res;
}

static void handleRxChar(uint8_t c) {
  if(gSz >= RXBUFSZ)
    return;
  gRxBuf[gPut++] = c;
  gSz++;
  if(gPut >= RXBUFSZ)
    gPut = 0;
}

void setup() {
    // put your setup code here, to run once:
    pinMode(LED_BUILTIN, OUTPUT);
    NeoSerial.attachInterrupt(handleRxChar);
    NeoSerial.begin( 9600 );                 // Instead of 'Serial1'
    strip.begin();
    strip.setBrightness(25);
    // Serial.begin(9600);
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel2(byte WheelPos, byte hell) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, hell, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(hell, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip.Color(WheelPos * 3, 255 - WheelPos * 3, hell);
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel2(((i * 256 / strip.numPixels()) + j) & 255, 0));
    }
    strip.show();
    delay(wait);
  }
}

void loop() {
    // put your main code here, to run repeatedly:
    // if(Serial.available()) {
    //   gLc = Serial.read();
    // }
    rainbowCycle(0);
    NeoSerial.print(gCount++);
    NeoSerial.write(" hello ");
    for(;;) {
      int c = rxGet();
      if(c == -1)
        break;
      NeoSerial.print((char) c);
    }
    NeoSerial.print("\r\n");
}
