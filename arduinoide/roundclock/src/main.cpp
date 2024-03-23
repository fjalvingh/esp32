#include "Wire.h"

#include "Adafruit_NeoPixel.h"


//#define DS3231_I2C_ADDRESS 0xD0

#define DS3231_I2C_ADDRESS 0x68
// #define PIXEL_PIN			12	// miso
#define PIXEL_PIN			2 //pcb2
#define BUTTON_HOUR_PIN		0			// PD0
#define	BUTTON_MIN_PIN		1			// PD1

/*--- Button handling ---*/
#define BOUNCEDELAY 		50

#define BS_IDLE			0		// Button has previous state
#define BS_WAIT			1		// Waiting for button press timeout
#define BS_DONE			2		// Debounce is done, state is valid
#define BS_MASK			0x03

#define BE_CHANGED		0x10
#define BE_PRESSED		0x20
#define BE_CLICKED		0x40

class Button {
private:
	int m_pin;
	int m_time;				// Last time it changed
	boolean m_lastPressed;	// Currently known to be pressed or not.
	byte m_state;			// current collection state

public:
	Button(int pin) {
		m_pin = pin;
	}

	void begin() {
		pinMode(m_pin, INPUT);
	}

	void read() {
		boolean pressed = digitalRead(m_pin) == LOW;
		byte pressedMask = pressed ? BE_PRESSED : 0;
		if(pressed != m_lastPressed) {
			m_lastPressed = pressed;
			m_time = millis();						// Time the value changed
			m_state = BS_WAIT | pressedMask;
		} else if((m_state & BS_MASK) == BS_WAIT) {	// Waiting for timeout?
			long dt = millis() - m_time;
			if(dt >= BOUNCEDELAY) {
				m_state = BS_DONE | BE_CHANGED;
				if(pressed)
					m_state |= BE_PRESSED | BE_CLICKED;
			}
		}
	}

	/*
	 * T if the current state is PRESSED, debounced.
	 */
	boolean isPressed() {
		return (m_state & BS_MASK) == BS_DONE && (m_state & BE_PRESSED) != 0;
	}

	boolean isClicked() {
		if(! (m_state & BE_CLICKED))
			return false;
		m_state &= ~BE_CLICKED;
		return true;
	}
};


Button hourButton = Button(BUTTON_HOUR_PIN);
Button minButton = Button(BUTTON_MIN_PIN);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
// Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, PIXEL_PIN, NEO_BRG + NEO_KHZ800);

// Convert normal decimal numbers to binary coded decimal
byte decToBcd(byte val)
{
	return( (val / 10 * 16) + (val % 10) );
}
// Convert binary coded decimal to normal decimal numbers
byte bcdToDec(byte val)
{
	return( (val / 16 * 10) + (val % 16) );
}
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year)
{
	// sets time and date data to DS3231
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0); // set next input to start at the seconds register
	Wire.write(decToBcd(second)); // set seconds
	Wire.write(decToBcd(minute)); // set minutes
	Wire.write(decToBcd(hour)); // set hours
	Wire.write(decToBcd(dayOfWeek)); // set day of week (1=Sunday, 7=Saturday)
	Wire.write(decToBcd(dayOfMonth)); // set date (1 to 31)
	Wire.write(decToBcd(month)); // set month
	Wire.write(decToBcd(year)); // set year (0 to 99)
	Wire.endTransmission();
}

int readDS3231time(byte *second,
                   byte *minute,
                   byte *hour,
                   byte *dayOfWeek,
                   byte *dayOfMonth,
                   byte *month,
                   byte *year)
{
	Wire.beginTransmission(DS3231_I2C_ADDRESS);
	Wire.write(0); // set DS3231 register pointer to 00h
	Wire.endTransmission();
	int count = Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
	if(count != 7)
		return 0;
	// request seven bytes of data from DS3231 starting from register 00h
	*second = bcdToDec(Wire.read() & 0x7f);
	*minute = bcdToDec(Wire.read());
	*hour = bcdToDec(Wire.read() & 0x3f);
	*dayOfWeek = bcdToDec(Wire.read());
	*dayOfMonth = bcdToDec(Wire.read());
	*month = bcdToDec(Wire.read());
	*year = bcdToDec(Wire.read());
	return 1;
}
uint32_t Wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
        return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
        WheelPos -= 85;
        return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
        WheelPos -= 170;
        return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
}

