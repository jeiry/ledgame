#include "ArduinoJson.h"

Ticker mqttTicker;

DynamicJsonDocument doc(1024); //内存池
char* mq_name = (char*)"";
char* mq_pwd = (char*)"";
long mqttPort = 1883;

/**
 * mqtt指令回调
 */
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("收到mqtt传过来的消息 [");
  Serial.print(topic);
  Serial.print("] ");
  String str;
  for (int i = 0; i < length; i++) {
    str += String((char)payload[i]);
  }
  Serial.println();
  Serial.print(str);

  //对传过来的消息进行json解析
  auto error = deserializeJson(doc, str); //解析消息
  if (error) { //检查解析中的错误
    Serial.println("Json 解析错误"+String(error.c_str()));
    return;
  }

  const char* optType = doc["OptType"];//操作类型
  const char* command = doc["Command"];//操作指令
  const char* extData = doc["ExtData"];//扩展函数

  // 异步执行指令
  mqttTicker.once_ms(10,[optType,command,extData,str]() {
    runMQTTCommand(optType,command,extData,str);
  });

  
}




/**
 * 写入指令到mqtt
 */
void writeCommandMqtt(char* command,int len){
    Serial.print("写入指令到mqtt: "+String(command));
    client.publish("outTopic", command,len);
}

void writeCommandMqtt(const char* command){
    Serial.print("写入指令到mqtt: "+String(command));
    client.publish("outTopic", command);
}


/**
 * mqtt重连
 */
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    onMqttConnect();
    // Create a random client ID
    
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), mq_name, mq_pwd)) {
      Serial.println("connected");
      // 发送一条连接成功的广播
      String message = "connect success:"+clientId;
      client.publish(subTopic.c_str(), message.c_str());
      //订阅主题
      
      client.subscribe(inTopic.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 5秒后重试
      delay(5000);
    }
  }
  //连接成功
  onMqttConnectSuccess();
 }

void onMqttConnect(){
  showLight(5);//6闪
  Serial.println("正在连接Mqtt");
}

void onMqttConnectSuccess(){
  showLight(4);//常亮
  Serial.println("Mqtt连接成功");
}

  /**
   * 初始化mqtt
   */
  void initMqtt(char* mqtt_server,char* name,char* pwd){
    mq_name=name;
    mq_pwd=pwd;
    Serial.println("mqtt");
    Serial.println(mqtt_server);
    Serial.println(mqttPort);
    Serial.println(mq_name);
    Serial.println(mq_pwd);
    client.setServer(mqtt_server, mqttPort);
    client.setCallback(callback);  
  }
  /**
   * 连接mqtt
   */
  void startMqtt(){
    //重连mqtt
    if (!client.connected()) {
      reconnect();
    }
    client.loop();  
  }
