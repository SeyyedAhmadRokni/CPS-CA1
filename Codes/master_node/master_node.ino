#include <Wire.h>
#define SLAVE_ADDR 0x08

void setup() {
  Wire.begin();
  Serial.begin(9600);
}

void loop() {
  Wire.requestFrom(SLAVE_ADDR, 1);
  if(Wire.available()) {
    byte data = Wire.read();
    // Serial.print("Received: ");
    Serial.println(data);
  }
  delay(1000);
}