#include <math.h>
const int ledPin = 2;  
const int freq = 5000;          
const int resolution = 8;

void setup() {
  Serial.begin(115200);
  ledcAttach(ledPin, freq, resolution);
}

void loop() {
  // 渐亮：归一化0~1，指数校正
  for(float x=0;x<=1;x+=0.003f){
    int duty = pow(x,2.2f)*255;
    ledcWrite(ledPin,duty);
    delay(8);
  }
  // 渐暗
  for(float x=1;x>=0;x-=0.003f){
    int duty = pow(x,2.2f)*255;
    ledcWrite(ledPin,duty);
    delay(8);
  }
  Serial.println("指数呼吸周期完成");
}