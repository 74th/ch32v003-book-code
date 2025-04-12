#include "ch32fun.h"
#include <stdio.h>

#define I2C_SLAVE_ADDRESS 0x44
#define LOOP_MS 1000

#define USE_REMAP 1

void init_rcc(void)
{
  // クロック供給の有効化
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;
  RCC->APB1PCENR |= RCC_APB1Periph_I2C1;
#if USE_REMAP
  RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
#endif
}

/**
 * set up i2c master
 * https://github.com/cnlohr/ch32v003fun/blob/ee60fd756aa015be799e430d230a8f33266421de/examples/i2c_oled/i2c.h#L321
 * https://github.com/cnlohr/ch32v003fun/blob/ee60fd756aa015be799e430d230a8f33266421de/examples/i2c_oled/i2c.h#L40
 */
void init_i2c_master()
{

#if USE_REMAP
  // リマップ設定リセット
  AFIO->PCFR1 &= ~(AFIO_PCFR1_I2C1_REMAP | AFIO_PCFR1_I2C1_HIGH_BIT_REMAP);
  // リマップ設定
  // Remap1: AFIO_PCFR1_I2C1_REMAP
  // Remap2: AFIO_PCFR1_I2C1_HIGH_BIT_REMAP
  AFIO->PCFR1 |= AFIO_PCFR1_I2C1_HIGH_BIT_REMAP;

  // PC6: SDA
  GPIOC->CFGLR &= ~(0xf << (4 * 6));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_OD_AF) << (4 * 6);

  // PC5: SCL
  GPIOC->CFGLR &= ~(0xf << (4 * 5));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_OD_AF) << (4 * 5);
#else
  // PC1: SDA
  GPIOC->CFGLR &= ~(0xf << (4 * 1));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_OD_AF) << (4 * 1);

  // PC2: SCL
  GPIOC->CFGLR &= ~(0xf << (4 * 2));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_OD_AF) << (4 * 2);
#endif

  uint16_t tempreg;

  I2C1->CTLR1 |= I2C_CTLR1_SWRST;
  I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

  uint32_t clkrate = 400000;
  uint32_t prerate = clkrate * 2;

  // 回路のクロック周波数を設定
  tempreg = I2C1->CTLR2;
  tempreg &= ~I2C_CTLR2_FREQ;
  tempreg |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;
  I2C1->CTLR2 = tempreg;

  // I2Cクロックスピードの設定
  tempreg = I2C1->CKCFGR;
  tempreg &= ~I2C_CKCFGR_CCR;
  tempreg = (FUNCONF_SYSTEM_CORE_CLOCK / (25 * clkrate)) & I2C_CKCFGR_CCR;
  I2C1->CKCFGR = tempreg;

  I2C1->CKCFGR |=
      // デューティーサイクル(16:9の場合1、2:1の場合は設定なし)
      I2C_CKCFGR_DUTY |
      // ファストモード（100kHzより大きい速度の場合指定）
      I2C_CKCFGR_FS;

  // Enable I2C
  I2C1->CTLR1 |= I2C_CTLR1_PE;

  // set ACK mode
  I2C1->CTLR1 |= I2C_CTLR1_ACK;
}

#define TIMEOUT_MAX 100000
// https://github.com/cnlohr/ch32v003fun/blob/ee60fd756aa015be799e430d230a8f33266421de/examples/i2c_oled/i2c.h#L115
#define I2C_EVENT_MASTER_MODE_SELECT ((uint32_t)0x00030001)               /* BUSY, MSL and SB flag */
#define I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ((uint32_t)0x00070082) /* BUSY, MSL, ADDR, TXE and TRA flags */
#define I2C_EVENT_MASTER_BYTE_TRANSMITTED ((uint32_t)0x00070084)          /* TRA, BUSY, MSL, TXE and BTF flags */
#define I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED ((uint32_t)0x00030002)    /* BUSY, SB, ADDR flags */
#define I2C_EVENT_MASTER_BYTE_RECEIVED ((uint32_t)0x00030040)             /* TRA, SB, ADDR flags */

/*
 * イベントコード
 * https://github.com/cnlohr/ch32v003fun/blob/8f7517e9acf2a3de650fc3cdd125ed1a1d01f9a2/extralibs/ssd1306_i2c.h#L125
 */
uint8_t check_i2c_event(uint32_t event_mask)
{
  /* read order matters here! STAR1 before STAR2!! */
  uint32_t status = I2C1->STAR1 | (I2C1->STAR2 << 16);
  return (status & event_mask) == event_mask;
}

/*
 * low-level packet send for blocking polled operation via i2c
 * https://github.com/cnlohr/ch32v003fun/blob/8f7517e9acf2a3de650fc3cdd125ed1a1d01f9a2/extralibs/ssd1306_i2c.h#L245
 */
