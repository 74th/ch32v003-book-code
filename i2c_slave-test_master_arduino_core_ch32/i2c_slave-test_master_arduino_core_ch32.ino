#include "Wire.h"

#define SHT31_I2C_ADDR 0x70

void setup() {
  Serial.begin(115200);

  // not working - 動作を確認できなかった
  // USART_Printf_Init(115200);

  Serial.println("init");
  // printf("init\r\n");

  Wire.begin();

  Serial.println("start");
  // printf("start\r\n");
}

uint32_t count = 0;

void loop() {
  Serial.print("loop ");
  Serial.println(count++);

  // --- 0x10-0x13に書き込み ---
  uint8_t send_data[5] = { 0 };
  send_data[0] = 0x10;
  send_data[1] = count;
  send_data[2] = count + 1;
  send_data[3] = count + 2;
  send_data[4] = count + 3;

  Wire.beginTransmission(SHT31_I2C_ADDR);

  // データ書き込み
  Wire.write(send_data, 5);

  Wire.endTransmission();

  Serial.println("write");

  delay(100);

  // --- 0x20-0x23を読み込み ---
  uint8_t receive_data[4] = { 0 };
  uint8_t receive_available = 0;

  // 読み込みレジスタアドレスを最初に送る
  Wire.beginTransmission(SHT31_I2C_ADDR);
  // Wire.write(0x20);
  // delay(10);
  // データ読み込み
  Wire.requestFrom(SHT31_I2C_ADDR, 4, 0x20, 0, true);
  receive_available = Wire.available();
  if (3 <= receive_available) {
    for (int i = 0; i < 4; i++) {
      receive_data[i] = Wire.read();
    }
  }
  Wire.endTransmission();

  Serial.print("receive available: ");
  Serial.println(receive_available);
  // 読み込みデータをプリント
  if(receive_available == 0 ){
    Serial.println("receive failed");
    return;
  }
  Serial.print("read :");
  for (int i = 0; i < 4; i++) {
    Serial.print(receive_data[i]);
    if (i < 3) {
      Serial.print(" ");
    } else {
      Serial.println("");
    }
  }

  delay(1000);
}
