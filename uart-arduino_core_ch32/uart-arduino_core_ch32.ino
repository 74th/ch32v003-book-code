#include <Wire.h>
#include <oled.h>

uint8_t CMD_READ_CO2_CONNECTION[9] = { 0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79 };
uint8_t CMD_TURN_ON_SELF_CALIBRATION[9] = { 0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6 };

OLED oled = OLED(PC1, PC2, NO_RESET_PIN, OLED::W_128, OLED::H_32, OLED::CTRL_SSD1306, 0x3c);

void setup() {
  // UART初期化
  Serial.begin(9600);

  // 自動キャリブレーションをオン
  Serial.write(CMD_TURN_ON_SELF_CALIBRATION, sizeof CMD_TURN_ON_SELF_CALIBRATION);

  delay(1);

  // キャリブレーション命令の応答
  uint8_t read_buf[9] = { 0 };
  Serial.readBytes(read_buf, sizeof(read_buf));

  // OLED初期化
  oled.begin();

  delay(1000);
}

uint32_t loop_count = 0;

void loop() {
  loop_count++;

  uint8_t read_buf[9] = { 0 };

  // CO2読み取り
  Serial.write(CMD_READ_CO2_CONNECTION, sizeof(CMD_READ_CO2_CONNECTION));
  // Serial.readBytes() にはタイムアウト機能が付いており、常にこれを使う
  uint32_t len = Serial.readBytes(read_buf, sizeof(read_buf));
  if(len<9)
  {
    // タイムアウト
    oled.clear();
    oled.setCursor(0, 0);
    oled.printf(6, 8, "loop: %5d", loop_count);
    oled.printf(6, 20, "timeout");
    oled.display();
    delay(1000);
    return;
  }

  if (read_buf[0] != 0xff) {
    // start byte不正
    oled.clear();
    oled.setCursor(0, 0);
    oled.printf(6, 8, "loop: %5d", loop_count);
    oled.printf(6, 20, "invalid start byte");
    oled.display();
    delay(1000);
    return;
  }
  if (read_buf[1] != 0x86) {
    // cmd byte不正
    oled.clear();
    oled.setCursor(0, 0);
    oled.printf(6, 8, "loop: %5d", loop_count);
    oled.printf(6, 20, "invalid cmd byte");
    oled.display();
    delay(1000);
    return;
  }


  uint8_t c = 0;
  for (int i = 1; i < 8; i++) {
    c += read_buf[i];
  }
  if (0xff - c + 1 != read_buf[8]) {
    // チェックサム不正
    oled.clear();
    oled.setCursor(0, 0);
    oled.printf(6, 8, "loop: %5d", loop_count);
    oled.printf(6, 20, "invalid checksum");
    oled.display();
    delay(1000);
    return;
  }

  uint16_t co2 = read_buf[2] * 256 + read_buf[3];

  oled.clear();
  oled.setCursor(0, 0);
  oled.printf(6, 8, "loop: %5d", loop_count);
  oled.printf(6, 20, "co2 : %5d ppm", co2);
  oled.display();
  delay(1000);
}
