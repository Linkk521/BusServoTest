#include <Arduino.h>
#include <Dynamixel2Arduino.h>

#define DXL_RX_PIN 2    
#define DXL_TX_PIN 4    
#define DXL_DIR_PIN 18  
#define DXL_SERIAL Serial2

const uint8_t DXL_ID = 1; 
Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

void setup() {
  Serial.begin(115200);
  DXL_SERIAL.begin(57600, SERIAL_8N1, DXL_RX_PIN, DXL_TX_PIN);
  dxl.begin(57600);
  dxl.setPortProtocolVersion(2.0);

  // 1. 关扭矩
  dxl.torqueOff(DXL_ID);

  // 2. 强制切换到单圈位置模式 (Mode 3)，这会清除之前累加的数千甚至上万的数值
  dxl.writeControlTableItem(11, DXL_ID, 3); 
  delay(200);

  // 3. 检查模式是否切换成功
  uint8_t mode = dxl.readControlTableItem(11, DXL_ID);
  Serial.print("Current Mode (Should be 3): ");
  Serial.println(mode);

  Serial.println("Ready to read 0-4095 raw data...");
}

void loop() {
  // 强制读取当前的绝对位置 (4字节)
  int32_t raw_pos = dxl.getPresentPosition(DXL_ID);

  // 针对 XC430 的单圈掩码处理，确保数值被锁定在 0-4095 之间
  // 如果你的数值还是很大，说明模式切换没成功，或者固件在以多圈模式运行
  uint32_t clean_pos = (uint32_t)raw_pos % 4096; 

  if (raw_pos != -1) {
    float angle = (float)clean_pos * 360.0 / 4096.0;

    Serial.print("Raw:"); Serial.print(raw_pos);
    Serial.print(" | Cleaned:"); Serial.print(clean_pos);
    Serial.print(" | Angle:"); Serial.print(angle);
    Serial.println(" deg");
  }

  delay(100); 
}