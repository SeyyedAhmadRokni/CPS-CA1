#include <Wire.h>
#include <DHT.h>

// پیکربندی پین‌ها و سنسورها
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LIGHT_SENSOR_LEFT A0
#define LIGHT_SENSOR_RIGHT A1
#define EDGE_NODE_1 0x10
#define EDGE_NODE_2 0x11

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Wire.begin(); // Master
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("سیستم مدیریت گلدان‌ها راه‌اندازی شد");
}

void loop() {
  float temperature = dht.readTemperature();
  
  int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
  int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
  
  int optimalPosition = (lightLeft > lightRight) ? 0 : 60;
  
  manageEdgeNode(EDGE_NODE_1, temperature, optimalPosition);
  manageEdgeNode(EDGE_NODE_2, temperature, optimalPosition);
  
  delay(3000);
}

void manageEdgeNode(int nodeAddress, float temperature, int rotationPosition) {
  int moisture = 0;
  
  // درخواست رطوبت
  Wire.beginTransmission(nodeAddress);
  Wire.write('M'); // درخواست مقدار رطوبت
  Wire.endTransmission();
  
  Wire.requestFrom(nodeAddress, 1);
  if(Wire.available()) {
    moisture = Wire.read();
    Serial.print("گلدان ");
    Serial.print(nodeAddress == 0x10 ? 1 : 2);
    Serial.print(" - رطوبت: ");
    Serial.print(moisture);
    Serial.print("%، دما: ");
    Serial.print(temperature);
    Serial.println("°C");
  }
  
  int irrigationRate = calculateIrrigationRate(moisture, temperature);
  
  // ارسال فرمان آبیاری
  Wire.beginTransmission(nodeAddress);
  Wire.write('W');
  Wire.write(irrigationRate);
  Wire.endTransmission();
  
  // ارسال فرمان چرخش
  Wire.beginTransmission(nodeAddress);
  Wire.write('R');
  Wire.write(rotationPosition);
  Wire.endTransmission();
  
  Serial.print("ارسال فرمان به گلدان ");
  Serial.print(nodeAddress == 0x10 ? 1 : 2);
  Serial.print(": آبیاری ");
  Serial.print(irrigationRate);
  Serial.print(" قطره/دقیقه، چرخش به ");
  Serial.print(rotationPosition);
  Serial.println(" درجه");
}

int calculateIrrigationRate(int moisture, float temperature) {
  if(moisture > 80) {
    return 0;
  } else if(moisture < 50) {
    return 15;
  } else {
    if(temperature > 25) {
      return 10;
    } else {
      return 5;
    }
  }
}
