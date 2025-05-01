#include <Wire.h>
#define SLAVE_ADDR 0x08

byte data_to_send = 150;

void setup() {
  Wire.begin(SLAVE_ADDR);
  Serial.begin(9600);
  Wire.onRequest(sendData);
  pinMode(A4, INPUT_PULLUP);
  pinMode(A5, INPUT_PULLUP);
}

void loop() {
  delay(1000);
}

void sendData(){
  Wire.write(data_to_send);
}