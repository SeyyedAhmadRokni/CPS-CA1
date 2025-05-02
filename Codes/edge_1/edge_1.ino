#include <EtherCard.h>

#define REQUEST_RATE 5000

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x32 };
static byte myip[] = { 192,168,2,3 };
static byte gwip[] = { 192,168,2,1 };
static byte hisip[] = { 192,168,2,2 };

byte Ethernet::buffer[1000]; // افزایش اندازه بافر
static long timer;
bool ethernetInitialized = false;

#define SOIL_SENSOR_PIN A0

int soil_moisture;
static void my_result_cb(byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}

void setup() {
  Serial.begin(57600);
  Serial.println(F("Starting Edge Client..."));

  for (int i = 0; i < 5; i++) {
    if (ether.begin(sizeof Ethernet::buffer, mymac, 10) != 0) {
      ethernetInitialized = true;
      break;
    }
    Serial.println(F("Retrying Ethernet initialization..."));
    delay(2000);
  }
  
  ether.staticSetup(myip, gwip);
  ether.copyIp(ether.hisip, hisip);
  ether.printIp("Server IP: ", ether.hisip);

  while (ether.clientWaitingGw()) {
    ether.packetLoop(ether.packetReceive());
  }
  
  Serial.println(F("Gateway found"));
  timer = -REQUEST_RATE;
}

void loop() {
  ether.packetLoop(ether.packetReceive());

  if (millis() > timer + REQUEST_RATE) {
    timer = millis();
    int rawMoisture = analogRead(SOIL_SENSOR_PIN);
    soil_moisture = map(rawMoisture, 0, 1023, 0, 100);
    
    Serial.print(F("Sending moisture: "));
    Serial.println(soil_moisture);

    // ساخت پارامترهای GET به صورت صحیح
    char params[25];
    sprintf(params, "?moisture=%d&edge=1", soil_moisture);
    
    // ارسال درخواست
    ether.browseUrl(PSTR("/moisture"), params, PSTR("192.168.2.2"), my_result_cb);
  }
}