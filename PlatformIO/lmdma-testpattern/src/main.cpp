#include <Arduino.h>
#include <SPI.h>

#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
MatrixPanel_I2S_DMA dma_display;

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


// Or use an Alternative non-DMA library, i.e:
//#include <P3RGB64x32MatrixPanel.h>
//P3RGB64x32MatrixPanel display;

// Input a value 0 to 24 to get a color value.
// The colours are a transition r - g - b - back to r.
uint16_t Wheel(byte WheelPos) {
  if(WheelPos < 8) {
   return dma_display.color444(15 - WheelPos*2, WheelPos*2, 0);
  } else if(WheelPos < 16) {
   WheelPos -= 8;
   return dma_display.color444(0, 15-WheelPos*2, WheelPos*2);
  } else {
   WheelPos -= 16;
   return dma_display.color444(0, WheelPos*2, 7 - WheelPos*2);
  }
}

void setup() {


  Serial.begin(115200);

  Serial.println("*****************************************************");
  Serial.println(" HELLO !");
  Serial.println("*****************************************************");

  dma_display.begin(R1_PIN, G1_PIN, B1_PIN, R2_PIN, G2_PIN, B2_PIN, A_PIN, B_PIN, C_PIN, D_PIN, E_PIN, LAT_PIN, OE_PIN, CLK_PIN );  // setup the LED matrix

  // dma_display.begin(); // use default pins

  // draw a pixel in solid white
  dma_display.drawPixel(0, 0, dma_display.color444(15, 15, 15));
  delay(500);

  // fix the screen with green
  dma_display.fillRect(0, 0, dma_display.width(), dma_display.height(), dma_display.color444(0, 15, 0));
  delay(500);

  // draw a box in yellow
  dma_display.drawRect(0, 0, dma_display.width(), dma_display.height(), dma_display.color444(15, 15, 0));
  delay(500);

  // draw an 'X' in red
  dma_display.drawLine(0, 0, dma_display.width()-1, dma_display.height()-1, dma_display.color444(15, 0, 0));
  dma_display.drawLine(dma_display.width()-1, 0, 0, dma_display.height()-1, dma_display.color444(15, 0, 0));
  delay(500);

  // draw a blue circle
  dma_display.drawCircle(10, 10, 10, dma_display.color444(0, 0, 15));
  delay(500);

  // fill a violet circle
  dma_display.fillCircle(40, 21, 10, dma_display.color444(15, 0, 15));
  delay(500);

  // fill the screen with 'black'
  dma_display.fillScreen(dma_display.color444(0, 0, 0));


  // draw some text!
  dma_display.setTextSize(1);     // size 1 == 8 pixels high
  dma_display.setTextWrap(false); // Don't wrap at end of line - will do ourselves

  dma_display.setCursor(5, 0);    // start at top left, with 8 pixel of spacing
  uint8_t w = 0;
  char *str = "ESP32 DMA";
  for (w=0; w<9; w++) {
    dma_display.setTextColor(dma_display.color565(255, 0, 255));
    dma_display.print(str[w]);
  }

  dma_display.println();
  for (w=9; w<18; w++) {
    dma_display.setTextColor(Wheel(w));
    dma_display.print(str[w]);
  }
  dma_display.println();
  //dma_display.setTextColor(dma_display.Color333(4,4,4));
  //dma_display.println("Industries");
  dma_display.setTextColor(dma_display.color444(15,15,15));
  dma_display.println("LED MATRIX!");

  // print each letter with a rainbow color
  dma_display.setTextColor(dma_display.color444(0,8,15));
  dma_display.print('3');
  dma_display.setTextColor(dma_display.color444(15,4,0));
  dma_display.print('2');
  dma_display.setTextColor(dma_display.color444(15,15,0));
  dma_display.print('x');
  dma_display.setTextColor(dma_display.color444(8,15,0));
  dma_display.print('6');
  dma_display.setTextColor(dma_display.color444(8,0,15));
  dma_display.print('4');

  // Jump a half character
  dma_display.setCursor(34, 24);
  dma_display.setTextColor(dma_display.color444(0,15,15));
  dma_display.print("*");
  dma_display.setTextColor(dma_display.color444(15,0,0));
  dma_display.print('R');
  dma_display.setTextColor(dma_display.color444(0,15,0));
  dma_display.print('G');
  dma_display.setTextColor(dma_display.color444(0,0,15));
  dma_display.print("B");
  dma_display.setTextColor(dma_display.color444(15,0,8));
  dma_display.println("*");

  delay(2000);
/*
 for (int i = 15; i > 0; i--)
 {
    // fade out
     dma_display.fillScreen(dma_display.color565(0, 0, i*17));
     delay(250);
 }

  for (int i = 15; i > 0; i--)
 {
    // draw a blue circle
    dma_display.drawCircle(10, 10, 10, dma_display.color565(i*17, 0, 0));
    delay(250);
 }
*/



  // whew!
}

void loop() {
  // do nothing
}


