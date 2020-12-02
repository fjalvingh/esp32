
// TETRIS
//#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <LEDMatrix.h>
#include <LEDSprites.h>
#include <LEDText.h>
#include <FontMatrise.h>
#include "BluetoothSerial.h"

#define LED_PIN        2
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B
#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  32
#define MATRIX_TYPE    HORIZONTAL_ZIGZAG_MATRIX

#define APP_MENU -1
#define APP_TETRIS 0
#define APP_SNAKE 1
#define APP_BREAKOUT 2
#define APP_PIXEL 3
#define APP_ALLWHITE 4

int currentApp = APP_PIXEL;

cLEDMatrix<MATRIX_WIDTH, -MATRIX_HEIGHT, MATRIX_TYPE> leds;
BluetoothSerial SerialBT;

// Bluetooth input
enum btnInput {NONE, UP, DOWN, LEFT, RIGHT};
btnInput currentInput = NONE;

#include "Tetris.h"
#include "Menu.h"
#include "Snake.h"
#include "Breakout.h"
#include "Pixel.h"
#include "allwhite.h"

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting");
  SerialBT.begin("ESP32Tetris");
  
  Serial.println("bt inited");
  
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
//  FastLED.setMaxPowerInVoltsAndMilliamps(5,1000);
  FastLED.clear(true);
  FastLED.setBrightness(64);
  FastLED.show();
  Serial.println("fastled shows");
}

void loop()
{
  switch (currentApp) {
    case APP_MENU : runMenu();
    break;
    case APP_TETRIS : runTetris();
    break;
    case APP_SNAKE : runSnake();
    break;
    case APP_BREAKOUT : runBreakout();
    break;
    case APP_PIXEL : runPixel(); break;
    case APP_ALLWHITE: runAllWhite(); break;
  }
}

void runAllWhite() {
  bool isRunning = true;
  AllWhite aw = AllWhite();
  aw.setup();
  while(isRunning) {
    isRunning = aw.loop();
  }
}

void runMenu(){
  bool isRunning = true;
  Menu menu = Menu();
  menu.setup();
  while (isRunning) {
    isRunning = menu.loop();
  }
}

void runTetris(){
  bool isRunning = true;
  Tetris tetris = Tetris();
  tetris.setup();
  while (isRunning) {
    isRunning = tetris.loop();
  }
}

void runSnake(){
  bool isRunning = true;
  Snake snake = Snake();
  snake.setup();
  while (isRunning) {
    isRunning = snake.loop();
  }
}

void runBreakout(){
  bool isRunning = true;
  Breakout breakout = Breakout();
  breakout.setup();
  while (isRunning) {
    isRunning = breakout.loop();
  }
}

void runPixel(){
  bool isRunning = true;
  Pixel pixel = Pixel();
  pixel.setup();
  while (isRunning) {
    isRunning = pixel.loop();
  }
}
