#include <EtherCard.h>

#define REQUEST_RATE 5000
#define SOIL_SENSOR_PIN A0

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x32 };
static byte myip[] = { 192,168,2,3 };
static byte gwip[] = { 192,168,2,1 };
static byte hisip[] = { 192,168,2,2 };

byte Ethernet::buffer[1000];
static long timer;
bool ethernetInitialized = false;
int edgeId = 1; // شناسه گره

// تابع callback برای پردازش پاسخ
static void responseCallback(byte status, word off, word len) {
  Serial.println("\n=== Response Received ===");
  
  if (status == 0) { // اگر وضعیت 0 باشد یعنی پاسخ دریافت شده
    Serial.print("Status: Success | ");
    Serial.print("Length: ");
    Serial.println(len);
    
    if (len > 0) {
      Serial.print("Data: ");
      Serial.println((const char*) Ethernet::buffer + off);
      
      // پردازش پاسخ سرور
      String response = String((const char*) Ethernet::buffer + off);
      if (response.startsWith("WATER:")) {
        int rate = response.substring(6).toInt();
        Serial.print("Start watering with rate: ");
        Serial.println(rate);
        // فعال کردن موتور آبیاری
      } else if (response == "NO_WATER") {
        Serial.println("No watering needed");
        // غیرفعال کردن موتور آبیاری
      }
    }
  } else {
    Serial.print("Error in response, status: ");
    Serial.println(status);
  }
}

void setup() {
  Serial.begin(57600);
  Serial.println(F("\nStarting Edge Client..."));

  // تلاش برای اتصال اترنت
  for (int i = 0; i < 5; i++) {
    if (ether.begin(sizeof Ethernet::buffer, mymac, 10)) {
      ethernetInitialized = true;
      break;
    }
    Serial.println(F("Retrying Ethernet initialization..."));
    delay(2000);
  }
  

  ether.staticSetup(myip, gwip);
  ether.copyIp(ether.hisip, hisip);
  
  while (ether.clientWaitingGw()) {
    ether.packetLoop(ether.packetReceive());
  }

  Serial.println(F("Network ready"));
  timer = -REQUEST_RATE;
}

void loop() {
  ether.packetLoop(ether.packetReceive());
  
  if (millis() > timer + REQUEST_RATE) {
    timer = millis();
    
    // خواندن رطوبت خاک
    int rawMoisture = analogRead(SOIL_SENSOR_PIN);
    int moisture = map(rawMoisture, 0, 1023, 0, 100);
    
    Serial.print(F("\nSending moisture: "));
    Serial.print(moisture);
    Serial.print(F("% | Edge ID: "));
    Serial.println(edgeId);

    // ساخت پارامترهای درخواست
    char params[30];
    sprintf(params, "?moisture=%d&edge=%d", moisture, edgeId);
    
    // بررسی اتصال قبل از ارسال
    if (!ether.clientWaitingGw()) {
      ether.browseUrl(PSTR("/moisture"), params, PSTR("192.168.2.2"), responseCallback);
    } else {
      Serial.println(F("Network not ready, skipping request"));
    }
  }
}