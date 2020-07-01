#include <FastLED.h>
#include <Adafruit_GFX.h>

#include <Fonts/FreeSerif9pt7b.h>
//#include "pacman2_1.h"
//#include "pacman2_2.h"
//#include "pacman2_4.h"

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


struct FRAMESET {
  int width;
  int height;
  int framecount;
  CRGB color;
  const byte** frames;
};

struct FRAMESET2 {
  int width;
  int height;
  int framecount;
  const int** palette;
  const byte** frames;
};

struct ANIMITEM {
  FRAMESET2* frameset;
  int y;
  int relx;
};

//static const byte* pacman_framelist[] = { pacman1, pacman2, pacman1, pacman4 };
//
//static const FRAMESET pacman = {12, 13, 4, CRGB::Yellow, pacman_framelist };


/**
 * Ghosts: 2bit depth indexed bitmap.
 */
static const unsigned char ghost1_2_bm[] = {
0x0,0x14,0x0,0x1,0x55,0x40,0x5,0x55,0x50,0x6,0x96,0x90,0x1b,0xda,0xf4,0x1b
,0xda,0xf4,0x5a,0x56,0x95,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55
,0x55,0x55,0x55,0x55,0x15,0x15,0x14,0x14,0x14,0x14};
static const int ghost1_2_col[] = {0x0, 0xd95763, 0xffffff, 0x0};
static const unsigned char ghost1_1_bm[] = {
0x0,0x14,0x0,0x1,0x55,0x40,0x5,0x55,0x50,0x6,0x96,0x90,0x1b,0xda,0xf4,0x1b
,0xda,0xf4,0x5a,0x56,0x95,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55
,0x55,0x55,0x55,0x55,0x51,0x51,0x51,0x40,0x40,0x40};
static const int ghost1_1_col[] = {0x0, 0xd95763, 0xffffff, 0x0};
static const unsigned char ghost1_3_bm[] = {
0x0,0x14,0x0,0x1,0x55,0x40,0x5,0x55,0x50,0x6,0x96,0x90,0x1b,0xda,0xf4,0x1b
,0xda,0xf4,0x5a,0x56,0x95,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55
,0x55,0x55,0x55,0x55,0x55,0x15,0x15,0x14,0x4,0x5};
static const int ghost1_3_col[] = {0x0, 0xd95763, 0xffffff, 0x0};


static const unsigned char pacman2_4_bm[] = {
0x0,0x15,0x40,0x1,0x55,0x54,0x5,0x55,0x55,0x15,0x55,0x55,0x15,0x55,0x55,0x55
,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x55,0x15,0x55,0x55,0x15,0x55,0x55,0x5,0x55
,0x55,0x1,0x55,0x54,0x0,0x15,0x40};
static const int pacman2_4_col[] = {0x0, 0xfbf236};
static const unsigned char pacman2_1_bm[] = {
0x0,0x15,0x40,0x1,0x55,0x54,0x5,0x55,0x55,0x15,0x55,0x55,0x15,0x55,0x50,0x55
,0x54,0x0,0x55,0x0,0x0,0x55,0x54,0x0,0x15,0x55,0x50,0x15,0x55,0x55,0x5,0x55
,0x55,0x1,0x55,0x54,0x0,0x15,0x40};
static const int pacman2_1_col[] = {0x0, 0xfbf236};
static const unsigned char pacman2_2_bm[] = {
0x0,0x15,0x40,0x1,0x55,0x50,0x5,0x55,0x40,0x15,0x55,0x0,0x15,0x54,0x0,0x55
,0x50,0x0,0x55,0x40,0x0,0x55,0x50,0x0,0x15,0x54,0x0,0x15,0x55,0x0,0x5,0x55
,0x40,0x1,0x55,0x50,0x0,0x15,0x40};
static const int pacman2_2_col[] = {0x0, 0xfbf236};
static const unsigned char pacman2_3_bm[] = {
0x0,0x15,0x40,0x1,0x55,0x54,0x5,0x55,0x55,0x15,0x55,0x55,0x15,0x55,0x50,0x55
,0x54,0x0,0x55,0x0,0x0,0x55,0x54,0x0,0x15,0x55,0x50,0x15,0x55,0x55,0x5,0x55
,0x55,0x1,0x55,0x54,0x0,0x15,0x40};
static const int pacman2_3_col[] = {0x0, 0xfbf236};
static const unsigned char* pacman_frames[] = {pacman2_1_bm,pacman2_2_bm,pacman2_3_bm,pacman2_4_bm};
static const int* pacman_palette[] = {pacman2_1_col,pacman2_2_col,pacman2_3_col,pacman2_4_col};
static FRAMESET2 pacman = { 12, 13, 4, pacman_palette, pacman_frames };





