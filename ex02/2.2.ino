// 定义板载LED引脚，避免使用"魔法数字"
#define LED_PIN 2

// millis计时相关变量，必须用unsigned long
unsigned long previousMillis = 0;
// 闪烁周期间隔：1Hz完整周期1000ms
const unsigned long blinkInterval = 1000;
// LED状态标记
bool ledState = LOW;

void setup() {
  // 初始化串口通信
  Serial.begin(115200);
  // 初始化板载LED引脚为输出模式
  pinMode(LED_PIN, OUTPUT); 
}

void loop() {
  // 获取当前系统运行毫秒数
  unsigned long currentMillis = millis();

  // 判断是否到达切换LED状态的时间
  if (currentMillis - previousMillis >= blinkInterval) {
    // 更新时间戳
    previousMillis = currentMillis;
    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }

  // 串口打印不会被闪烁阻塞，持续输出
  Serial.println("Hello ESP32!");
}