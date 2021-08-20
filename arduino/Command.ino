/**
   MQTT指令执行操作类
*/
#include "Ticker.h"
#define D0 12
#define D1 5
#define D4 14
Ticker lazyTicker;
Ticker deviceStatusCheckTicker;
int num = 0;
int statusNum = 0;

/**
   执行MQTT操作指令
*/
void runMQTTCommand(const char* optType, const char* command, const char* extData, String str) {
  Serial.println(optType);
  Serial.println(command);
  Serial.println(extData);
  Serial.println(str);
  if (String(optType) == "rfidOption") {
    //计算串口指令长度
    int commandLen = getlen(command);
    char commandCache[commandLen];
    //字符串指令转16进制指令
    StrToHex(commandCache, command, commandLen);

    //收到mqtt指令
    Serial.print("收到mqtt下发的指令: " + String(command) + " 长度 " + commandLen);
    //将收到的串口指令下发给串口
    //      mySerialPort.writeData(commandCache,commandLen);

    // 远程电源按键
  } else if (String(optType) == "gameStart") {
    Serial.println("开始游戏" + String(extData));
    gameStart(atoi(extData));
  } else {
    Serial.print("指令错误: " + str);
  }
}

/**
   初始化指定gpio引脚
*/
void initGPIOPin() {
  // 设置D1引脚为输出引脚,用于开关控制
//  pinMode(D1, OUTPUT);
//  pinMode(D0, OUTPUT);
}

/**
   延时开关
*/
void gameStart(int t) {
  if (isStart == 0 and t == 1) {
    //      如果未开始并传入的是1就开始
    isStart = t;
  } else if (isStart == 2 and t == 3) {
    //      如果已经开始并传入的是3就跑结果
    isStart = t;
  }
}

/**
   设备开机状态检测
*/
void deviceRunningCheck() {
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U,  D4);  // GPIO_4设为IO口
  GPIO_DIS_OUTPUT(GPIO_ID_PIN(D4));            // GPIO_0失能输出(默认)
//  PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO4_U);         // 开启内部上拉
  GPIO_OUTPUT_SET(GPIO_ID_PIN(D4), LOW); //设置gpio4为低电平

  // 每隔500毫秒检测一次当前设备的开机状态
  deviceStatusCheckTicker.attach_ms(500, []() {
    num++;
    if (num > 10) {
      const char* deviceStat = "sleep";
      // 周期性调用一次状态判断
      if (statusNum >= 10) {
        deviceStat = "running";
      }

      //清除计数器
      num = 1;
      statusNum = 0;
      //通过mqtt通知客户端设备已经在线
      String message = "{\"OptType\":\"DeviceStatus\",\"Command\":\""+String(deviceStat)+"\",\"ExtData\":\"\"}";
      writeCommandMqtt(message.c_str());
      Serial.println("当前设备状态: " + String(deviceStat));
    }
    Serial.println(GPIO_INPUT_GET(GPIO_ID_PIN(D4)));
    // 当前引脚为低电平,证明电源灯不亮
    if (!GPIO_INPUT_GET(GPIO_ID_PIN(D4))) {
      statusNum -= 1;
      if (statusNum < 1) {
        statusNum = 1;
      }
      Serial.println("off");
    } else {
      statusNum += 1;
      Serial.println("on");
//      GPIO_OUTPUT_SET(GPIO_ID_PIN(D4), 1); //复位高电平
    }
  });
}


/*
  // C prototype : void StrToHex(byte *pbDest, char *pszSrc, int nLen)
  // parameter(s): [OUT] pbDest - 输出缓冲区
  //  [IN] pszSrc - 字符串
  //  [IN] nLen - 16进制数的字节数(字符串的长度/2)
  // return value:
  // remarks : 将字符串转化为16进制数
*/
void StrToHex(char *pbDest, const char *pszSrc, int nLen)
{
  char h1, h2;
  char s1, s2;
  for (int i = 0; i < nLen; i++)
  {
    h1 = pszSrc[2 * i];
    h2 = pszSrc[2 * i + 1];

    s1 = toupper(h1) - 0x30;
    if (s1 > 9)
      s1 -= 7;

    s2 = toupper(h2) - 0x30;
    if (s2 > 9)
      s2 -= 7;

    pbDest[i] = s1 * 16 + s2;
  }
}

/**
   获取char类型长度
*/
int getlen(const char *result) {
  int i = 0;
  while (result[i]) {
    i++;
  }
  return i / 2;
}

int getlen(char *result) {
  int i = 0;
  while (result[i]) {
    i++;
  }
  return i / 2;
}
