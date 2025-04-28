#include <Wire.h>
#include <DHT.h>

// پیکربندی پین‌ها و سنسورها
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LIGHT_SENSOR_LEFT A0
#define LIGHT_SENSOR_RIGHT A1
#define EDGE_NODE_1 8
#define EDGE_NODE_2 9

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Wire.begin();
  Serial.begin(9600);
  dht.begin();
  
  Serial.println("سیستم مدیریت گلدان‌ها راه‌اندازی شد");
}

void loop() {
  // خواندن دمای محیط
  float temperature = dht.readTemperature();
  
  // خواندن سنسورهای نور
  int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
  int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
  
  // تعیین موقعیت بهینه گلدان‌ها
  int optimalPosition = (lightLeft > lightRight) ? 0 : 60;
  
  // مدیریت هر گره لبه
  manageEdgeNode(EDGE_NODE_1, temperature, optimalPosition);
  manageEdgeNode(EDGE_NODE_2, temperature, optimalPosition);
  
  delay(3000); // تأخیر بین هر چرخه کنترل
}

// تابع مدیریت گره لبه
void manageEdgeNode(int nodeAddress, float temperature, int rotationPosition) {
  // درخواست داده رطوبت از گره لبه
  Wire.requestFrom(nodeAddress, 1);
  int moisture = 0;
  
  if(Wire.available()) {
    moisture = Wire.read();
    Serial.print("گلدان ");
    Serial.print(nodeAddress - 7);
    Serial.print(" - رطوبت: ");
    Serial.print(moisture);
    Serial.print("%، دما: ");
    Serial.print(temperature);
    Serial.println("°C");
  }
  
  // محاسبه نرخ آبیاری
  int irrigationRate = calculateIrrigationRate(moisture, temperature);
  
  // ارسال فرمان به گره لبه
  Wire.beginTransmission(nodeAddress);
  Wire.write('W'); // فرمان آبیاری
  Wire.write(irrigationRate);
  Wire.write('R'); // فرمان چرخش
  Wire.write(rotationPosition);
  Wire.endTransmission();
  
  Serial.print("ارسال فرمان به گلدان ");
  Serial.print(nodeAddress - 7);
  Serial.print(": آبیاری ");
  Serial.print(irrigationRate);
  Serial.print(" سی‌سی/دقیقه، چرخش به ");
  Serial.print(rotationPosition);
  Serial.println(" درجه");
}

// تابع محاسبه نرخ آبیاری
int calculateIrrigationRate(int moisture, float temperature) {
  if(moisture > 80) {
    return 0; // آبیاری انجام نشود
  }
  else if(moisture < 50) {
    return 15; // آبیاری با نرخ 15 سی‌سی/دقیقه
  }
  else {
    // رطوبت بین 50 تا 80 درصد
    if(temperature > 25) {
      return 10; // نرخ 10 سی‌سی/دقیقه
    }
    else {
      return 5; // نرخ 5 سی‌سی/دقیقه
    }
  }
}