// 定义LED引脚，ESP32通常板载LED连接在GPIO 2
const int ledPin = 2;

// 时间常量，统一管理时长（单位ms）
const unsigned long DOT_ON = 200;    // 短闪点亮时长
const unsigned long DASH_ON = 600;   // 长闪点亮时长
const unsigned long DOT_GAP = 200;   // 点/划之间熄灭间隔
const unsigned long CHAR_GAP = 500;   // 字母(S/O/S)之间间隔
const unsigned long SOS_GAP = 2000;   // 整套SOS结束后的长停顿

// millis计时变量
unsigned long previousTime = 0;
// LED当前状态
bool ledState = LOW;
// 状态机标记：区分当前执行到SOS哪一步
enum State {
    S_DOT,      // S的3个短点
    CHAR_S_O,   // S与O之间间隔
    O_DASH,     // O的3个长划
    CHAR_O_S,   // O与最后S之间间隔
    S_DOT_END,  // 最后一组S的3个短点
    SOS_WAIT    // 整套SOS结束，长等待
} currentState = S_DOT;

// 计数器：记录当前点/划闪烁次数
int flashCount = 0;

void setup() {
  // 初始化串口通信，设置波特率为115200
  Serial.begin(115200);
  // 将LED引脚设置为输出模式
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  unsigned long currentTime = millis();

  // 状态机主逻辑
  switch (currentState)
  {
    // 第一段：S 3次短闪
    case S_DOT:
      if (currentTime - previousTime >= (ledState ? DOT_ON : DOT_GAP))
      {
        previousTime = currentTime;
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        // 熄灭时计数+1，完成一次短闪
        if (!ledState)
        {
          flashCount++;
          // 3次短闪完成，切换到字母间隔
          if (flashCount >= 3)
          {
            flashCount = 0;
            ledState = LOW;
            digitalWrite(ledPin, LOW);
            currentState = CHAR_S_O;
            previousTime = currentTime;
          }
        }
      }
      break;

    // S 和 O 之间500ms间隔
    case CHAR_S_O:
      if (currentTime - previousTime >= CHAR_GAP)
      {
        previousTime = currentTime;
        currentState = O_DASH;
      }
      break;

    // 第二段：O 3次长闪
    case O_DASH:
      if (currentTime - previousTime >= (ledState ? DASH_ON : DOT_GAP))
      {
        previousTime = currentTime;
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        if (!ledState)
        {
          flashCount++;
          if (flashCount >= 3)
          {
            flashCount = 0;
            ledState = LOW;
            digitalWrite(ledPin, LOW);
            currentState = CHAR_O_S;
            previousTime = currentTime;
          }
        }
      }
      break;

    // O 和 最后S 之间500ms间隔
    case CHAR_O_S:
      if (currentTime - previousTime >= CHAR_GAP)
      {
        previousTime = currentTime;
        currentState = S_DOT_END;
      }
      break;

    // 第三段：末尾S 3次短闪
    case S_DOT_END:
      if (currentTime - previousTime >= (ledState ? DOT_ON : DOT_GAP))
      {
        previousTime = currentTime;
        ledState = !ledState;
        digitalWrite(ledPin, ledState);
        if (!ledState)
        {
          flashCount++;
          if (flashCount >= 3)
          {
            flashCount = 0;
            ledState = LOW;
            digitalWrite(ledPin, LOW);
            currentState = SOS_WAIT;
            previousTime = currentTime;
          }
        }
      }
      break;

    // 整套SOS完成，2000ms长停顿，之后重新循环
    case SOS_WAIT:
      if (currentTime - previousTime >= SOS_GAP)
      {
        previousTime = currentTime;
        currentState = S_DOT;
      }
      break;
  }

  // 可并行执行其他代码，不会被灯光阻塞
  Serial.println("系统运行中，非阻塞SOS输出");
}