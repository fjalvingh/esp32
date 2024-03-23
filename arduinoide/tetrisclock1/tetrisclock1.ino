/*******************************************************************
    Tetris clock that fetches its time Using the EzTimeLibrary

    For use with the ESP32 or TinyPICO
 *                                                                 *
    Written by Brian Lough
    YouTube: https://www.youtube.com/brianlough
    Tindie: https://www.tindie.com/stores/brianlough/
    Twitter: https://twitter.com/witnessmenow

 convert bitmaps to Adafruit 565 format:
 http://javl.github.io/image2cpp/
 
 *******************************************************************/

// ----------------------------
// Standard Libraries - Already Installed if you have ESP32 set up
// ----------------------------

#include <WiFi.h>
#include <HTTPClient.h>           // for weather

// ----------------------------
// Additional Libraries - each one of these will need to be installed.
// ----------------------------

// Enabling this is meant to have a performance
// improvement but its worse for me.
// https://github.com/2dom/PxMatrix/pull/103
//#define double_buffer

#include <FastLED.h>
#include <LEDMatrix.h>
#include <FastLED_NeoMatrix.h>

// The library for controlling the LED Matrix
// At time of writing this my changes for the TinyPICO
// Have been merged into the main PxMatrix library,
// but have not been released, so you will need to install
// from Github
//
// If you are using a regular ESP32 you may be able to use
// the library manager version
// https://github.com/2dom/PxMatrix

// Adafruit GFX library is a dependancy for the PxMatrix Library
// Can be installed from the library manager
// https://github.com/adafruit/Adafruit-GFX-Library

#include <TetrisMatrixDraw.h>
// This library draws out characters using a tetris block
// amimation
// Can be installed from the library manager
// https://github.com/toblum/TetrisAnimation

#include <ezTime.h>
// Library used for getting the time and adjusting for DST
// Search for "ezTime" in the Arduino Library manager
// https://github.com/ropg/ezTime

// ---- Stuff to configure ----

// Initialize Wifi connection to the router
char ssid[] = "trocadero-2";     // your network SSID (name)
char password[] = "sl@shd0t"; // your network key

// Set a timezone using the following list
// https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
#define MYTIMEZONE "Europe/Amsterdam"

// Sets whether the clock should be 12 hour format or not.
bool twelveHourFormat = false;

// If this is set to false, the number will only change if the value behind it changes
// e.g. the digit representing the least significant minute will be replaced every minute,
// but the most significant number will only be replaced every 10 minutes.
// When true, all digits will be replaced every minute.
bool forceRefresh = true;
// -----------------------------

#define LED_PIN        2
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  32
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> ledmatrix;
CRGB *leds = ledmatrix[0];

