#include <Arduino.h>
#include <Dynamixel2Arduino.h>

#define DXL_RX_PIN 2    
#define DXL_TX_PIN 4    
#define DXL_DIR_PIN 18  
#define DXL_SERIAL Serial2

const uint8_t DXL_ID = 1; 
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

int32_t last_pos = 0;
float PREDICT_FACTOR = 1.2; // 预判系数：1.0表示跟随，1.2表示稍微超前助力

void setup() {
  Serial.begin(115200);
  DXL_SERIAL.begin(57600, SERIAL_8N1, DXL_RX_PIN, DXL_TX_PIN);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);

  dxl.torqueOff(DXL_ID);
  
  // 1. 设为位置控制模式 (Mode 3)
  dxl.setOperatingMode(DXL_ID, OP_POSITION); 
  
  // 2. 【核心修改】降低位置环 P 增益 (地址 84)
  // 默认通常是 800，调到 100-200 会让电机像弹簧一样“变软”，从而能被你掰动
  dxl.writeControlTableItem(84, DXL_ID, 150); 
  
  dxl.torqueOn(DXL_ID);
  
  last_pos = dxl.getPresentPosition(DXL_ID);
  Serial.println("Position Mode Follower Started.");
}

void loop() {
  // 读取当前位置
  int32_t current_pos = dxl.getPresentPosition(DXL_ID);
  if (current_pos == -1) return;

  // 计算位移差
  int32_t diff = current_pos - last_pos;

  // 实现你的逻辑：目标 = 当前 + (当前 - 上次) * 预判
  // 这样电机就会朝着你推的方向主动移过去
  int32_t goal_pos = current_pos + (int32_t)(diff * PREDICT_FACTOR);

  if (abs(diff) > 0) {
    // 使用库推荐的函数名
    dxl.setGoalPosition(DXL_ID, goal_pos);
  }

  // 观察 Real 和 Goal 的差距
  Serial.print("Real:"); Serial.print(current_pos);
  Serial.print(" Goal:"); Serial.println(goal_pos);

  last_pos = current_pos;
  delay(10); 
}