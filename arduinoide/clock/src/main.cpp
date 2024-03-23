#include "Wire.h"

#include "Adafruit_NeoPixel.h"


#define DS3231_I2C_ADDRESS 0x68
#define PIXEL_PIN	14

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(60, PIXEL_PIN, NEO_GRB + NEO_KHZ800);

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
void displayTime()
{
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	// retrieve data from DS3231
	int valid = readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month,
	                           &year);
	if(! valid)
	{
		Serial.print("Invalid\n\r");
		return;
	}
	// send it to the serial monitor
	Serial.print(hour, DEC);
	// convert the byte variable to a decimal number when displayed
	Serial.print(":");

	if (minute < 10)
	{
		Serial.print("0");
	}
	Serial.print(minute, DEC);
	Serial.print(":");
	if (second < 10)
	{
		Serial.print("0");
	}
	Serial.print(second, DEC);
	Serial.print(" ");
	Serial.print(dayOfMonth, DEC);
	Serial.print("/");
	Serial.print(month, DEC);
	Serial.print("/");
	Serial.print(year, DEC);
	Serial.print(" Day of week: ");
	switch(dayOfWeek)
	{
		case 1:
			Serial.println("Sunday");
			break;
		case 2:
			Serial.println("Monday");
			break;
		case 3:
			Serial.println("Tuesday");
			break;
		case 4:
			Serial.println("Wednesday");
			break;
		case 5:
			Serial.println("Thursday");
			break;
		case 6:
			Serial.println("Friday");
			break;
		case 7:
			Serial.println("Saturday");
			break;
	}
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

int ledno(int ledno) {
	if(ledno < 0) {
		ledno = 60 - ledno;
	} else if(ledno > 60) {
		ledno -= 60;
	}
	return ledno;
}

/**
 * Hours are HOUR_COLOR, spread 12 hour over 60 pixels, 5 pixels per hour with the "big" thing in the center.
 */
void setHour(int hour) {
	hour %= 12;

	int firstled = hour * 5 - 2;
	if(firstled < 0)
		firstled = 60 - firstled;
  pixels.setPixelColor(ledno(firstled++), HOUR_COLOR2);
	pixels.setPixelColor(ledno(firstled++), HOUR_COLOR1);
	pixels.setPixelColor(ledno(firstled++), HOUR_COLOR);
	pixels.setPixelColor(ledno(firstled++), HOUR_COLOR1);
	pixels.setPixelColor(ledno(firstled++), HOUR_COLOR2);
}

#define MINUTE_COLOR 0x00ff00
#define MINUTE_COLOR1 0x002200

#define SECOND_COLOR 0x0000ff

/*
 *
 */
void setMinute(byte minute) {
	int led = minute % 60 - 1;
	pixels.setPixelColor(ledno(led++), MINUTE_COLOR1);
	pixels.setPixelColor(ledno(led++), MINUTE_COLOR);
	pixels.setPixelColor(ledno(led++), MINUTE_COLOR1);
}

void setSecond(byte second) {
	int led = second % 60;
	pixels.setPixelColor(led, SECOND_COLOR);

}

byte lastSecond = 255;

void setLedTime() {
	byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;
	// retrieve data from DS3231
	int valid = readDS3231time(&second, &minute, &hour, &dayOfWeek, &dayOfMonth, &month, &year);
	if(! valid)
		return;
	if(second == lastSecond)
		return;
	lastSecond = second;

	pixels.clear();
	setHour(hour);
	setMinute(minute);
	setSecond(second);
}

void setup()
{
	Wire.begin();
	Serial.begin(9600);
	// pixels.setBrightness(50);
	pixels.begin();
	pixels.show();					// Turn whole clock off
	// set the initial time here:
	// DS3231 seconds, minutes, hours, day, date, month, year
	setDS3231time((byte) 30,(byte)21,(byte)19,(byte)2,(byte)5,(byte)9,(byte)16);
}

void loop()
{
	setLedTime();
	pixels.show();
	delay(100);

	// displayTime(); // display the real-time clock data on the Serial Monitor,
	// delay(1000); // every second
	// for(int j = 0; j < 60; j++) {
	// 	uint32_t col = Wheel((count + j) & 0xff);
	// 	pixels.setPixelColor(j, col);
	// }
}
