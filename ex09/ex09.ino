#include <WiFi.h>
#include <WebServer.h>

// ===== WiFi AP 配置 =====
const char* ap_ssid = "ESP32-Dashboard";
const char* ap_pass = "12345678";

// ===== 引脚定义 =====
const int LED_PIN = 2;          // 板载 LED (GPIO2)
const int TOUCH_PIN = T0;       // GPIO4 触摸引脚 (T0)

// ===== 系统状态 =====
bool isArmed = false;
bool isAlarming = false;

// ===== LED 闪烁控制 =====
unsigned long lastBlink = 0;
bool ledState = false;

WebServer server(80);

// ========== 生成仪表盘 HTML（含 AJAX） ==========
String getDashboardPage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 触摸仪表盘</title>
  <style>
    * { box-sizing: border-box; }
    body {
      text-align: center;
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      margin: 0;
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      background: #f0f2f5;
    }
    .container {
      max-width: 500px;
      width: 90%;
      padding: 30px 20px;
      background: white;
      border-radius: 28px;
      box-shadow: 0 15px 35px rgba(0,0,0,0.1);
    }
    h1 {
      font-size: 26px;
      color: #0d6efd;
      margin-bottom: 5px;
    }
    .subtitle {
      font-size: 14px;
      color: #6c757d;
      margin-bottom: 20px;
    }
    .status {
      font-size: 20px;
      padding: 8px 20px;
      background: #e9ecef;
      border-radius: 30px;
      display: inline-block;
      margin: 10px 0 20px;
      font-weight: 500;
    }
    .status.alarm {
      background: #f8d7da;
      color: #dc3545;
      animation: blink 0.6s infinite;
    }
    @keyframes blink {
      0% { opacity: 1; }
      50% { opacity: 0.3; }
      100% { opacity: 1; }
    }
    .touch-value {
      font-size: 90px;
      font-weight: 700;
      color: #0d6efd;
      background: #e3f2fd;
      border-radius: 20px;
      padding: 20px 0;
      margin: 10px 0;
      transition: 0.1s;
    }
    .label {
      font-size: 16px;
      color: #6c757d;
      margin-top: -5px;
      margin-bottom: 20px;
    }
    .button-group {
      display: flex;
      justify-content: center;
      gap: 15px;
      flex-wrap: wrap;
      margin: 20px 0 10px;
    }
    button {
      padding: 14px 35px;
      font-size: 18px;
      font-weight: 600;
      border: none;
      border-radius: 50px;
      cursor: pointer;
      transition: 0.2s;
      color: white;
      flex: 1 1 auto;
      min-width: 120px;
    }
    .arm-btn {
      background: #0d6efd;
      box-shadow: 0 4px 10px rgba(13,110,253,0.3);
    }
    .arm-btn:hover { background: #0b5ed7; transform: scale(1.02); }
    .disarm-btn {
      background: #dc3545;
      box-shadow: 0 4px 10px rgba(220,53,69,0.3);
    }
    .disarm-btn:hover { background: #bb2d3b; transform: scale(1.02); }
    .footer {
      margin-top: 20px;
      color: #adb5bd;
      font-size: 13px;
    }
  </style>
</head>
<body>
<div class="container">
  <h1>📡 触摸传感器</h1>
  <div class="subtitle">实时数据监控</div>
  
  <div class="status" id="statusDisplay">⏳ 待机</div>
  
  <div class="touch-value" id="touchValue">0</div>
  <div class="label">触摸读数</div>

  <div class="button-group">
    <button class="arm-btn" onclick="arm()">🔒 布防</button>
    <button class="disarm-btn" onclick="disarm()">🔓 撤防</button>
  </div>
  <div class="footer">刷新间隔 100ms · 引脚 GPIO4 (T0)</div>
</div>

<script>
  // ===== AJAX 数据拉取 =====
  function updateTouch() {
    fetch('/data')
      .then(res => res.text())
      .then(val => {
        document.getElementById('touchValue').innerText = val;
      })
      .catch(err => console.warn('数据获取失败'));
  }

  function updateStatus() {
    fetch('/status')
      .then(res => res.text())
      .then(s => {
        const el = document.getElementById('statusDisplay');
        el.innerText = s;
        if (s.includes('报警')) {
          el.className = 'status alarm';
        } else {
          el.className = 'status';
        }
      })
      .catch(err => console.warn('状态获取失败'));
  }

  // 定时刷新
  setInterval(updateTouch, 100);
  setInterval(updateStatus, 300);

  // 控制函数
  function arm()   { fetch('/arm'); }
  function disarm(){ fetch('/disarm'); }

  // 页面加载后立即更新一次
  window.onload = function() {
    updateTouch();
    updateStatus();
  };
</script>
</body>
</html>
  )rawliteral";
  return html;
}

// ========== Web 请求处理 ==========
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", getDashboardPage());
}

// 返回触摸原始数值
void handleTouchData() {
  int val = touchRead(TOUCH_PIN);
  server.send(200, "text/plain", String(val));
}

// 返回系统状态字符串
void handleStatus() {
  String s;
  if (isAlarming)      s = "🚨 报警中！";
  else if (isArmed)    s = "✅ 已布防";
  else                 s = "⏳ 待机";
  server.send(200, "text/plain", s);
}

// 布防
void handleArm() {
  isArmed = true;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

// 撤防
void handleDisarm() {
  isArmed = false;
  isAlarming = false;
  digitalWrite(LED_PIN, LOW);
  server.send(200, "text/plain", "OK");
}

// ========== 初始化 ==========
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 启动 WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_pass);

  Serial.println("===== ESP32 仪表盘已启动 =====");
  Serial.print("WiFi 热点: "); Serial.println(ap_ssid);
  Serial.print("密码: "); Serial.println(ap_pass);
  Serial.print("访问地址: http://"); Serial.println(WiFi.softAPIP());

  // 注册 Web 路由
  server.on("/", handleRoot);
  server.on("/data", handleTouchData);
  server.on("/status", handleStatus);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();
  Serial.println("Web 服务器运行中");
}

// ========== 主循环 ==========
void loop() {
  server.handleClient();

  // ----- 布防检测（触摸触发报警）-----
  // 阈值 40：无触摸时约 50~80，触摸后降至 30 以下
  const int TOUCH_THRESHOLD = 600;
  if (isArmed && !isAlarming) {
    int val = touchRead(TOUCH_PIN);
    if (val < TOUCH_THRESHOLD) {
      isAlarming = true;
    }
  }

  // ----- 报警高频闪烁（30ms 切换）-----
  if (isAlarming) {
    if (millis() - lastBlink >= 30) {
      lastBlink = millis();
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
  }
}