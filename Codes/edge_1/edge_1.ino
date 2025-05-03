// ===== Edge Node =====
#include <EtherCard.h>
#include <Servo.h>

#define REQUEST_RATE     3000
#define SOIL_SENSOR_PIN  A0
#define DC_MOTOR_PIN     8
#define WATER_10CC_LED   7  // was 6
#define WATER_5CC_LED    3  // was 5
#define SERVO_PIN        9
#define SERVO_POS_LED    4

static byte mymac[] = {0x74,0x69,0x69,0x2D,0x30,0x32};
static byte myip[]  = {192,168,2,3};
static byte gwip[]  = {192,168,2,1};
static byte hisip[] = {192,168,2,2};

byte Ethernet::buffer[1000];
static long timer;
bool ethernetInitialized = false;
int edgeId = 1;
Servo potServo;

int currentServoPos = 0; 

static void responseCallback(byte status, int off, int len) {
  if (status != 0 || len == 0) {
    Serial.print("Error status="); Serial.println(status);
    return;
  }

  String raw = String((char*)Ethernet::buffer + off);
  int idx = raw.indexOf("\r\n\r\n");
  if (idx < 0) {
    Serial.println("Invalid HTTP");
    return;
  }
  String body = raw.substring(idx + 4);

  Serial.println("===== RAW BODY =====");
  Serial.println(body);
  Serial.println("====================");

  int sep1 = body.indexOf(';');
  int sep2 = body.indexOf(';', sep1 + 1);

  if (sep1 < 0 || sep2 < 0) {
    Serial.println("Invalid body format");
    return;
  }

  String waterCmd = body.substring(0, sep1);
  String rotCmd   = body.substring(sep1 + 1, sep2);

  Serial.print("Water Cmd: "); Serial.println(waterCmd);
  Serial.print("Rotate Cmd: "); Serial.println(rotCmd);

  // اجرای آبیاری
  if (waterCmd.startsWith("WATER:")) {
    int rate = waterCmd.substring(6).toInt();
    Serial.print("Water rate: "); Serial.println(rate);

    digitalWrite(DC_MOTOR_PIN, HIGH);
    if (rate == 15) {
      digitalWrite(WATER_10CC_LED, HIGH);
      digitalWrite(WATER_5CC_LED, HIGH);
    } else if (rate == 10) {
      digitalWrite(WATER_10CC_LED, HIGH);
      digitalWrite(WATER_5CC_LED, LOW);
    } else if (rate == 5) {
      digitalWrite(WATER_10CC_LED, LOW);
      digitalWrite(WATER_5CC_LED, HIGH);
    }

    delay(5 * rate);

    digitalWrite(DC_MOTOR_PIN, LOW);
    digitalWrite(WATER_10CC_LED, LOW);
    digitalWrite(WATER_5CC_LED, LOW);
  }

  // اجرای چرخش
  if (rotCmd.startsWith("ROTATE:")) {
    int targetPos = rotCmd.substring(7).toInt();
    if (targetPos != currentServoPos) {
      potServo.write(targetPos);
      Serial.print("Rotate to "); Serial.print(targetPos); Serial.println("°");

      if (targetPos == 0) {
        digitalWrite(SERVO_POS_LED, LOW);
      } else if (targetPos == 60) {
        digitalWrite(SERVO_POS_LED, HIGH);
      }
      currentServoPos = targetPos;
    }
  }
}

void setup() {
  Serial.begin(9600);
  Serial.print(F("\nStarting Edge "));
  Serial.print(edgeId);
  Serial.println("...");
  potServo.attach(SERVO_PIN);
  pinMode(DC_MOTOR_PIN, OUTPUT);
  pinMode(WATER_10CC_LED, OUTPUT);
  pinMode(WATER_5CC_LED, OUTPUT);
  pinMode(SERVO_POS_LED, OUTPUT);

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
