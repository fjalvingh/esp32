/*
 * https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-I2S-DMA
 */

/* ------------------------- CUSTOM GPIO PIN MAPPING ------------------------- */
#if 0
#define R1_PIN 18
#define G1_PIN 25
#define B1_PIN 5
#define R2_PIN 17
#define G2_PIN 26
#define B2_PIN 16
#define A_PIN 14
#define B_PIN 27 
#define C_PIN 12
#define D_PIN 4
#define E_PIN -1
#define LAT_PIN 13
#define OE_PIN 15
#define CLK_PIN 2

#else

#define R1_PIN  25
#define G1_PIN  26
#define B1_PIN  27
#define R2_PIN  14
#define G2_PIN  12
#define B2_PIN  13

#define A_PIN   23
#define B_PIN   19
#define C_PIN   5
#define D_PIN   18
#define E_PIN   -1	// required for 1/32 scan panels, like 64x64. Any available pin would do, i.e. IO32
          
#define LAT_PIN 4
#define OE_PIN  15

#define CLK_PIN 21

#endif

/* -------------------------- Display Config Initialisation -------------------- */

#define MATRIX_WIDTH 64
#define MATRIX_HEIGHT 32

/* -------------------------- Class Initialisation -------------------------- */
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
MatrixPanel_I2S_DMA matrix;

#include <FastLED.h>

#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Playlist.h"
//#include "Geometry.h"

#include "Patterns.h"
Patterns patterns;

/* -------------------------- Some variables -------------------------- */
unsigned long fps = 0, fps_timer; // fps (this is NOT a matix refresh rate!)
unsigned int default_fps = 30, pattern_fps = 30;  // default fps limit (this is not a matix refresh conuter!)
unsigned long ms_animation_max_duration = 20000;  // 20 seconds
unsigned long last_frame=0, ms_previous=0;


void listPatterns() {
  patterns.listPatterns();
}

void setup()
{
  // Setup serial interface
  Serial.begin(115200);
  delay(250);
  matrix.begin(R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN );  // setup the LED matrix
  /**
   * this demos runs pretty fine in fast-mode which gives much better fps on large matrixes (>128x64)
   * see comments in the lib header on what does that means
   */
  //dma_display.setFastMode(true);

  // SETS THE BRIGHTNESS HERE. MAX value is MATRIX_WIDTH, 2/3 OR LOWER IDEAL, default is about 50%
  // dma_display.setPanelBrightness(30);
  /* another way to change brightness is to use
   * dma_display.setPanelBrightness8(uint8_t brt);	// were brt is within range 0-255
   * it will recalculate to consider matrix width automatically
   */
  //dma_display.setPanelBrightness8(180);

  Serial.println("**************** Starting Aurora Effects Demo ****************");

   // setup the effects generator
  effects.Setup();

  delay(500);
  Serial.println("Effects being loaded: ");
  listPatterns();


  // patterns.moveRandom(1); // start from a random pattern

  Serial.print("Starting with pattern: ");
  Serial.println(patterns.getCurrentPatternName());
  patterns.start();
  ms_previous = millis();
  fps_timer = millis();
}

void loop()
{
    // menu.run(mainMenuItems, mainMenuItemCount);  

  if ( (millis() - ms_previous) > ms_animation_max_duration ) 
  {
      patterns.stop();
      //  patterns.moveRandom(1);
#ifndef TESTPATTERN
      patterns.move(1);
      Serial.print("Changing pattern to:  ");
      Serial.println(patterns.getCurrentPatternName());
#endif      
      patterns.start();  
      ms_previous = millis();

       // Select a random palette as well
       //effects.RandomPalette();
    }
 
    if ( 1000 / pattern_fps + last_frame < millis()){
      last_frame = millis();
      pattern_fps = patterns.drawFrame();
      if (!pattern_fps)
        pattern_fps = default_fps;

      ++fps;
    }

    if (fps_timer + 1000 < millis()){
       Serial.printf_P(PSTR("Effect fps: %ld\n"), fps);
       fps_timer = millis();
       fps = 0;
    }
       
}
