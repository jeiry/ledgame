#include <ESP8266WiFi.h>;
#include <ArduinoJson.h>
#include "FS.h"
#include <LittleFS.h>

Ticker checkWifiTicker;
boolean runSuccess = true;
boolean checkWifiStatus = true;
long connectionTime = 0;
char* sKeys = "wifiStatus\0";
boolean wifiInitStatus = false;

/**
   读取配置文件
*/
const char* loadConfig(char *key) {
  File configFile = LittleFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println("Failed to open config file");
    return "";
  }

  size_t size = configFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return "";
  }

  std::unique_ptr<char[]> buf(new char[size]);

  configFile.readBytes(buf.get(), size);

  StaticJsonDocument<200> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println("Failed to parse config file");
    return "";
  }
  configFile.close();
  return doc[key];
}

/**
   写入配置文件
*/
bool saveConfig(char *key, char *value) {
  StaticJsonDocument<200> doc;
  doc[key] = value;
  //  doc["serverName"] = "api.example.com";
  //  doc["accessToken"] = "128du9as8du12eoue8da98h123ueh9h98";

  File configFile = LittleFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  serializeJson(doc, configFile);
  delay(200);
  configFile.close();
  return true;
}



/**
   初始化wifi模块
*/
void initWifi() {
  Serial.println("wifiInitStatus");
  Serial.println(wifiInitStatus);
  if (!wifiInitStatus) {
    //初始化wifi模块
    WiFi.begin();
    wifiInitStatus = true;
  }
}


/**
   重置wifi信息
*/
void resetWifiConfig() {
  Serial.println("正在智能wifi配置,请稍等...");
  delay(1000);
  releaseStorage();
  delay(1000);
  ESP.restart();  //复位esp32
}
/**
   启动wifi
*/
void startWifi() {
  Serial.print("startWifi");
  initWifi();
  const char* value = loadConfig("smart_config");
  Serial.print("smart_config: ");
  Serial.println(value);
  Serial.println(String(value) == "true");
  //自动连接wifi
  if (!readStorage()) {
    Serial.print("开始智能配网 ");
    // 启动智能配网
    SmartConfig();
  } else if (!AutoConfig())
  {
    connectTimeOut();
  }

  //检查网络连接状态
  Serial.println("正在检查wifi连接状态1");
  runSuccess = true;
  checkWifiStatus = true;
  checkWifiTicker.attach_ms(1000, checkWifiConnect);
}

/**
   检查网络连接状态
*/
void checkWifiConnect() {
  if (checkWifiStatus) {
    Serial.println("正在检查wifi连接状态");
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED)
    {
      //连接成功
      Serial.println("WIFI 连接成功");
      connectionTime = 0;
      if (runSuccess) {
        runSuccess = false;
        // 关闭定时
        checkWifiTicker.detach();
        connectSuccess();
      }
    } else { //连接失败
      runSuccess = true;
      //1秒检查一次
      connectionTime++;
      if (connectionTime > 60) {
        // 关闭定时
        checkWifiTicker.detach();
        connectionTime = 0;
        connectTimeOut();
      } else {
        //连接失败
        connectError();
      }

    }
  }

}

/**
   读取数据
*/
boolean readStorage() {
  const char* wifiStatus = loadConfig("smart_config");
  Serial.println("");
  Serial.println(wifiStatus);
  if (String(wifiStatus) == "true") {
    return true;
  }
  return false;
}

/**
   写入数据
*/
void writeStorage(char *wifiStatus) {
  saveConfig("smart_config", wifiStatus);
  
}

/**
   清除存储的数据
*/
void releaseStorage() {
  char* str1 = "false";
  writeStorage(str1);
}



/**
   启动自动配置wifi功能
*/
void SmartConfig()
{
  checkWifiStatus = false;
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig...");
  WiFi.beginSmartConfig();
  onSmartConfigIng();
  while (1)
  {
    Serial.print(".");
    delay(500);                   // wait for a second
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      checkWifiStatus = true;
      writeStorage("true");//存储wifi标记
      //连接成功
      if (WiFi.status() == WL_CONNECTED) {
        delay(1000);
        return;
      }
      break;
    }

  }
}

/**
   wifi连接成功回调
*/
void connectSuccess() {
  Serial.println("Wifi Connect Success");
  closeLight();// 关灯
}

/**
   wifi连接失败回调
*/
void connectError() {
  Serial.println("Wifi Connect Error");
  showLight(3);// 慢闪
}
/**
   连接超时回调
*/
void connectTimeOut() {
  Serial.println("Wifi connect TimeOut");
  showLight(3);// 慢闪
}
/**
   wifi连接中
*/
void onConnection() {
  showLight(2);// 双闪
}
/**
   智能配网中
*/
void onSmartConfigIng() {
  showLight(1);//led快闪
}

/**
   检测wifi状态,如果超时则启动wifi配置
*/
bool AutoConfig()
{
  onConnection();
  //如果觉得时间太长可改
  for (int i = 0; i < 10; i++)
  {
    int wstatus = WiFi.status();
    if (wstatus == WL_CONNECTED)
    {
      Serial.println("WIFI SmartConfig Success");
      Serial.printf("SSID:%s", WiFi.SSID().c_str());
      Serial.printf(", PSW:%s\r\n", WiFi.psk().c_str());
      Serial.print("LocalIP:");
      Serial.print(WiFi.localIP());
      Serial.print(" ,GateIP:");
      Serial.println(WiFi.gatewayIP());
      return true;
    }
    else
    {
      Serial.print("WIFI AutoConfig Waiting......");
      Serial.println(wstatus);
      delay(1000);
    }
  }
  Serial.println("WIFI AutoConfig Faild!" );
  return false;
}

/**
 * 初始化文件系统
 */
void initFileSystem(){
//  LittleFS.format();
  Serial.println("Mount LittleFS");
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }  
}
