// ===== Edge Node =====
#include <EtherCard.h>
#include <Servo.h>

#define REQUEST_RATE     3000
#define SOIL_SENSOR_PIN  A0
#define DC_MOTOR_PIN     7
#define SERVO_PIN        6
#define SERVO_LED_PIN    12

static byte mymac[] = {0x74,0x69,0x69,0x2D,0x30,0x32};
static byte myip[]  = {192,168,2,3};
static byte gwip[]  = {192,168,2,1};
static byte hisip[] = {192,168,2,2};

byte Ethernet::buffer[1000];
static long timer;
bool ethernetInitialized = false;
int edgeId = 1;
Servo potServo;

static void responseCallback(byte status, int off, int len) {
  if (status != 0 || len == 0) {
    Serial.print("Error status="); Serial.println(status);
    return;
  }

  // استخراج body از پاسخ HTTP
  char body[len + 1];
  memcpy(body, Ethernet::buffer + off, len);
  body[len] = '\0'; // پایان رشته

  Serial.println("===== RAW BODY =====");
  Serial.println(body);
  Serial.println("====================");

  // تفکیک دستورات
  char* sep = strchr(body, ';');
  if (!sep) {
    Serial.println("Invalid format: missing ';'");
    return;
  }

  *sep = '\0'; // جدا کردن دو بخش
  const char* waterCmd = body;
  const char* rotCmd = sep + 1;

  Serial.print("Water Cmd: "); Serial.println(waterCmd);
  Serial.print("Rotate Cmd: "); Serial.println(rotCmd);

  // اجرای آبیاری
  if (strncmp(waterCmd, "WATER:", 6) == 0) {
    int rate = atoi(waterCmd + 6);
    Serial.print("Water rate: "); Serial.println(rate);
    digitalWrite(DC_MOTOR_PIN, HIGH);
    delay(rate * 1000);
    digitalWrite(DC_MOTOR_PIN, LOW);
  }

  // اجرای چرخش
  if (strcmp(rotCmd, "ROTATE:0") == 0) {
    Serial.println("Rotate to 0°");
    digitalWrite(SERVO_LED_PIN, HIGH);
    potServo.write(0);
    delay(500);
    digitalWrite(SERVO_LED_PIN, LOW);
  } else if (strcmp(rotCmd, "ROTATE:60") == 0) {
    Serial.println("Rotate to 60°");
    digitalWrite(SERVO_LED_PIN, HIGH);
    potServo.write(60);
    delay(500);
    digitalWrite(SERVO_LED_PIN, LOW);
  } else {
    Serial.println("Unknown rotation command.");
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("\nStarting Edge Client..."));
  potServo.attach(SERVO_PIN);
  pinMode(DC_MOTOR_PIN, OUTPUT);
  pinMode(SERVO_LED_PIN, OUTPUT);
  digitalWrite(DC_MOTOR_PIN, LOW);
  digitalWrite(SERVO_LED_PIN, LOW);

  // راه‌اندازی Ethernet
  for (int i = 0; i < 3; i++) {
    if (ether.begin(sizeof Ethernet::buffer, mymac, SS)) break;
    Serial.println(F("Retrying Ethernet initialization..."));
    delay(1000);
  }
  ether.staticSetup(myip, gwip);
  ether.copyIp(ether.hisip, hisip);

  while (ether.clientWaitingGw()) {
    ether.packetLoop(ether.packetReceive());
  }

  Serial.println("Edge Client ready");
}

void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (millis() - timer < REQUEST_RATE) return;
  timer = millis();

  int rawM = analogRead(SOIL_SENSOR_PIN);
  int moisture = map(rawM, 0, 1023, 0, 100);
  Serial.print("Sending moisture="); Serial.println(moisture);

  char params[30];
  sprintf(params, "?moisture=%d&edge=%d", moisture, edgeId);
  ether.browseUrl(PSTR("/moisture"), params, PSTR("192.168.2.2"), responseCallback);
}
