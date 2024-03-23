#include <Arduino.h>

#include <SPI.h>
#include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SSD1306.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Si7021.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

SSD1306AsciiWire display;
// Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

Adafruit_BME280 bme; // I2C

#define DHT1_PIN 2     // Digital pin connected to the DHT sensor
#define DHT2_PIN 3     // Digital pin connected to the DHT sensor

// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment the type of sensor in use:
#define DHT1_TYPE   DHT11     // DHT 11
#define DHT2_TYPE   DHT22
// #define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

// See guide for details on sensor wiring and usage:
//   https://learn.adafruit.com/dht/overview

DHT_Unified dht1(DHT1_PIN, DHT1_TYPE);
DHT_Unified dht2(DHT2_PIN, DHT2_TYPE);

Si7021 si7021;

// void print(int x, int y, char* what) {
//   display.setTextSize(1);      // Normal 1:1 pixel scale
//   display.setTextColor(WHITE); // Draw white text
//   display.setCursor(x, y);     // Start at top-left corner
//   display.cp437(true);         // Use full 256 char 'Code Page 437' font
//   while(*what != 0) {
//     display.write(*what++);
//   }
// }

// void initPrint() {
//   display.setTextSize(1);      // Normal 1:1 pixel scale
//   display.setTextColor(WHITE); // Draw white text
//   display.cp437(true);         // Use full 256 char 'Code Page 437' font
// }

void print(char* what) {
  display.print(what);
  // while(*what != 0) {
  //   display.write(*what++); display.print(what);
  // }
}

void setup() {
  Serial.begin(9600);

  // // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for(;;); // Don't proceed, loop forever
  // }
  // initPrint();
  Wire.begin();
  Wire.setClock(400000L);
  display.begin(&Adafruit128x64, 0x3c, OLED_RESET);
  display.setFont(Adafruit5x7);

  bool status;

  // default settings
  // (you can also pass in a Wire library object like &Wire2)

  status = bme.begin(0x76);
  if (!status) {
      display.setCursor(0, 0);
      print("No BME280 found");
      while (1);
  }

  bme.setSampling(Adafruit_BME280::MODE_FORCED,
  		Adafruit_BME280::SAMPLING_X1, // temperature
  		Adafruit_BME280::SAMPLING_X1, // pressure
  		Adafruit_BME280::SAMPLING_X1, // humidity
  		Adafruit_BME280::FILTER_OFF,
  		Adafruit_BME280::STANDBY_MS_1000);

  dht1.begin();
  dht2.begin();

  si7021.begin();

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  // display.display();
  delay(1500); // Pause for 2 seconds

  // Clear the buffer
  display.clear();

  // testdrawchar();      // Draw characters of the default font
  //
  // testdrawstyles();    // Draw 'stylized' characters
}

static int dhtcount = 0;
static sensors_event_t eventt;
static sensors_event_t eventh;
static sensors_event_t eventt2;
static sensors_event_t eventh2;

void loop() {
  char buf[20];

  // display.clear();
  // initPrint();

  bme.takeForcedMeasurement();
  float t = bme.readTemperature();

  display.setCursor(0, 0);     // Start at top-left corner
  print("bme28 ");
  dtostrf(t, 4, 1, buf);
  print(buf);
  print("C");
  print("  ");


  float h = bme.readHumidity();
  dtostrf(h, 4, 1, buf);
  print(buf);
  print("%");

  //-- dht1 sensor
  if(dhtcount == 0) {
    dht1.temperature().getEvent(&eventt);
    dht1.humidity().getEvent(&eventh);
    dht2.temperature().getEvent(&eventt2);
    dht2.humidity().getEvent(&eventh2);
  }
  display.setCursor(0, 8);

  if (isnan(eventt.temperature)) {
    print("\nnotemp!");
  } else {
    print("\ndht11 ");
    dtostrf(eventt.temperature, 4, 1, buf);
    print(buf);
    print("C  ");
  }

  // Get humidity event and print its value.
  if (isnan(eventh.relative_humidity)) {
    print("nohu!");
  } else {
    dtostrf(eventh.relative_humidity, 4, 1, buf);
    print(buf);
    print("%");
  }

  // dht22
  display.setCursor(0, 16);
  if (isnan(eventt.temperature)) {
    print("\nnotemp!");
  } else {
    print("\ndht22 ");
    dtostrf(eventt2.temperature, 4, 1, buf);
    print(buf);
    print("C  ");
  }

  // Get humidity event and print its value.
  if (isnan(eventh2.relative_humidity)) {
    print("nohu!");
  } else {
    dtostrf(eventh2.relative_humidity, 4, 1, buf);
    print(buf);
    print("%");
  }

  if(++dhtcount > 2)
    dhtcount = 0;

  //-- Si7021
  float h3 = si7021.measureHumidity();
  float t3 = si7021.getTemperatureFromPreviousHumidityMeasurement();
  display.setCursor(0, 24);
  print("\n7021  ");

  dtostrf(t3, 4, 1, buf);
  print(buf);
  print("C  ");

  dtostrf(h3, 4, 1, buf);
  print(buf);
  print("%");

  // display.setCursor(0, 30);
  // display.print(count++);

  // display.display();
  delay(1000);
}
