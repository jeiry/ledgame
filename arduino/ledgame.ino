#include <ESP8266WiFi.h>
#include "Ticker.h"
char* mqtt_server = (char*)"t.xxx.com";
char* mqtt_name = (char*)"xxx";
char* mqtt_pwd = (char*)"xxx";

#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);
String clientId = "ESP8266Client-";
String inTopic = "";
String subTopic = "";

//wifi复位  d3
#include "OneButton.h"
#define PIN_INPUT D3
Ticker buttonTicker;
OneButton button(PIN_INPUT, true);

#include "FS.h"
#include <LittleFS.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#define LED_PIN   4
#define LED_COUNT 16
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);


#define PIN_LED BUILTIN_LED
int relayInput = PIN_LED;
boolean light = false;

int isStart = 0;


/**
   双击
*/
void doubleclick()
{
  Serial.println("doubleclick");
}

/**
   短按
*/
void click()
{
  if(isStart == 2){
    isStart = 3;
  }
  Serial.println("click");
}

/**
   长按
*/
void longclick()
{
  Serial.println("longclick");
  //启动智能配网
  resetWifiConfig();
}

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.setBrightness(255);
  pinMode(relayInput, OUTPUT);
  // 启动灯光检查
  LED_Task();
  // 初始化按钮
  button.attachClick(click);
  button.attachDoubleClick(doubleclick);
  button.attachLongPressStart(longclick);
  button.setPressTicks(5000);//设置长按时间为5秒
  //初始化LittleFS文件系统
  initFileSystem();
  //启动网络连接
  Serial.println("启动WIFI连接");
  delay(1000);
  clientId = "ESP8266Client-";
  uint64_t chipid =ESP.getChipId();
  clientId += String((uint32_t)chipid);
  inTopic = "/home/r/ledgame/"+clientId;
  subTopic = "/home/s/ledgame/"+clientId;
  //50毫秒检查一次按键
  buttonTicker.attach_ms(100, []() {
    button.tick();
  });
  delay(500);
  Serial.printf("%08X\n",(uint32_t)chipid);//print Low 4bytes.
  Serial.println(inTopic);
  //启动wifi
  startWifi();

  Serial.println("初始化mqtt服务器");
  //初始化mqtt服务器
  initMqtt(mqtt_server, mqtt_name, mqtt_pwd);
}

int lastLed = 0;
int countLed = 8;
int delayInt = 50;
void loop() {

  //启动mqtt服务
  if (WiFi.status() == WL_CONNECTED) {
  // 读取串口数据
    startMqtt();
  }
  

  if (isStart == 1) {
    //    随机开始位
    lastLed = random(0, 7);
    isStart = 2;
  }
  if (isStart == 2) {
    //    一直循环
    delayInt = 50;
    for (int i = 0; i < 8; i++) {
      strip.clear();
      int r = random(10, 255);
      int g = random(10, 255);
      int b = random(10, 255);
      strip.setPixelColor(lastLed + lastLed, strip.Color(r, g, b));
      strip.setPixelColor(lastLed + lastLed + 1, strip.Color(r, g, b));
      strip.show();
      lastLed ++;
      lastLed = lastLed > 7 ? 0 : lastLed;
      Serial.println(lastLed);
      delay(delayInt);
    }
  }
  if (isStart == 3) {
    //    收到停止信号
    for (int x = 0; x <= 7; x++) {
      //      循环7次慢慢变慢
      countLed = x < 7 ? 8 : random(0, 8);
      //      到了最后一次时，再加一次随机数
      for (int i = 0; i < countLed; i++) {
        strip.clear();
        int r = random(10, 255);
        int g = random(10, 255);
        int b = random(10, 255);
        strip.setPixelColor(lastLed + lastLed, strip.Color(r, g, b));
        strip.setPixelColor(lastLed + lastLed + 1, strip.Color(r, g, b));
        strip.show();
        lastLed ++;
        lastLed = lastLed > 7 ? 0 : lastLed;
        Serial.println(lastLed);
        //        这里就是最后停止的位置
        delay(delayInt);
      }
      if (x > 1) {
        delayInt = delayInt + 50;
      }
    }
    String str = String(lastLed);

    client.publish(subTopic.c_str(), str.c_str());
    Serial.println("----");
    Serial.println(countLed);
    countLed = 8;
    isStart = 0;
    //    复位
  }

}
