#include <Wire.h>
#include <Servo.h>

// مشخصات سخت افزاری
#define SOIL_SENSOR_PIN A0  // پین سنسور رطوبت
#define DC_MOTOR_PIN1 8      // برای کنترل L298N - IN1
#define SERVO_PIN 9          // پین موتور سروو

Servo potServo; // سروو برای چرخش گلدان

int soil_moisture = 71; // متغیر برای رطوبت خاک

void setup() {
  Wire.begin(23);
  Wire.onReceive(receiveData); // وقتی Master داده فرستاد
  Wire.onRequest(sendData);    // وقتی Master درخواست داده کرد

  potServo.attach(SERVO_PIN);

  pinMode(DC_MOTOR_PIN1, OUTPUT);

  Serial.begin(9600);
  Serial.println("edge 1 started");
}

void loop() {
  // هر ثانیه رطوبت خاک رو بخون
  int rawMoisture = analogRead(SOIL_SENSOR_PIN);
  soil_moisture = map(rawMoisture, 0, 1023, 0, 100);
  Serial.println(soil_moisture);
  // int data = Wire.read();
  // Serial.println(data);
  
  delay(1000);
}

// دریافت فرمان از Master
void receiveData(int howMany) {
  
  while(Wire.available()) {
    char command = Wire.read();
    if (command == -1){
      continue;
    }
    if(command == 'W') {
      // فرمان آبیاری

      Serial.println("Command W recieved");
      // int irrigationRate = Wire.read(); // نرخ آبیاری بر اساس درصد
      // Serial.print("irrigationRate :");
      // Serial.println(irrigationRate);

      // if (irrigationRate > 0) {
      //   openWaterValve();
      // } else {
      //   closeWaterValve();
      // }
    }
    else if(command == 'R') {
      // فرمان چرخش گلدان
      int rotation = Wire.read();
      potServo.write(rotation);
      Serial.print("rotation :");
      Serial.println(rotation);
    }

  }
}

// ارسال داده به Master
void sendData() {
  Wire.write(sprintf("%d", soil_moisture)); // ارسال مقدار رطوبت
  Serial.print("soil moisture :");
  Serial.println(sprintf("%d", soil_moisture));
}

// توابع کنترل آب
void openWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, HIGH);
}

void closeWaterValve() {
  digitalWrite(DC_MOTOR_PIN1, LOW);
}
