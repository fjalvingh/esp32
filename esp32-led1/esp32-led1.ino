#include <FastLED.h>

#define LED_PIN     27
#define COLOR_ORDER BRG
#define CHIPSET     WS2812
#define NUM_LEDS    256

#define BRIGHTNESS  200
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

void setup() {
  delay(3000);
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS); //.setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
}

void setRowLeds(int row, CRGB color) {
  int ledix = 8*row;
  for(int i = 0; i < 8; i++) {
    leds[ledix++] = color;
  }
  FastLED.show();
}

void loop() {
  // In steps of 8 leds, cycle through all colors
  int row = 0;
  int maxrow = NUM_LEDS / 8;
  while(row < maxrow) {
    setRowLeds(row, CRGB::Red);
    delay(250);
    setRowLeds(row, CRGB::Green);
    delay(250);
    setRowLeds(row, CRGB::Blue);
    delay(250);
    setRowLeds(row, CRGB::Black);
    row++;
  }
}
