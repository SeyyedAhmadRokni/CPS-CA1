// ===== Master Node =====
#include <EtherCard.h>

// تنظیمات شبکه
static byte mymac[] = {0x74,0x69,0x69,0x2D,0x30,0x31};
static byte myip[]  = {192,168,2,2};
static byte gwip[]  = {192,168,2,1};

byte Ethernet::buffer[1000];

// سنسورها
#define LIGHT_SENSOR_LEFT  A1
#define LIGHT_SENSOR_RIGHT A2
#define TEMPERATURE_SENSOR A0

// دستورات PROGMEM
const char water15[] PROGMEM = "WATER:15";
const char water10[] PROGMEM = "WATER:10";
const char water5[]  PROGMEM = "WATER:5";
const char noWater[] PROGMEM = "NO_WATER";
const char rot0[]    PROGMEM = "ROTATE:0";
const char rot60[]   PROGMEM = "ROTATE:60";

void setup() {
  Serial.begin(9600);
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS)) {
    Serial.println("Ethernet init failed");
  }
  ether.staticSetup(myip, gwip);
  
  // while (ether.clientWaitingGw()) {
  //   ether.packetLoop(ether.packetReceive());
  // }
  
  Serial.println("Master is ready");
}

void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (!pos) return;
  
  char* req = (char*)Ethernet::buffer + pos;
  if (!strstr(req, "moisture=")) return;
  Serial.println("Mew");

  // خواندن پارامترها
  int moisture = getParamValue(req, "moisture");
  int edgeId   = getParamValue(req, "edge");
  int temp     = analogRead(TEMPERATURE_SENSOR);
  int lightL   = analogRead(LIGHT_SENSOR_LEFT);
  int lightR   = analogRead(LIGHT_SENSOR_RIGHT);

  // تصمیم آبیاری
  const char* waterCmd;
  if (moisture > 80) {
    waterCmd = noWater;
  } else if (moisture < 50) {
    waterCmd = water15;
  } else {
    waterCmd = (temp > 25) ? water10 : water5;
  }

  // تصمیم چرخش
  const char* rotCmd = (lightL > lightR) ? rot0 : rot60;
  
  // ساخت پاسخ HTTP
  char waterBuf[20], rotBuf[20];
  strcpy_P(waterBuf, waterCmd);
  strcpy_P(rotBuf, rotCmd);
  char body[100];

  strcpy(body, waterBuf);
  strcat(body, ";");
  strcat(body, rotBuf);

  int bodyLen = strlen(body);
  char resp[256];
  snprintf(resp, sizeof(resp),
  "HTTP/1.0 200 OK\r\n"
  "%s"
  "\r\n",
   body);
  Serial.print("Resp Len: ");
  
  int totalLen = strlen(resp);
  Serial.println(totalLen);
  memcpy(ether.tcpOffset(), resp, totalLen);
  ether.httpServerReply(totalLen);
  // ارسال
  // لاگ برای دیباگ
  printSensorData(moisture, edgeId, temp, lightL, lightR);
}
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

// استخراج مقدار query parameter
int getParamValue(char* data, const char* param) {
  char* p = strstr(data, param);
  if (!p) return -1;
  p = strchr(p, '=') + 1;
  char buf[6] = {0};
  int i = 0;
  while (*p && *p!='&' && i<5) buf[i++] = *p++;
  return atoi(buf);
}
