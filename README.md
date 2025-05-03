# Smart Irrigation Embedded System Using Arduino Boards

## Code explenation

### Master

```cpp
void setup() {
  Serial.begin(9600);
  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS)) {
    Serial.println("Ethernet init failed");
  }
  ether.staticSetup(myip, gwip);
  
  Serial.println("Master is ready");
}
```

آماده‌سازی ماژول شبکه، تنظیم آدرس IP.

```cpp
void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (!pos) return;
```

دریافت یک بسته از شبکه و بررسی بسته دریافتی و چک کردن اینکه آیا یک درخواست HTTP است یا نه. اگر هست، مکان شروع داده‌های آن را برمی‌گرداند.

```cpp
  char* req = (char*)Ethernet::buffer + pos;
  if (!strstr(req, "moisture=")) return;
```

این بخش بررسی می‌کند که آیا در داده‌ی دریافتی پارامتر moisture (رطوبت) وجود دارد یا نه. در صورت نبودن، ادامه‌ی پردازش متوقف می‌شود.

```cpp
  int moisture = getParamValue(req, "moisture");
  int edgeId   = getParamValue(req, "edge");
  int temp     = analogRead(TEMPERATURE_SENSOR);
  int lightL   = analogRead(LIGHT_SENSOR_LEFT);
  int lightR   = analogRead(LIGHT_SENSOR_RIGHT);
```

در این بخش، مقدار پارامترهای ارسالی از کلاینت استخراج می‌شود(رطوبیت و شماره edge) و همچنین مقادیر سنسورها دما و نور خوانده می‌شود.

```cpp
  const char* waterCmd;
  if (moisture > 80) {
    waterCmd = noWater;
  } else if (moisture < 50) {
    waterCmd = water15;
  } else {
    waterCmd = (temp > 25) ? water10 : water5;
  }

```

بر اساس مقدار رطوبت و دما، سیستم تصمیم می‌گیرد که چقدر آبیاری انجام شود.

اگر رطوبت بیشتر از ۸۰ باشد -> آبیاری انجام نشود.

اگر کمتر از ۵۰ باشد -> آبیاری زیاد انجام شود.

اگر بین این دو باشد، بسته به دما مقداری برای آبیاری انتخاب می‌شود.

```cpp
  const char* rotCmd = (lightL > lightR) ? rot0 : rot60;
```
بر اساس شدت نور در دو طرف چپ و راست، تصمیم گرفته می‌شود که دستگاه یا گیاه به کدام سمت بچرخد. اگر نور سمت چپ بیشتر باشد، نیازی به چرخش نیست ، در غیر این صورت چرخش 60 درجه انجام می‌شود.
```cpp
char waterBuf[20], rotBuf[20];
strcpy_P(waterBuf, waterCmd);
strcpy_P(rotBuf, rotCmd);
char body[100];

strcpy(body, waterBuf);
strcat(body, ";");
strcat(body, rotBuf);
strcat(body, ";");

```

دستورهای تصمیم‌گیری (آبیاری و چرخش) در قالب یک رشته با قالب مشخص (waterCmd;rotCmd;) آماده می‌شوند تا در پاسخ HTTP فرستاده شوند.

```cpp
int bodyLen = strlen(body);
char resp[256];
snprintf(resp, sizeof(resp),
"HTTP/1.0 200 OK\r\n\r\n"
"%s",
 body);
Serial.print("Resp Len: ");
int totalLen = strlen(resp);
Serial.println(totalLen);
memcpy(ether.tcpOffset(), resp, totalLen);
ether.httpServerReply(totalLen);
```
پاسخ HTTP ساخته شده و سپس با استفاده از توابع کتابخانه‌ی ether به کلاینت ارسال می‌شود.

### Edge

```cpp
#define REQUEST_RATE     3000
#define SOIL_SENSOR_PIN  A0
#define DC_MOTOR_PIN     8
#define WATER_10CC_LED   7
#define WATER_5CC_LED    3
#define SERVO_PIN        9
#define SERVO_POS_LED    4

```
در این بخش، پین‌های متصل به سنسورها، موتور آبیاری، سروو، و LED تعریف می‌شوند. همچنین فاصله‌ی زمانی بین هر درخواست HTTP مقدار ۳ ثانیه مشخص شده است.


```cpp
static byte mymac[] = {...};
static byte myip[]  = {...};
static byte gwip[]  = {...};
static byte hisip[] = {...};

byte Ethernet::buffer[1000];
static long timer;
bool ethernetInitialized = false;
int edgeId = 1;
Servo potServo;

int currentServoPos = 0;

```

در اینجا آدرس MAC و IP ها تعریف شده‌اند. همچنین یک بافر شبکه برای ارتباطات و متغیرهای مرتبط با زمان و سروو موتور نیز تنظیم شده‌اند.
شماره edge مشخص شده است و متغییری هم برای اینکه شبکه ethernet مقدار دهی شده است یا خیر.