void flashRings(int till = 255, uint32_t color = 0x010101) {
	int rgb = 0;
	for(int col = 0; col <= till; col += 8) {
		for(int i = 0; i < pixels.numPixels(); i++) {
			rgb += color;
			pixels.setPixelColor(i, rgb);
		}
		pixels.show();
		delay(10);
	}
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel((i+j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< pixels.numPixels(); i++) {
      pixels.setPixelColor(i, Wheel(((i * 256 / pixels.numPixels()) + j) & 255));
    }
    pixels.show();
    delay(wait);
  }
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, c);    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      pixels.show();

      delay(wait);

      for (uint16_t i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

int gongType;

void gong() {
	if(gongType > 4)
		gongType = 0;
	switch(gongType) {
		case 0:
			theaterChaseRainbow(20);
			break;

		case 1:
			rainbow(20);
			break;

		case 2:
			rainbowCycle(20);
			break;

		case 3:
			theaterChase(0xaaee44, 20);
			break;
	}
	gongType++;
}

#define HOUR_COLOR	0xFF0000			// red
#define HOUR_COLOR1	0x440000			// red
#define HOUR_COLOR2	0x110000			// red

int ledno(int ledno) {
	if(ledno < 0) {
		ledno = 60 - ledno;
	} else if(ledno > 60) {
		ledno -= 60;
	}
	return ledno;
}

void setLed(int led, uint32_t color) {
	uint32_t old = pixels.getPixelColor(led);
	pixels.setPixelColor(led, old | color);
}

/**
 * Hours are HOUR_COLOR, spread 12 hour over 60 pixels, 5 pixels per hour with the "big" thing in the center.
 */
void setHour(int hour, int minute) {
	hour %= 12;
	minute %= 60;

	int firstled = hour * 5 - 2 + (minute / (60/5));
	while(firstled < 0)
		firstled += 60;

	setLed(ledno(firstled++), HOUR_COLOR2);
	setLed(ledno(firstled++), HOUR_COLOR1);
	setLed(ledno(firstled++), HOUR_COLOR);
	setLed(ledno(firstled++), HOUR_COLOR1);
	setLed(ledno(firstled++), HOUR_COLOR2);
}

#define MINUTE_COLOR 0x00ff00
#define MINUTE_COLOR1 0x002200

#define SECOND_COLOR 0x0000ff

/*
 *
 */
void setMinute(byte minute) {
	int led = minute % 60 - 1;
	setLed(ledno(led++), MINUTE_COLOR1);
	setLed(ledno(led++), MINUTE_COLOR);
	setLed(ledno(led++), MINUTE_COLOR1);
}

void setSecond(byte second) {
	int led = second % 60;
	setLed(led, SECOND_COLOR);

}

byte lastSecond = 0xff, lastHour, lastMinute;

void setLedTime() {
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	// retrieve data from DS3231
	int valid = readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
	if(! valid) {
		for(int i = 0; i < 60; i++) {
			pixels.setPixelColor(i, 0xff00ff);
		}
		return;
	}

	// if(hour == lastHour && minute == lastMinute && second == lastSecond)
	// 	return;
	lastHour = hour;
	lastMinute = minute;
	lastSecond = second;

	pixels.clear();
	if((minute == 0 || minute == 30) && second == 0) {
		gong();
		return;
	}

	setHour(hour, minute);
	setMinute(minute);
	setSecond(second);
}

void setup()
{
	pinMode(PIXEL_PIN, OUTPUT);
	minButton.begin();
	hourButton.begin();
	Wire.begin();
	// Serial.begin(9600);
	pixels.begin();
	pixels.show();					// Turn whole clock off
}

void readButtons() {
	minButton.read();
	hourButton.read();
}

boolean timeSetMode;
long timeSetEnterTime;

byte hourToSet, minToSet, secToSet;

void updateSetLeds() {
	pixels.clear();
	setHour(hourToSet, 0);
	setMinute(minToSet);
	pixels.show();
}

void enterTimesetMode() {
	if(timeSetMode)
		return;
	timeSetMode = true;
	timeSetEnterTime = millis();

	//-- Read the current time from the clock
	byte dayOfWeek, dayOfMonth, month, year;
	readDS3231time(&secToSet, &minToSet, &hourToSet, &dayOfWeek, &dayOfMonth, &month, &year);
	flashRings();

	//-- Now set the pixel data: hour and minute only, skip seconds
	updateSetLeds();
}

void checkButtonsForTimeSetMode() {
	if(hourButton.isPressed() && minButton.isPressed()) {
		hourButton.isClicked();
		minButton.isClicked();
		enterTimesetMode();
	}
}

void handleTimeSet() {
	long delta = millis() - timeSetEnterTime;
	if(delta > 5000) {
		timeSetMode = false;

		byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

		// retrieve data from DS3231
		readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
		setDS3231time(secToSet, minToSet, hourToSet, dayOfWeek, dayOfMonth, month, year);
		return;
	}

	if(hourButton.isClicked()) {
		flashRings(20, 0x010000);
		hourToSet++;
		if(hourToSet >= 24)
			hourToSet = 0;
		updateSetLeds();
		timeSetEnterTime = millis();
	}
	if(minButton.isClicked()) {
		flashRings(20, 0x000100);
		minToSet++;
		if(minToSet >= 60)
			minToSet = 0;
		updateSetLeds();
		timeSetEnterTime = millis();
	}
}

static int count = 0;

void loop()
{
	readButtons();
	if(timeSetMode) {
		handleTimeSet();
	} else {
		//-- Do we need to enter TIMESET mode?
		checkButtonsForTimeSetMode();

		// if((count++ & 0x01) == 0) {
		// 	pixels.setPixelColor(0, 0xff, 0, 0);
		// } else {
		// 	pixels.setPixelColor(0, 0, 0xff, 0);
		// }
		// pixels.show();

		setLedTime();
		pixels.show();
	}
	delay(10);
}