// 送信
uint8_t send_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
  int32_t timeout;

  // I2Cのバスがビジーでなくなるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (I2C1->STAR2 & I2C_STAR2_BUSY))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for not BUSY is timeout\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  // 通信の開始の送信
  I2C1->CTLR1 |= I2C_CTLR1_START;

  // マスターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (!check_i2c_event(I2C_EVENT_MASTER_MODE_SELECT)))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for master select is timeout\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  // スレーブアドレスをマスターからスレーブへの通信として送信
  I2C1->DATAR = address << 1;

  // トランスミッターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (!check_i2c_event(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for transmit condition is timeout\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  for (int i = 0; i < length; i++)
  {
    // 1バイトのデータ送信
    timeout = TIMEOUT_MAX;
    while (!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--))
      ;
    if (timeout < 0)
    {
      printf("i2c error: waiting for data send is timeout\r\n");
      I2C1->CTLR1 |= I2C_CTLR1_STOP;
      return -1;
    }

    // 1バイト送信
    I2C1->DATAR = data[i];
  }

  // 送信完了イベントまで待つ
  timeout = TIMEOUT_MAX;
  while ((!check_i2c_event(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--))
    ;
  if (timeout == -1)
  {
    printf("i2c error: waiting for tx complete is timeout\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  // 通信の終了の送信
  I2C1->CTLR1 |= I2C_CTLR1_STOP;

  return 0;
}

int read_i2c_data(uint8_t address, uint8_t *buf, uint8_t length)
{
  int32_t timeout;

  I2C1->CTLR1 |= I2C_CTLR1_ACK;

  // I2Cのバスがビジーでなくなるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (I2C1->STAR2 & I2C_STAR2_BUSY))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for not BUSY is timeout 2\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  // 通信の開始の送信
  I2C1->CTLR1 |= I2C_CTLR1_START;

  // マスターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (!check_i2c_event(I2C_EVENT_MASTER_MODE_SELECT)))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for master select is timeout\r\n");
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  // スレーブアドレスをスレーブからマスターへの通信として送信
  I2C1->DATAR = address << 1 | 0x1;

  // レシーバーモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && (!check_i2c_event(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED)))
    ;
  if (timeout < 0)
  {
    printf("i2c error: waiting for transmit condition is timeout %04x, %04x\r\n", I2C1->STAR1, I2C1->STAR2);
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    return -1;
  }

  for (int i = 0; i < length; i++)
  {
    // 最後のバイトの受信の前にACKを降ろす
    if (length - 1 == i)
    {
      printf("i2c receive last byte\r\n");
      I2C1->CTLR1 &= ~I2C_CTLR1_ACK;
    }

    // 受信完了イベントを待つ
    timeout = TIMEOUT_MAX;
    while (timeout-- && (!check_i2c_event(I2C_EVENT_MASTER_BYTE_RECEIVED)))
      ;
    if (timeout == -1)
    {
      printf("i2c error: receiving for data send is timeout\r\n");
      I2C1->CTLR1 |= I2C_CTLR1_STOP;
      return -1;
    }

    // 受信データの受信
    buf[i] = I2C1->DATAR;
    printf("i2c receive: [%2d] 0x%02x\r\n", i, buf[i]);
  }

  // 通信の終了の送信
  I2C1->CTLR1 |= I2C_CTLR1_STOP;

  return 0;
}

int main()
{
  SystemInit();
  init_rcc();

  printf("init\r\n");

  init_i2c_master(I2C_SLAVE_ADDRESS);

  printf("setup\r\n");

  Delay_Ms(100);

  uint8_t buf1[2] = {0x30, 0xA2};
  send_i2c_data(I2C_SLAVE_ADDRESS, buf1, 2);
  Delay_Ms(300);

  uint8_t buf2[2] = {0x30, 0x41};
  send_i2c_data(I2C_SLAVE_ADDRESS, buf2, 2);
  Delay_Ms(300);
  printf("test i2c\r\n");

  printf("test i2c done\r\n");

  while (1)
  {
    uint8_t dac[6];
    uint32_t t, h;
    uint32_t temperature, humidity;

    uint8_t buf[2] = {0x24, 0x00};
    send_i2c_data(I2C_SLAVE_ADDRESS, buf, sizeof(buf));

    Delay_Ms(300);

    read_i2c_data(I2C_SLAVE_ADDRESS, dac, sizeof(dac));

    t = (dac[0] << 8) | dac[1];                       // 1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、tに代入
    temperature = (((uint32_t)(t) * 175) >> 16) - 45; // 温度の計算、temperatureに代入
    h = (dac[3] << 8) | dac[4];                       // 4Byte目のデータを8bit左にシフト、OR演算子で5Byte目のデータを結合して、hに代入
    humidity = (((uint32_t)(h) * 100) >> 16);         // 湿度の計算、humidityに代入

    printf("Temperature: %lu, Humidity: %lu\r\n", temperature, humidity);

    Delay_Ms(5000);
  }
}
