#include <Wire.h>
#include <Servo.h>

#define SOIL_SENSOR_PIN A0
#define DC_MOTOR_PIN1 8
#define SERVO_PIN 9

Servo potServo;
int soil_moisture = 0;

void setup() {
  Wire.begin(23);  // آدرس نود اج
  Wire.onReceive(receiveData);
  Wire.onRequest(sendData);

  potServo.attach(SERVO_PIN);
  pinMode(DC_MOTOR_PIN1, OUTPUT);

  Serial.begin(9600);
  Serial.println("Edge node started");
}

void loop() {
  int rawMoisture = analogRead(SOIL_SENSOR_PIN);
  soil_moisture = map(rawMoisture, 0, 1023, 0, 100);
  Serial.print("Current soil moisture: ");
  Serial.println(soil_moisture);
  delay(1000);
}

void receiveData(int howMany) {
  while (Wire.available()) {
    char command = Wire.read();
    if (command == 'W') {
      if (Wire.available()) {
        int irrigationRate = Wire.read();
        Serial.print("Irrigation command received: ");
        Serial.println(irrigationRate);
        if (irrigationRate > 0) {
          openWaterValve();
        } else {
          closeWaterValve();
        }
      } else {
        // دستور دریافت مقدار رطوبت بود (صرفاً 'W')
        // پاسخ در sendData داده می‌شود
      }
    } else if (command == 'R') {
      int rotation = Wire.read();
      Serial.print("Rotation command received: ");
      Serial.println(rotation);
      potServo.write(rotation);
    }
  }
}

void sendData() {
  Wire.write((uint8_t)soil_moisture);
  Serial.print("Sending soil moisture: ");
  Serial.println(soil_moisture);
}

void openWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, HIGH);
}

void closeWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, LOW);
}
