#include <Wire.h>
#include <DHT.h>

#define DHT_PIN 2
#define DHT_TYPE DHT22
#define EDGE_NODE_1 23
#define LIGHT_SENSOR_LEFT A1
#define LIGHT_SENSOR_RIGHT A2

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Wire.begin(); // به عنوان مستر
  Serial.begin(9600);
  dht.begin();

  delay(2000); // فرصت دادن به سنسور DHT
  Serial.println("Plant management system initialized");
}

void loop() {
  float temperature = dht.readTemperature();
  int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
  int lightRight = analogRead(LIGHT_SENSOR_RIGHT);

  int optimalPosition = (lightLeft > lightRight) ? 0 : 60;

  manageEdgeNode(EDGE_NODE_1, temperature, optimalPosition);

  delay(5000); // هر ۵ ثانیه یک‌بار بررسی
}

void manageEdgeNode(int nodeAddress, float temperature, int rotationPosition) {
  uint8_t moisture = 0;

  // ارسال فرمان 'W' برای درخواست مقدار رطوبت
  Wire.beginTransmission(nodeAddress);
  Wire.write('W');
  Wire.endTransmission();

  // دریافت مقدار رطوبت
  Wire.requestFrom(nodeAddress, 1);
  unsigned long start = millis();
  while (Wire.available() < 1) {
    if (millis() - start > 1000) {
      Serial.println("Timeout reading moisture");
      return;
    }
  }
  moisture = Wire.read();

  Serial.print("Node ");
  Serial.print(nodeAddress, HEX);
  Serial.print(" - Moisture: ");
  Serial.print(moisture);
  Serial.print("%, Temperature: ");
  Serial.print(temperature);
  Serial.println("°C");

  int irrigationRate = calculateIrrigationRate(moisture, temperature);

  // ارسال نرخ آبیاری
  Wire.beginTransmission(nodeAddress);
  Wire.write('W');
  Wire.write((uint8_t)irrigationRate);
  Wire.endTransmission();

  // ارسال زاویه چرخش
  Wire.beginTransmission(nodeAddress);
  Wire.write('R');
  Wire.write((uint8_t)rotationPosition);
  Wire.endTransmission();

  Serial.print("Sent to Node ");
  Serial.print(nodeAddress, HEX);
  Serial.print(": Irrigation ");
  Serial.print(irrigationRate);
  Serial.print(", Rotation ");
  Serial.print(rotationPosition);
  Serial.println("°");
}

int calculateIrrigationRate(int moisture, float temperature) {
  if (moisture > 80) return 0;
  else if (moisture < 50) return 15;
  else return (temperature > 25) ? 10 : 5;
}
