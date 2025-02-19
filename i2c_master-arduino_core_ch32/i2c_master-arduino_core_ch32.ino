#include "Wire.h"

#define SHT31_I2C_ADDR 0x44

#define I2C_SDA_PIN PC1
#define I2C_SCL_PIN PC2

void setup()
{
  Serial.begin(115200);

  // not working - 動作を確認できなかった
  // USART_Printf_Init(115200);

  Serial.println("init");
  // printf("init\r\n");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  // ソフトリセット
  Wire.beginTransmission(SHT31_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0xA2);
  Wire.endTransmission();

  delay(300);

  // ステータスレジスタ消去
  Wire.beginTransmission(SHT31_I2C_ADDR);
  Wire.write(0x30);
  Wire.write(0x41);
  Wire.endTransmission();

  delay(300);

  Serial.println("start");
  // printf("start\r\n");
}

uint32_t count = 0;

void loop()
{
  Serial.print("loop ");
  Serial.println(count++);
  // printf("loop %d\r\n", count);

  uint8_t dac[6];
  int i, t, h;
  uint32_t temperature, humidity;

  Wire.beginTransmission(SHT31_I2C_ADDR);
  Wire.write(0x24);
  Wire.write(0x00);
  Wire.endTransmission();

  delay(300);

  Wire.requestFrom(SHT31_I2C_ADDR, 6);
  for (i = 0; i < 6; i++)
  {
    dac[i] = Wire.read();
  }
  Wire.endTransmission();

  t = (dac[0] << 8) | dac[1];                       // 1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、tに代入
  temperature = (((uint32_t)(t) * 175) >> 16) - 45; // 温度の計算、temperatureに代入
  h = (dac[3] << 8) | dac[4];                       // 4Byte目のデータを8bit左にシフト、OR演算子で5Byte目のデータを結合して、hに代入
  humidity = (((uint32_t)(h)*100) >> 16);           // 湿度の計算、humidityに代入

  Serial.print("temperature: ");
  Serial.print(temperature);
  Serial.print(" humidity: ");
  Serial.println(humidity);

  delay(5000);
}
