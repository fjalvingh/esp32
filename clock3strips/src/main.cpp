#include "Wire.h"

#include "Adafruit_NeoPixel.h"


#define DS3231_I2C_ADDRESS 0x68
#define PIXEL_PIN			14
#define BUTTON_HOUR_PIN		11
#define	BUTTON_MIN_PIN		12
#define LDR_PIN				A1

#define RING1_SIZE		60
#define RING2_SIZE		24
#define RING3_SIZE		12

#define FULL_SIZE		(RING1_SIZE + RING2_SIZE + RING3_SIZE)


/*--- Button handling ---*/
#define BOUNCEDELAY 	50

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



Adafruit_NeoPixel pixels = Adafruit_NeoPixel(FULL_SIZE, PIXEL_PIN, NEO_GRB + NEO_KHZ800);
Button hourButton = Button(BUTTON_HOUR_PIN);
Button minButton = Button(BUTTON_MIN_PIN);

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

#define HOUR_COLOR	0xFF0000			// red
#define HOUR_COLOR1	0x440000			// red
#define HOUR_COLOR2	0x110000			// red

int ledno(int ring, int ledno) {
	int base, mod;
	switch(ring) {
		case 0:
			base = 0; mod = RING1_SIZE;
			break;

		case 1:
			base = RING1_SIZE; mod = RING2_SIZE;
			break;

		case 2:
			base = RING2_SIZE+RING1_SIZE; mod = RING3_SIZE;
			break;
	}

	while(ledno < 0)
		ledno += mod;
	while(ledno > mod)
		ledno -= mod;
//	Serial.print(ledno + base);
//	Serial.write("\n");

	return ledno + base;
}

void setLed(int led, uint32_t color) {
	uint32_t old = pixels.getPixelColor(led);
	pixels.setPixelColor(led, old | color);
}

/**
 * Hours are HOUR_COLOR, spread 12 hour over 60 pixels, 5 pixels per hour with the "big" thing in the center.
 */
void setHour(int hour) {
	hour %= 12;

	int firstled = hour * 5 - 2;
	if(firstled < 0)
		firstled = 60 - firstled;
  	setLed(ledno(0, firstled++), HOUR_COLOR2);
	setLed(ledno(0, firstled++), HOUR_COLOR1);
	setLed(ledno(0, firstled++), HOUR_COLOR);
	setLed(ledno(0, firstled++), HOUR_COLOR1);
	setLed(ledno(0, firstled++), HOUR_COLOR2);

	//-- ring 2: 24 leds -> 2 leds per hour
	firstled = hour * 2 - 1;
	pixels.setPixelColor(ledno(1, firstled++), HOUR_COLOR1);
	pixels.setPixelColor(ledno(1, firstled++), HOUR_COLOR1);

	//-- Ring 3: 12 leds
	pixels.setPixelColor(ledno(2, hour), HOUR_COLOR2);
}

#define MINUTE_COLOR 0x00ff00
#define MINUTE_COLOR1 0x002200

#define SECOND_COLOR 0x0000ff

/*
 *
 */
void setMinute(byte minute) {
	int led = minute % 60 - 1;
	setLed(ledno(0, led++), MINUTE_COLOR1);
	setLed(ledno(0, led++), MINUTE_COLOR);
	setLed(ledno(0, led++), MINUTE_COLOR1);
}

void setSecond(byte second) {
	int led = second % 60;
	setLed(ledno(0, led), SECOND_COLOR);

}

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

	pixels.clear();
	setHour(hour);
	setMinute(minute);
	setSecond(second);
}

void flashRings(int till = 255, int color = 0x010101) {
	int rgb = 0;
	for(int col = 0; col <= till; col += 8) {
		for(int i = 0; i < FULL_SIZE; i++) {
			rgb += color;
			pixels.setPixelColor(i, rgb);
		}
		pixels.show();
		delay(10);
	}
}

void setup()
{
	minButton.begin();
	hourButton.begin();
//	pinMode(BUTTON_HOUR_PIN, INPUT);
//	pinMode(BUTTON_MIN_PIN, INPUT);
	Wire.begin();
	Serial.begin(9600);
//	pixels.setBrightness(50);
	pixels.begin();
	pixels.show();					// Turn whole clock off
	// set the initial time here:
	// DS3231 seconds, minutes, hours, day, date, month, year
//	setDS3231time((byte) 30,(byte)21,(byte)19,(byte)2,(byte)5,(byte)9,(byte)16);
}

void readLDR() {
	int val = analogRead(LDR_PIN);
	Serial.write("val=");
	Serial.print(val);
	Serial.write("\n");
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
	setHour(hourToSet);
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
	int valid = readDS3231time(&secToSet, &minToSet, &hourToSet, &dayOfWeek, &dayOfMonth, &month, &year);
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
		int valid = readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
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

void loop()
{
	readButtons();
	if(timeSetMode) {
		handleTimeSet();


	} else {
		//-- Do we need to enter TIMESET mode?
		checkButtonsForTimeSetMode();


		setLedTime();
		pixels.show();
	}
	delay(5);
	readLDR();

	// displayTime(); // display the real-time clock data on the Serial Monitor,
	// delay(1000); // every second
	// for(int j = 0; j < 60; j++) {
	// 	uint32_t col = Wheel((count + j) & 0xff);
	// 	pixels.setPixelColor(j, col);
	// }
}
