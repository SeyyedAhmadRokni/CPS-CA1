#include <Wire.h>
#include <Servo.h>

// مشخصات سخت افزاری
#define SOIL_SENSOR_PIN A0  // پین سنسور رطوبت
#define DC_MOTOR_PIN1 4      // برای کنترل L298N - IN1
#define DC_MOTOR_PIN2 5      // برای کنترل L298N - IN2
#define SERVO_PIN 3          // پین موتور سروو

Servo potServo; // سروو برای چرخش گلدان

int soil_moisture = 70; // متغیر برای رطوبت خاک

void setup() {
  Wire.begin(0x10); // آدرس گره لبه اول  (گره دوم بشه 0x11)
  Wire.onReceive(receiveData); // وقتی Master داده فرستاد
  Wire.onRequest(sendData);    // وقتی Master درخواست داده کرد

  potServo.attach(SERVO_PIN);

  pinMode(DC_MOTOR_PIN1, OUTPUT);
  pinMode(DC_MOTOR_PIN2, OUTPUT);

  Serial.begin(9600);
}

void loop() {
  // هر ثانیه رطوبت خاک رو بخون
  int rawMoisture = analogRead(SOIL_SENSOR_PIN);
  soil_moisture = map(rawMoisture, 0, 1023, 0, 100); // تبدیل به درصد
  
  delay(1000);
}

// دریافت فرمان از Master
void receiveData(int howMany) {
  while(Wire.available()) {
    char command = Wire.read();
    
    if(command == 'W') {
      // فرمان آبیاری
      int irrigationRate = Wire.read(); // نرخ آبیاری بر اساس درصد
      if (irrigationRate > 0) {
        openWaterValve();
      } else {
        closeWaterValve();
      }
    }
    else if(command == 'R') {
      // فرمان چرخش گلدان
      int rotation = Wire.read();
      potServo.write(rotation);
    }
  }
}

// ارسال داده به Master
void sendData() {
  Wire.write(soil_moisture); // ارسال مقدار رطوبت
}

// توابع کنترل آب
void openWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, HIGH);
  digitalWrite(DC_MOTOR_PIN2, LOW);
}

void closeWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, LOW);
  digitalWrite(DC_MOTOR_PIN2, LOW);
}
