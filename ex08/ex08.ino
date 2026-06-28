#include <WiFi.h>
#include <WebServer.h>

// WiFi AP 配置
const char* ap_ssid = "ESP32-安防主机";
const char* ap_pass = "12345678";

// 引脚定义
const int LED_PIN = 2;          // 板载 LED（GPIO2）
const int TOUCH_PIN = 4;       // GPIO13 作为触摸引脚（T4）

// 系统状态
bool isArmed = false;           // 是否布防
bool isAlarming = false;        // 是否报警中

// LED 闪烁控制
unsigned long lastBlink = 0;
bool ledState = false;          // 静态保持当前 LED 状态

WebServer server(80);

// ========== 生成网页（状态动态显示）==========
String makePage() {
  String status;
  if (isAlarming) {
    status = "<span style='color:red'>⚠ 报警中！</span>";
  } else if (isArmed) {
    status = "<span style='color:blue'>✅ 已布防</span>";
  } else {
    status = "<span>已撤防</span>";
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>安防系统</title>
</head>
<body style="text-align:center; margin-top:50px;">
  <h1>ESP32 报警系统</h1>
  <h2>状态：)rawliteral" + status + R"rawliteral(</h2>

  <a href="/arm"><button style="padding:15px 30px; font-size:20px;">布防</button></a>
  <a href="/disarm"><button style="padding:15px 30px; font-size:20px;">撤防</button></a>
</body>
</html>
)rawliteral";
  return html;
}

// ========== Web 请求处理 ==========
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", makePage());
}

void handleArm() {
  isArmed = true;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);   // 关闭 LED
  server.sendHeader("Location", "/");
  server.send(303);
}

void handleDisarm() {
  isArmed = false;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);   // 关闭 LED
  server.sendHeader("Location", "/");
  server.send(303);
}

// ========== 初始化 ==========
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 启动 WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);
  Serial.println("AP 已启动");
  Serial.print("IP 地址: ");
  Serial.println(WiFi.softAPIP());

  // 注册 Web 路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();
  Serial.println("Web 服务器已启动");
}

// ========== 主循环 ==========
void loop() {
  server.handleClient();

  // ----- 布防检测（使用触摸引脚）-----
  // 触摸阈值：当触摸时读数通常低于 30（无触摸时约 50~80），可根据环境微调
  const int TOUCH_THRESHOLD = 600;
  if (isArmed && !isAlarming) {
    if (touchRead(TOUCH_PIN) < TOUCH_THRESHOLD) {
      isAlarming = true;        // 触发报警，锁定状态
    }
  }

  // ----- 报警高频闪烁（50ms 切换）-----
  if (isAlarming) {
    if (millis() - lastBlink >= 50) {
      lastBlink = millis();
      ledState = !ledState;     // 翻转 LED 状态
      digitalWrite(LED_PIN, ledState);
    }
  }
}