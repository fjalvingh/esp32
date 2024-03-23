#include "Arduino.h"

#define PIN13 13

void setup() {
  pinMode(PIN13, OUTPUT);
}

void loop() {
  for(;;) {
    digitalWrite(PIN13, LOW);
    delay(1200);
    digitalWrite(PIN13, HIGH);
    delay(600);
  }
}
