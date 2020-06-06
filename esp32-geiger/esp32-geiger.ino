/**************************************************************************
  This is a library for several Adafruit displays based on ST77* drivers.

  Works with the Adafruit 1.8" TFT Breakout w/SD card
    ----> http://www.adafruit.com/products/358
  The 1.8" TFT shield
    ----> https://www.adafruit.com/product/802
  The 1.44" TFT breakout
    ----> https://www.adafruit.com/product/2088
  The 1.14" TFT breakout
  ----> https://www.adafruit.com/product/4383
  The 1.3" TFT breakout
  ----> https://www.adafruit.com/product/4313
  The 1.54" TFT breakout
    ----> https://www.adafruit.com/product/3787
  The 2.0" TFT breakout
    ----> https://www.adafruit.com/product/4311
  as well as Adafruit raw 1.8" TFT display
    ----> http://www.adafruit.com/products/618

  Check out the links above for our tutorials and wiring diagrams.
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional).

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 **************************************************************************/

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>

// see https://github.com/adafruit/Adafruit-ST7735-Library/issues/65
#define TFT_CS 26
#define TFT_DC 27
#define TFT_MOSI 23
#define TFT_SCLK 18
#define TFT_RST 4

// For ST7735-based displays, we will use this call
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

// size is 128x160 pxl

int count = 1234;
int lastCount = 0;
long lastMillis = millis();

//-- bucket counter for radiation average, 10 second intervals
#define MINUTES 5
#define SEC_PER_BUCKET 10

#define MAX_BUCKETS         (MINUTES * 60 / SEC_PER_BUCKET + 1)
#define BUCKETS_PER_MINUTE  (60 / SEC_PER_BUCKET)

#define TUBE_CPM_TO_USV     0.0057

int buckets[MAX_BUCKETS];
int bucketindex;            // put index
int nbuckets;               // How many buckets are currently filled
unsigned long nextBucketSwap;

#define GSY       (128 - 40)
#define GEY       (128)
#define GSX       0
#define GEX       160

void setup(void) {
  memset(buckets, 0, sizeof(buckets));
  Serial.begin(9600);
  Serial.println(F("Geiger counter init"));

  // Use this initializer if using a 1.8" TFT screen:
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab

  Serial.println(F("Initialized"));

  tft.setRotation(1);
  uint16_t time = millis();
  tft.fillScreen(ST77XX_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);
  delay(500);

  pinMode(34, INPUT);
  attachInterrupt(34, tickReceived, FALLING);

  tft.setTextWrap(false);
  tft.setCursor(0, 30);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  tft.fillRoundRect(0, 20, 120, 80, 8, ST77XX_BLUE);
  tft.setCursor(8, 25);
  tft.print("Collecting..");

  //-- graph
  tft.fillRect(GSX, GSY, GEX, GEY, ST77XX_CYAN);
  int col = ST77XX_WHITE;
  tft.drawLine(GSX, GSY, GEX, GSY, col);
  tft.drawLine(GEX, GSY, GEX, GEY, col);
  tft.drawLine(GSX, GEY, GEX, GEY, col);
  tft.drawLine(GSX, GSY, GSX, GEY, col);

  //-- Start timing data
  nextBucketSwap = millis() + (SEC_PER_BUCKET * 1000);
}


IRAM_ATTR void tickReceived() {
  buckets[bucketindex]++;
}

int graphIndex = GSX + 1;

/**
 * Render a small graph of radiation counts per 10 seconds.
 */
void renderGraphArea() {
    int bix = bucketindex - 1;
    if(bix < 0)
      bix = MAX_BUCKETS - 1;
    int count = buckets[bix];
    int gh = GEY - GSY - 2;

    int h = round(log(count) / log(1.2));
    if(h > gh)
      h = gh;
    tft.drawLine(graphIndex, GEY- 1, graphIndex, GEY-1 - h, ST77XX_BLUE);
    tft.drawPixel(graphIndex, GEY-1 - h, ST77XX_RED);
    graphIndex++;
    tft.drawLine(graphIndex, GEY-1, graphIndex, GEY-1 - h, ST77XX_BLUE);
    tft.drawPixel(graphIndex, GEY-1 - h, ST77XX_RED);
    if(graphIndex >= GEX - 1)
      graphIndex = GSX + 1;
}

void calculateMeasures() {
  //-- Calculate the 1 minute and max minute averages using the available buckets.
  int b = nbuckets;
  int bix = bucketindex;
  if(b < 2)
    return;

  int count = 0;
  int oneminutecount = -1;
  int totalcount = 0;
  while(count < b- 1) {
    count++;

    bix--;
    if(bix < 0)
      bix = MAX_BUCKETS-1;
    int rc = buckets[bix];
    totalcount += rc;

    if(count == BUCKETS_PER_MINUTE) {
      oneminutecount = totalcount;
    }
  }

  //-- Calculate average per minute over the full bucket size
  double mins = count * SEC_PER_BUCKET / 60.0;
  double cpm = ((double) totalcount / mins);
  
  double cpm1m;

  if(oneminutecount == -1) {    // No minute data collected yet?
    cpm1m = cpm;
  } else {
    cpm1m = (double) oneminutecount;
  }
  
  //-- Start reporting
  tft.fillRect(0, 0, 160, GSY, ST77XX_BLACK);
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  
  char buf[20];
  int secs= ((b - 1) * SEC_PER_BUCKET);
  sprintf(buf, "#mins %d:%02d", secs / 60, secs % 60);
  tft.println(buf);

  sprintf(buf, "#tot %d", totalcount);
  tft.println(buf);

  sprintf(buf, "5m cpm %.2f", cpm);
  tft.println(buf);

  sprintf(buf, "1m cpm %.0f", cpm1m);
  tft.println(buf);

  //-- Calculate to uSv/hr
  double usv = cpm1m * TUBE_CPM_TO_USV;
  if(usv < 0.0010) {
    //present in nanosievert/hr
    sprintf(buf, "nSv/hr %.2f", (usv * 1000));
  } else {
    sprintf(buf, "uSv/hr %.2f", usv);
  }
  tft.println(buf);
}

void loop() {
  char buf[20];

  //-- Need to advance the put index?
  bool needUpdate = false;
  unsigned long cts = millis();
  if(cts > nextBucketSwap) {
    needUpdate = true;
    int bix = bucketindex + 1;
    int bc = nbuckets;
    if(bix >= MAX_BUCKETS) {
      bix = 0;
    }
    buckets[bix] = 0;
    bucketindex = bix;
    if(bc < MAX_BUCKETS)
      nbuckets = bc + 1;
    
    nextBucketSwap = cts + (SEC_PER_BUCKET * 1000);
  }

  if(needUpdate) {
    calculateMeasures();
    renderGraphArea();
  }

//  if(count == lastCount) {
//    delay(100);
//  } else {
//    tft.fillRoundRect(0, 60, 120, 80, 8, ST77XX_BLUE);
//    tft.setCursor(8, 65);
//    lastCount = count;
//    tft.setTextSize(4);
//    sprintf(buf, "%d", count);
//    tft.print(buf);
//  }  
}