static const int* ghost1_colset[] { ghost1_1_col, ghost1_2_col, ghost1_3_col };
static const byte* ghost1_frameset[] = { ghost1_1_bm, ghost1_2_bm, ghost1_3_bm };

static FRAMESET2 ghost1 = { 12, 14, 3, ghost1_colset, ghost1_frameset };

static const int ghost2_col[] = {0x0, 0x5b6ee1, 0xffffff, 0x0};
static const int* ghost2_colset[] { ghost2_col, ghost2_col, ghost2_col };
static FRAMESET2 ghost2 = { 12, 14, 3, ghost2_colset, ghost1_frameset };


int pacman_frame = 0;
int ghost_frame = 0;

static const ANIMITEM animList[] = {
  { &pacman, 0, 0 },                  // Start with pacman
  { &ghost1, 0, -40 },
  { &ghost2, 0, -54 }
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
  
  render81();
  FastLED.show();
  delay(5000);

  for(int x = -16; x < 17; x++) {
    renderPacFrame(x);
  }

  for(int x = -16; x < 17; x++) {
    renderGhostFrame(&ghost1, x);
  }
  for(int x = -16; x < 17; x++) {
    renderGhostFrame(&ghost2, x);
  }
  delay(10000);
}

void renderAnimationItemList(ANIMITEM* itemList, int itemCount) {
  //-- calculate how many x'es we need
  int minx = 999999, maxx = -999999;

  for(int i = 0; i < itemCount; i++) {
    ANIMITEM item = itemList[i];
    int sx = item->relx - item->frameset->width;
    if(sx < minx)
      minx = sx;
    int ex = item-relx;
    if(ex > maxx)
      maxx = ex;
  }

  
  
  
}

void renderPacFrame(int x) {
  memset(leds, 0, sizeof(leds));
  render81();
  render2Frame(&pacman, x, 0, pacman_frame);
  FastLED.show();
  delay(100);
}

void renderGhostFrame(FRAMESET2* ani, int x) {
  memset(leds, 0, sizeof(leds));
  render81();
  render2Frame(ani, x, 0, ghost_frame);
  FastLED.show();
  delay(50);
}

void render81() {
  const GFXfont* font = &FreeSerif9pt7b;
  int w = renderChar(font, 0, 0, '8');
  renderChar(font, w, 0, '1');
}


void setLed(int x, int y, CRGB color) {
  if(x < 0 || x >= 16 || y < 0 || y >= 16) 
    return;
  y = 15 - y;

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

void render2Bits(const byte* pattern, int patwidth, int patheight, int x, int y, const int* palette) {
  const byte* src = pattern;
  for(int py = 0; py < patheight; py++) {
    int bitsleft = 0;
    int byva = 0;
    for(int px = 0; px < patwidth; px++) {
      if(bitsleft <= 0) {
        byva = (*src++) & 0xff;
        bitsleft = 6;
      } else {
        bitsleft-= 2;
      }
      int index = (byva >> 6) & 0x3;
      if(index > 0) {
        int color = palette[index];
//        Serial.print("col ");
//        Serial.print("x="); Serial.print(px, DEC);
//        Serial.print(" x="); Serial.print(py, DEC);
//        Serial.print(" color="); Serial.print(color, HEX);
//        Serial.println();
        setLed(x + px, y + py, color);
      }
      byva = byva << 2;
    }
  }
}

void render2Frame(const FRAMESET2* set, int x, int y, int& currentFrame) {
  render2Bits(set->frames[currentFrame], set->width, set->height, x, y, set->palette[currentFrame]);
  if(++currentFrame >= set->framecount)
    currentFrame = 0;
}

void render1Bits(const byte* pattern, int patwidth, int patheight, int x, int y, CRGB color) {
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


void renderFrame(const FRAMESET* set, int x, int y, int& currentFrame) {
  render1Bits(set->frames[currentFrame++], set->width, set->height, x, y, set->color);
  if(currentFrame >= set->framecount)
    currentFrame = 0;
}

int renderChar(const GFXfont* fi, int xoff, int yoff, char c) {
  if(c < fi->first || c > fi->last)
    c = '?';
  
  int cix = c - fi->first;

  const GFXglyph gly = fi->glyph[cix];
  
  int cWidth = gly.width;
  int coff = gly.bitmapOffset;

//  char buf[80];
//  sprintf(buf, "off=%d, gw=%d, gh=%d", coff, gly.width, gly.height);
//  Serial.println(buf);

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
      setLed(xoff + cx, (yoff + cy), pel == 0 ? CRGB::Black : CRGB::White);
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
