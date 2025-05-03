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

  if (pos) {
    char *data = (char *)Ethernet::buffer + pos; 
```

دریافت یک بسته از شبکه و بررسی بسته دریافتی و چک کردن اینکه آیا یک درخواست HTTP است یا نه. اگر هست، مکان شروع داده‌های آن را برمی‌گرداند.


```cpp
if (strstr(data, "moisture=")) {
      char* moistPtr = strstr(data, "moisture=");
      int moisture = atoi(moistPtr + 9);
```
اگر در داده‌ها کلمه moisture= وجود داشته باشد، یعنی این یک درخواست سنسور است که مقدار رطوبت را در URL فرستاده است.
و سپس تبدیل مقدار رطوبت از رشته به عدد صحیح اتفاق می افتد

```cpp
      if (moisture < 50)
        response = "START_WATERING"; 
      else
        response = "NO_WATER"; 

      ether.httpServerReply(strlen(response));
      memcpy(ether.tcpOffset(), response, strlen(response));
```

   پاسخ متنی به کلاینت (مثلاً نود سنسور) ارسال می‌شود تا مشخص کند باید آبیاری انجام شود یا نه که این به مقدار moisture بستگی دارد.
```cpp
else {
      memcpy_P(ether.tcpOffset(), welcomePage, sizeof welcomePage);
      ether.httpServerReply(sizeof welcomePage - 1);
    }
```

اگر هیچ داده‌ای مثل moisture= در URL نبود، فرض می‌شود که یک مرورگر وارد شده است، و صفحه‌ی HTML به او نمایش داده می‌شود.

### Edge1

```cpp
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x32 };
static byte myip[] = { 192,168,2,3 };
static byte gwip[] = { 192,168,2,1 };
static byte hisip[] = { 192,168,2,2 };

byte Ethernet::buffer[1000];
static long timer;
bool ethernetInitialized = false;

#define SOIL_SENSOR_PIN A0
int soil_moisture;
```

تنظیم مک و IP نود، مشخص کردن آدرس سرور (نود اصلی)، اندازه‌ی بافر شبکه، پایه سنسور رطوبت، و متغیرهای زمان‌بندی.


```cpp
static void my_result_cb(byte status, word off, word len) {
  Serial.print("<<< reply ");
  Serial.print(millis() - timer);
  Serial.println(" ms");
  Serial.println((const char*) Ethernet::buffer + off);
}
```

بعد از اینکه داده به سرور فرستاده شد، این تابع پاسخ دریافتی (مثلاً "START_WATERING") رو چاپ می‌کنه.

```cpp
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
```

راه‌اندازی ماژول شبکه، تلاش برای برقراری ارتباط تا ۵ بار، چاپ آدرس سرور، و صبر تا گیت‌وی پیدا بشه.

```cpp
void loop() {
  ether.packetLoop(ether.packetReceive());

  if (millis() > timer + REQUEST_RATE) {
    timer = millis();
    int rawMoisture = analogRead(SOIL_SENSOR_PIN);
    soil_moisture = map(rawMoisture, 0, 1023, 0, 100);

    Serial.print(F("Sending moisture: "));
    Serial.println(soil_moisture);

    char params[25];
    sprintf(params, "?moisture=%d&edge=1", soil_moisture);

    ether.browseUrl(PSTR("/moisture"), params, PSTR("192.168.2.2"), my_result_cb);
  }
}
```

هر ۵ ثانیه مقدار رطوبت خاک خونده می‌شه به درصد تبدیل می‌شه و در قالب URL به سرور ارسال می‌شه.
