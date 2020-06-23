#include <FastLED.h>
#include <Adafruit_GFX.h>

#include <Fonts/FreeSerif9pt7b.h>
#include "pacman2_1.h"
#include "pacman2_2.h"
#include "pacman2_4.h"

#define LED_PIN     27
#define COLOR_ORDER BRG
#define CHIPSET     WS2812
#define NUM_LEDS    256

#define BRIGHTNESS  40
#define FRAMES_PER_SECOND 60

bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

struct FONT_INFO {
  int charHeight;
  int startChar;
  int endChar;
  int spacePixelWidth;
  const void* descriptors;
  const byte* bitmaps;
};




void setup() {
  delay(3000);

  Serial.begin(115200);
  
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
//  const GFXfont* font = &FreeSerif9pt7b;
//  int w = renderChar(font, 0, 0, '8');
//  renderChar(font, w, 0, '1');

  render1Bits(pacman1, 12, 13, 0, 0, CRGB::Yellow);
  FastLED.show();

  memset(leds, 0, sizeof(leds));
  delay(200);
  render1Bits(pacman2, 12, 13, 0, 0, CRGB::Yellow);
  FastLED.show();
  
  memset(leds, 0, sizeof(leds));
  delay(200);
  render1Bits(pacman1, 12, 13, 0, 0, CRGB::Yellow);
  FastLED.show();

  memset(leds, 0, sizeof(leds));
  delay(200);
  render1Bits(pacman4, 12, 13, 0, 0, CRGB::Yellow);
  FastLED.show();

  memset(leds, 0, sizeof(leds));
  delay(200);
}

void setLed(int x, int y, CRGB color) {
  if(x < 0 || x >= 16 || y < 0 || y >= 16) 
    return;

  int panelRowIndex = x % 8;
  int xPanelSelect = ((~x >> 3) & 0x1);          // 0 = panel 1, 1 - panel 0

  int apx = (7 - panelRowIndex) + xPanelSelect * 64;
  if(y >= 8) {
    y -= 8;
    apx += 128;
  }

  apx += y * 8;
  if(apx < 0 || apx > NUM_LEDS) {
    Serial.println("offset error");
    return;
  }

  leds[apx] = color;
}

int render1Bits(const byte* pattern, int patwidth, int patheight, int x, int y, CRGB color) {
  const byte* src = pattern;
  for(int py = 0; py < patheight; py++) {
    int bitsleft = 0;
    int byva = 0;
    for(int px = 0; px < patwidth; px++) {
      if(bitsleft <= 0) {
        byva = (*src++) & 0xff;
        bitsleft = 7;
      } else {
        bitsleft--;
      }
      if(byva & 0x80) {
        setLed(x + px, y + py, color);
      }
      byva = byva << 1;
    }
  }
}


int renderChar(const GFXfont* fi, int xoff, int yoff, char c) {
  if(c < fi->first || c > fi->last)
    c = '?';
  
  int cix = c - fi->first;

  const GFXglyph gly = fi->glyph[cix];
  
  int cWidth = gly.width;
  int coff = gly.bitmapOffset;

  char buf[80];
  sprintf(buf, "off=%d, gw=%d, gh=%d", coff, gly.width, gly.height);
  Serial.println(buf);

  const byte* cptr = fi->bitmap + coff;
  int bl = 0;
  for(int cy = 0; cy < gly.height; cy++) {
    
    byte data;
    for(int cx = 0; cx < gly.width; cx++) {
      if(bl <= 0) {
        bl = 8;
        data = pgm_read_byte_near(cptr++);
      }
      bl--;
      int pel = data & 0x80;
      data = data << 1;
      setLed(xoff + cx, 15 - (yoff + cy), pel == 0 ? CRGB::Black : CRGB::White);
    }
  }

  return xoff + cWidth + 3;
}

/*
 * Getting bitmaps - use the Pixelorama editor to create the animations
 * Save the animation as separate png files
 * Each PNG file can now be converted using ImageMagick:
 * 
 * convert pacman2_1.png -define h:format=gray -depth 1 pacman2_1.h
 * 
 * The depth:1 means we will get 1 bit per pixel; format=gray makes it grayscale. Other options are rgb
 * and argb, with specific depts = pits per color per pixel.
 */
