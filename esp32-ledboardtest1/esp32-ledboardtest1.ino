#include <FastLED.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

#include "BluetoothSerial.h"

#define LED_PIN     2
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define NUM_LEDS    (32*32)

#define BRIGHTNESS  64

int hue = 0;

CRGB leds[NUM_LEDS];
BluetoothSerial SerialBT;


void listDir(fs::FS &fs, const char * dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);

  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.name(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("Starting");

  if (!SD.begin(5)) {
    Serial.println("SD card init failed");
  }

  int8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  listDir(SD, "/", 0);
  SerialBT.begin("ESP32Tetris");

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS); //.setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
}

void setRowLeds(int row, CRGB color) {
  int ledix = 8 * row;
  for (int i = 0; i < 8; i++) {
    leds[ledix++] = color;
  }
  FastLED.show();
}

void loop() {
  EVERY_N_MILLISECONDS(20) {
    hue += 10;
  }
  fill_rainbow(leds, NUM_LEDS, hue, 5);
  FastLED.show();
  //  delay(200);
}
