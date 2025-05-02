#include <EtherCard.h>

// تنظیمات شبکه
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192, 168, 2, 2 };
static byte gwip[] = { 192, 168, 2, 1 };

byte Ethernet::buffer[700];

#define LIGHT_SENSOR_LEFT A1
#define LIGHT_SENSOR_RIGHT A2
#define TEMPERATURE_SENSOR A0

void setup() {
  Serial.begin(9600);
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS)) {
    Serial.println("Ethernet init failed");
  }
  ether.staticSetup(myip, gwip);
  
  // انتظار برای اتصال به گیتوی
  while (ether.clientWaitingGw()) {
    ether.packetLoop(ether.packetReceive());
  }
  
  Serial.println("Master is ready");
}

void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {
    char *data = (char *)Ethernet::buffer + pos;

    // پردازش درخواست رطوبت
    if (strstr(data, "moisture=")) {
      // خواندن پارامترها
      int moisture = getParamValue(data, "moisture");
      int edgeId = getParamValue(data, "edge");
      
      // خواندن سنسورها
      int temperature = analogRead(TEMPERATURE_SENSOR);
      int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
      int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
      
      // تصمیم‌گیری برای آبیاری
      // String response = makeWateringDecision(moisture, temperature);
      // ether.httpServerReply(response.length());
      // memcpy(ether.tcpOffset(), response.c_str(), response.length());
      const char* fixedResponse = "TEST_RESPONSE";
      ether.httpServerReply(strlen(fixedResponse));
      memcpy(ether.tcpOffset(), fixedResponse, strlen(fixedResponse));
      
      // چاپ اطلاعات برای دیباگ
      printSensorData(moisture, edgeId, temperature, lightLeft, lightRight);
    }
  }
}

// تابع کمکی برای استخراج پارامترها
int getParamValue(char* data, const char* param) {
  char* ptr = strstr(data, param);
  if (ptr) {
    ptr = strchr(ptr, '=') + 1;
    return atoi(ptr);
  }
  return -1;
}

// تابع تصمیم‌گیری آبیاری
String makeWateringDecision(int moisture, int temperature) {
  if (moisture < 50) {
    return "WATER:10"; // آبیاری با نرخ 10 سی‌سی
  } else if (moisture < 80) {
    return "WATER:5"; // آبیاری با نرخ 5 سی‌سی
  } else {
    return "NO_WATER";
  }
}

// تابع نمایش اطلاعات سنسورها
void printSensorData(int moisture, int edgeId, int temp, int lightL, int lightR) {
  Serial.print("Edge: ");
  Serial.print(edgeId);
  Serial.print(" | Moisture: ");
  Serial.print(moisture);
  Serial.print("% | Temp: ");
  Serial.print(temp);
  Serial.print(" | Light L/R: ");
  Serial.print(lightL);
  Serial.print("/");
  Serial.println(lightR);
}