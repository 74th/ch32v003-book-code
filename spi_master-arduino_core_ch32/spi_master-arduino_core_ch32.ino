#include <Arduino.h>
#include <SPI.h>

#define USING_16BIT 0 // 16bitモードの場合は1にするが、動作しなかった

void reset_adt7310()
{
  Serial.println("software reset");

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  // ソフトウェアリセット
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);
  SPI.transfer(0xFF);

  delay(500);

  // 連続読み取りモード開始
  SPI.transfer(0x54);

  SPI.endTransaction();

  delay(500);
}

void setup()
{
  Serial.begin(115200);

  Serial.println("init");

  delay(3000);

  reset_adt7310();
}

void loop()
{
  Serial.println("loop");
  uint16_t raw;
  int32_t raw_int;

  SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  raw = (uint16_t)SPI.transfer(0) << 8;
  raw |= SPI.transfer(0);

  SPI.endTransaction();

  if (raw == 0x0000 || raw == 0xffff)
  {
    reset_adt7310();
    return;
  }

  Serial.printf("raw: %04X\r\n", raw);

  // 13bit
  raw = raw >> 3;

  if (raw & 0x1000)
  {
    raw_int = raw - 0x2000;
  }
  else
  {
    raw_int = raw;
  }

  Serial.print("raw_int:");
  Serial.println(raw_int);
  int32_t int_temp = raw_int >> 4;
  int32_t frac_temp = raw_int & 0x0f;
  Serial.printf("temp:%3d.%04d\r\n", int_temp, frac_temp * 625);
  ;
  // int32_t frac_temp = (raw_int >> 1) & 0x07;
  // Serial.printf("temp:%3d.%03d\r\n",int_temp, frac_temp * 125);

  delay(1000);
}