```cpp
static void responseCallback(byte status, int off, int len) {
  if (status != 0 || len == 0) {
    Serial.print("Error status="); Serial.println(status);
    return;
  }
```

اگر status != 0 یا len == 0، یعنی مشکلی در دریافت وجود دارد، تابع متوقف می‌شود.

```cpp
String raw = String((char*)Ethernet::buffer + off);
int idx = raw.indexOf("\r\n\r\n");
if (idx < 0) {
  Serial.println("Invalid HTTP");
  return;
}
String body = raw.substring(idx + 4);

```
پاسخ HTTP شامل هدر و بدنه است. این قسمت به دنبال جداکننده‌ی هدر از بدنه را پیدا می گردد.

در صورت یافتن فقط قسمت بدنه (body) را جدا می‌کند.

اگر جداکننده پیدا نشود، پاسخ نامعتبر فرض می‌شود.

```cpp
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
```
در این قسمت فرمان آبیاری و چرخش استخراج میشود
بدنه معمولاً به شکل "WATER:10;ROTATE:60;" است.
دو ; پیدا می‌شوند و سپس بخش اول به عنوان فرمان آبیاری (waterCmd) و بخش دوم به عنوان فرمان چرخش (rotCmd) جدا می‌شوند.
اگر فرمت مطابق انتظار نباشد، از تابع خرج میشود.
```cpp
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
```
اجرای دستور آبیاری اتفاق می افتد.
اگر دستور با "WATER:" شروع شود، حجم آب (5 یا 10 یا 15) استخراج می‌شود.
موتور پمپ روشن می‌شود.
LEDهای مربوط به حجم آب روشن می‌شوند.
پس از مدتی معادل 5ms * rate، موتور و LEDها خاموش می‌شوند.

```cpp
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
```

اگر فرمان با "ROTATE:" شروع شود، زاویه استخراج می‌شود.
اگر با موقعیت فعلی فرق دارد، سروو چرخانده می‌شود.
LED موقعیت سروو هم روشن یا خاموش می‌شود (مثلاً فقط در زاویه 60 روشن باشد).

```cpp
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
```
اتصال سروو موتور به پین مشخص‌شده برای کنترل زاویه.
تنظیم پین‌های موتور پمپ و LEDها به عنوان خروجی.

```cpp
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
```
 راه‌اندازی Ethernet با آدرس MAC داده‌شده (تا 3 بار تلاش).
تنظیم IP ثابت برای گره مرزی، Gateway و سرور مرکزی.
منتظر می‌ماند تا اترنت به Gateway وصل شود.
پس از موفقیت در اتصال، پیام آماده‌بودن گره چاپ می‌شود.

```cpp
void loop() {
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);
  if (millis() - timer < REQUEST_RATE) return;
  timer = millis();
```

بررسی می‌کند که آیا بسته‌ای از شبکه (مثلاً پاسخ سرور مرکزی) دریافت شده است یا نه، و در صورت وجود، آن را پردازش می‌کند.
سپس بررسی می‌کند که از آخرین ارسال داده، آیا به اندازه‌ی کافی زمان گذشته است 
اگر هنوز زمان کافی نگذشته، تابع از این نقطه خارج می‌شود تا ارسال‌های مکرر انجام نشود.
اگر زمان کافی گذشته باشد، زمان فعلی ذخیره می‌شود تا در دفعه بعد مقایسه شود.
```cpp
int rawM = analogRead(SOIL_SENSOR_PIN);
int moisture = map(rawM, 0, 1023, 0, 100);
Serial.print("Sending moisture="); Serial.println(moisture);

char params[30];
sprintf(params, "?moisture=%d&edge=%d", moisture, edgeId);
ether.browseUrl(PSTR("/moisture"), params, PSTR("192.168.2.2"), responseCallback);
```
مقدار آنالوگ از سنسور خاک خوانده می‌شود و به درصد رطوبت بین 0 تا 100 تبدیل می‌شود.
سپس با استفاده از sprintf پارامترهای URL ساخته می‌شود که شامل مقدار رطوبت و شناسه‌ی گره است.
این پارامترها به‌صورت یک درخواست HTTP GET به سرور مرکزی ارسال می‌شوند.
اگر سرور پاسخی ارسال کند، تابع responseCallback آن را پردازش خواهد کرد.
‍‍‍
## Proteus

![image](https://github.com/user-attachments/assets/84986e89-a022-4846-8969-412503b775f2)

- در این جایگاه device های مختلف را می بینیم.

- هر یک از Edge ها به Master متصل اند.

- همچنین به هر یک از Edge ها سنسور رطوبت متصل است.

- به Master  و هر یک از Edge ها یک دیوایس enc متصل است برای ارتباط wifi

- به Master  و هر یک از Edge ها یک ترمینال برای بررسی مقادیر رد و بدل شده متصل است.

- به هر یک از edge ها نیز LED  و موتور چرخش نیز متصل است.
