#include <EtherCard.h>

// MAC و IP این گره
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192, 168, 2, 2 };
static byte gwip[] = { 192, 168, 2, 1 }; // در صورت نیاز، Gateway

byte Ethernet::buffer[700];

#define LIGHT_SENSOR_LEFT A1
#define LIGHT_SENSOR_RIGHT A2
#define TEMPERATURE_SENSOR A0

// محتوای HTML برای مرورگر
const char welcomePage[] PROGMEM =
  "<!DOCTYPE html><html><head><title>Master Node</title></head>"
  "<body><h1>Welcome to Master Node</h1><p>Status OK</p></body></html>";

void setup() {
  Serial.begin(9600);
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS)) {
    Serial.println("Ethernet init failed");
  }
  ether.staticSetup(myip, gwip);
  Serial.println("Master is ready");
}

void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos) {
    char *data = (char *)Ethernet::buffer + pos;
    Serial.println("----- FULL REQUEST -----");
    Serial.println(data);
    Serial.println("------------------------");

    // اگر داده رطوبت داخل URL باشه
    if (strstr(data, "moisture=")) {
      char* moistPtr = strstr(data, "moisture=");
      int moisture = atoi(moistPtr + 9);
      Serial.print("Moisture received: ");
      Serial.println(moisture);
      int temperature = analogRead(TEMPERATURE_SENSOR);
      int lightLeft = analogRead(LIGHT_SENSOR_LEFT);
      int lightRight = analogRead(LIGHT_SENSOR_RIGHT);
      Serial.print("temperature: ");
      Serial.println(temperature);
      Serial.print("left light: ");
      Serial.println(lightLeft);
      Serial.print("right light: ");
      Serial.println(lightRight);

      const char* response;
      if (moisture < 50)
        response = "START_WATERING";
      else
        response = "NO_WATER";

      ether.httpServerReply(strlen(response));
      memcpy(ether.tcpOffset(), response, strlen(response));
    }
    else {
      // اگر مرورگر درخواست داد → نمایش HTML
      memcpy_P(ether.tcpOffset(), welcomePage, sizeof welcomePage);
      ether.httpServerReply(sizeof welcomePage - 1);
    }
  }
}