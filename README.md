# Smart Irrigation Embedded System Using Arduino Boards

## توضیح کد ها

### Master

```cpp
void setup() {
  Serial.begin(9600); // آغاز ارتباط سریال با سرعت 9600 bps

  if (!ether.begin(sizeof Ethernet::buffer, mymac, SS)) {
    Serial.println("Ethernet init failed"); // اگر راه‌اندازی شبکه با شکست مواجه شود
  }

  ether.staticSetup(myip, gwip); // تنظیم IP به‌صورت ثابت (Static IP)
  Serial.println("Master is ready"); // اعلام آمادگی نود مستر
}
```

آماده‌سازی ماژول شبکه، تنظیم آدرس IP.

```cpp
void loop() {
  word len = ether.packetReceive();  // دریافت بسته‌های شبکه
  word pos = ether.packetLoop(len);  // بررسی بسته‌ها برای درخواست‌های TCP

  if (pos) {
    char *data = (char *)Ethernet::buffer + pos;  // اشاره‌گر به محتوای درخواست
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
        response = "START_WATERING"; // اگر رطوبت کم بود، فرمان آبیاری ارسال می‌شود
      else
        response = "NO_WATER"; // در غیر این صورت، نیاز به آبیاری نیست

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
