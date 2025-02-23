#include "hal/wch-hal-i2c.h"

#define SHT31_I2C_ADDR 0x44

void setup()
{
  // UART
  Serial.begin(115200);
  Serial.println("init");

  // minichlink -T
  printf("init\r\n");

  i2c_init(80000, I2C_DutyCycle_16_9, 0x02, I2C_Ack_Enable, I2C_AcknowledgedAddress_7bit);

  // ソフトリセット
  uint8_t buf1[2] = {0x30, 0xA2};
  i2c_write(SHT31_I2C_ADDR, buf1, 2);
  delay(300);

  // ステータスレジスタ消去
  uint8_t buf2[2] = {0x30, 0x41};
  i2c_write(SHT31_I2C_ADDR, buf2, 2);
  delay(300);

  Serial.println("start");
  printf("start\r\n");
}

uint32_t count = 0;

void loop()
{
  Serial.print("loop ");
  Serial.println(count++);
  printf("loop %d\r\n", count);

  uint8_t dac[6];
  int i, t, h;
  uint32_t temperature, humidity;

  uint8_t buf[2] = {0x24, 0x00};
  i2c_write(SHT31_I2C_ADDR, buf, 2);

  delay(300);

  i2c_read(SHT31_I2C_ADDR, dac, 6);

  t = (dac[0] << 8) | dac[1];                       // 1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、tに代入
  temperature = (((uint32_t)(t) * 175) >> 16) - 45; // 温度の計算、temperatureに代入
  h = (dac[3] << 8) | dac[4];                       // 4Byte目のデータを8bit左にシフト、OR演算子で5Byte目のデータを結合して、hに代入
  humidity = (((uint32_t)(h)*100) >> 16);           // 湿度の計算、humidityに代入

  printf("temperature: %d humidity: %d\r\n", temperature, humidity);
  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.print(" humidity: ");
  Serial.println(humidity);

  delay(5000);
}
