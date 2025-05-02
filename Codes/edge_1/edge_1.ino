#include <EtherCard.h>

#define REQUEST_INTERVAL 5000

static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x32 };
static byte myip[] = { 192, 168, 2, 3 };
static byte gwip[] = { 192, 168, 2, 1 };
static byte hisip[] = { 192, 168, 2, 2 }; // IP گره مرکزی
const char website[] PROGMEM = "192.168.2.2";

byte Ethernet::buffer[500];
static unsigned long timer = 0;

static void responseCallback (byte status, word off, word len) {
  Serial.print(F("Reply from master: "));
  Serial.println((const char*) Ethernet::buffer + off);
}

void setup() {
  Serial.begin(57600);
  if (!ether.begin(sizeof Ethernet::buffer, mymac, 10)) {
    Serial.println("Ethernet init failed");
  }

  ether.staticSetup(myip, gwip);
  ether.copyIp(ether.hisip, hisip); // مقصد را مشخص کن
  Serial.println("Edge client ready");
}

void loop() {
  ether.packetLoop(ether.packetReceive());

  if (millis() - timer > REQUEST_INTERVAL) {
    timer = millis();

    int moisture = random(30, 90);
    Serial.print("Sending moisture: ");
    Serial.println(moisture);

    char url[50];
    sprintf(url, "/hello/?moisture=%d", moisture);

    ether.browseUrl(PSTR("/hello/"), url + 7, website, responseCallback);
  }
}
