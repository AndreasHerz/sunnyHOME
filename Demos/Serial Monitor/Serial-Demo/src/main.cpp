#include <Arduino.h>

void setup() {
  Serial.begin(9600);
  Serial.print("!!! I HAVE STARTED !!!");
}

void loop() {
  Serial.print(" - sunnyHOME ist cool");
  delay(800);
}
