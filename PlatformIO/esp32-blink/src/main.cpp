#include <Arduino.h>

int count = 0;

void setup() {
  pinMode(2, OUTPUT);
  // put your setup code here, to run once:
}

void waitabit() {
  if(count < 4) {
    delay(1000);
  } else {
    delay(250);
  }
}

void loop() {
  count++;
  digitalWrite(2, HIGH);
  waitabit();
  digitalWrite(2, LOW);
  waitabit();
}