FastLED_NeoMatrix& display= *new FastLED_NeoMatrix(leds, MATRIX_WIDTH, MATRIX_HEIGHT,
1, 1,
  NEO_MATRIX_BOTTOM     + NEO_MATRIX_RIGHT +
    NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

void matrix_clear() {
  FastLED.clear();
    // FastLED.clear does not work properly with multiple matrices connected via parallel inputs
    // on ESP8266 (not sure about other chips).
//    memset(leds, 0, NUMMATRIX*3);
}

#include "sunnyclouds.h"


portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
hw_timer_t * timer = NULL;
hw_timer_t * animationTimer = NULL;

// PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
// PxMATRIX display(64,32,P_LAT, P_OE,P_A,P_B,P_C,P_D);
//PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D, P_E);

TetrisMatrixDraw tetris(display); // Main clock
TetrisMatrixDraw tetris2(display); // The "M" of AM/PM
TetrisMatrixDraw tetris3(display); // The "P" or "A" of AM/PM

Timezone myTZ;
unsigned long oneSecondLoopDue = 0;

bool showColon = true;
volatile bool finishedAnimating = false;
bool displayIntro = true;

String lastDisplayedTime = "";
String lastDisplayedAmPm = "";

// This method is needed for driving the display
void IRAM_ATTR display_updater() {
//  portENTER_CRITICAL_ISR(&timerMux);
//     FastLED.show();
//  display.display(10);
//  portEXIT_CRITICAL_ISR(&timerMux);
}

// This method is for controlling the tetris library draw calls
void animationHandler()
{
#ifndef double_buffer
//  portENTER_CRITICAL_ISR(&timerMux);
#endif

  // Not clearing the display and redrawing it when you
  // dont need to improves how the refresh rate appears
  if (!finishedAnimating) {
#ifdef double_buffer
    display.fillScreen(tetris.tetrisBLACK);
#else
    matrix_clear();
//    display.clearDisplay();
#endif
    //display.fillScreen(tetris.tetrisBLACK);
    if (displayIntro) {
      finishedAnimating = tetris.drawText(1, 21);
    } else {
      if (twelveHourFormat) {
        // Place holders for checking are any of the tetris objects
        // currently still animating.
        bool tetris1Done = false;
        bool tetris2Done = false;
        bool tetris3Done = false;

        tetris1Done = tetris.drawNumbers(-6, 26, showColon);
        tetris2Done = tetris2.drawText(56, 25);

        // Only draw the top letter once the bottom letter is finished.
        if (tetris2Done) {
          tetris3Done = tetris3.drawText(56, 15);
        }

        finishedAnimating = tetris1Done && tetris2Done && tetris3Done;

      } else {
        finishedAnimating = tetris.drawNumbers(1, 26, showColon);
      }
    }
    FastLED.show();
  }
#ifndef double_buffer
//  portEXIT_CRITICAL_ISR(&timerMux);
#endif
}

void drawIntro(int x = 0, int y = 0)
{
  tetris.drawChar("P", x, y, tetris.tetrisCYAN);
  tetris.drawChar("o", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("w", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("e", x + 17, y, tetris.tetrisGREEN);
  tetris.drawChar("r", x + 22, y, tetris.tetrisBLUE);
  tetris.drawChar("e", x + 27, y, tetris.tetrisRED);
  tetris.drawChar("d", x + 32, y, tetris.tetrisWHITE);
  tetris.drawChar(" ", x + 37, y, tetris.tetrisMAGENTA);
  tetris.drawChar("b", x + 42, y, tetris.tetrisYELLOW);
  tetris.drawChar("y", x + 47, y, tetris.tetrisGREEN);
  FastLED.show();
}

void drawConnecting(int x = 0, int y = 0)
{
  tetris.drawChar("W", x, y, tetris.tetrisCYAN);
  tetris.drawChar("a", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("i", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("t", x + 17, y, tetris.tetrisGREEN);

  y += 9;
  tetris.drawChar("C", x, y, tetris.tetrisCYAN);
  tetris.drawChar("o", x + 5, y, tetris.tetrisMAGENTA);
  tetris.drawChar("n", x + 11, y, tetris.tetrisYELLOW);
  tetris.drawChar("n", x + 17, y, tetris.tetrisGREEN);
//  tetris.drawChar("e", x + 22, y, tetris.tetrisBLUE);
//  tetris.drawChar("c", x + 27, y, tetris.tetrisRED);
//  tetris.drawChar("t", x + 32, y, tetris.tetrisWHITE);
//  tetris.drawChar("i", x + 37, y, tetris.tetrisMAGENTA);
//  tetris.drawChar("n", x + 42, y, tetris.tetrisYELLOW);
//  tetris.drawChar("g", x + 47, y, tetris.tetrisGREEN);
  FastLED.show();
}

void setup() {
  disableCore0WDT();
  disableCore1WDT();
  Serial.begin(115200);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  // Set WiFi to station mode and disconnect from an AP if it was Previously
  // connected
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Do not set up display before WiFi connection
  // as it will crash!

  // Intialise display library
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(ledmatrix[0], ledmatrix.Size());
  FastLED.clear(true);
  FastLED.setBrightness(64);
  FastLED.show();

  //--
  display.drawRGBBitmap(0, 0, w01d, 32, 32);
  FastLED.show();
  delay(2000);

  
  //display.begin(16); // Generic ESP32 including Huzzah
//  FastLED.show();
//  display.flushDisplay();

  // Setup timer for driving display
//  timer = timerBegin(0, 80, true);
//  timerAttachInterrupt(timer, &display_updater, true);
//  timerAlarmWrite(timer, 2000, true);
//  timerAlarmEnable(timer);
//  yield();
#ifdef double_buffer
  display.fillScreen(tetris.tetrisBLACK);
#else
  matrix_clear();
//  display.clearDisplay();
#endif

  // "connecting"
  drawConnecting(0, 10);
#ifdef double_buffer

//  display.showBuffer();
#endif

  // Setup EZ Time
  setDebug(INFO);
  waitForSync();

  Serial.println();
  Serial.println("UTC:             " + UTC.dateTime());

  myTZ.setLocation(F(MYTIMEZONE));
  Serial.print(F("Time in your set timezone:         "));
  Serial.println(myTZ.dateTime());

#ifdef double_buffer
  display.fillScreen(tetris.tetrisBLACK);
#else
  matrix_clear();
//  display.clearDisplay();
#endif
  // "Powered By"
//  drawIntro(6, 12);
#ifdef double_buffer
  display.showBuffer();
#endif
  delay(2000);

  // Start the Animation Timer
  tetris.setText("HELO");
//  animationTimer = timerBegin(1, 80, true);
//  timerAttachInterrupt(animationTimer, &animationHandler, true);
//  timerAlarmWrite(animationTimer, 100000, true);
//  timerAlarmEnable(animationTimer);

  // Wait for the animation to finish
  while (!finishedAnimating)
  {
    Serial.println("beforeAnimationHandler");
    animationHandler();
    Serial.println("afterAnimationHandler");
    FastLED.show();
    delay(10); //waiting for intro to finish
  }
  delay(2000);                                                            
  finishedAnimating = false;
  displayIntro = false;
  tetris.scale = 1;
}

void loadWeather() {
  HTTPClient http;
 
  http.begin("http://api.openweathermap.org/data/2.5/weather?q=Lelystad,nl&appid=2c4b5287dddf6ee9893850f6eb2d8116"); //Specify the URL
  int httpCode = http.GET();  //Make the request

  if(httpCode > 0) { //Check for the returning code
    String payload = http.getString();
    Serial.println(httpCode);
    Serial.println(payload);
  } else {
    Serial.println("Error on HTTP request");
  }

  http.end(); //Free the resources
}


void setMatrixTime() {
  String timeString = "";
  String AmPmString = "";
  if (twelveHourFormat) {
    // Get the time in format "1:15" or 11:15 (12 hour, no leading 0)
    // Check the EZTime Github page for info on
    // time formatting
    timeString = myTZ.dateTime("g:i");

    //If the length is only 4, pad it with
    // a space at the beginning
    if (timeString.length() == 4) {
      timeString = " " + timeString;
    }

    //Get if its "AM" or "PM"
    AmPmString = myTZ.dateTime("A");
    if (lastDisplayedAmPm != AmPmString) {
      Serial.println(AmPmString);
      lastDisplayedAmPm = AmPmString;
      // Second character is always "M"
      // so need to parse it out
      tetris2.setText("M", forceRefresh);

      // Parse out first letter of String
      tetris3.setText(AmPmString.substring(0, 1), forceRefresh);
    }
  } else {
    // Get time in format "01:15" or "22:15"(24 hour with leading 0)
    timeString = myTZ.dateTime("H:i");
  }

  // Only update Time if its different
  if (lastDisplayedTime != timeString) {
    Serial.println(timeString);
    lastDisplayedTime = timeString;
    tetris.setTime(timeString, forceRefresh);

    // Must set this to false so animation knows
    // to start again
    finishedAnimating = false;
  }
}

void handleColonAfterAnimation() {

  // It will draw the colon every time, but when the colour is black it
  // should look like its clearing it.
  uint16_t colour =  showColon ? tetris.tetrisWHITE : tetris.tetrisBLACK;
  // The x position that you draw the tetris animation object
  int x = twelveHourFormat ? -6 : 1;
  // The y position adjusted for where the blocks will fall from
  // (this could be better!)
  int y = 26 - (TETRIS_Y_DROP_DEFAULT * tetris.scale);
  tetris.drawColon(x, y, colour);
}

unsigned long weatherUpdateDue = 0L;

void loop() {
  
  unsigned long now = millis();
  if (now > oneSecondLoopDue) {
    // We can call this often, but it will only
    // update when it needs to
    setMatrixTime();
    showColon = !showColon;

    // To reduce flicker on the screen we stop clearing the screen
    // when the animation is finished, but we still need the colon to
    // to blink
    if (finishedAnimating) {
      handleColonAfterAnimation();
    }
    oneSecondLoopDue = now + 1000;
  }
  if(now >= weatherUpdateDue) {
    loadWeather();
    weatherUpdateDue = now + 60000 * 5;
  }
  animationHandler();
  FastLED.show();
  delay(100);
//  Serial.println("Loop");
}
