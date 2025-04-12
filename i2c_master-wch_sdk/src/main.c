#include <ch32v00x.h>
#include <debug.h>

#define TIMEOUT_MAX 100000

#define SHT31_I2C_ADDR 0x44

#define USE_REMAP 1

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

// 送信
int8_t send_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
  int32_t timeout;

  // I2Cのバスがビジーでなくなるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
    ;

  if (timeout < 0)
  {
    printf("i2c error: i2c bus busy\r\n");
    return -1;
  }

  // 通信の開始の送信
  I2C_GenerateSTART(I2C1, ENABLE);

  // マスターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c start error\r\n");
    return -1;
  }

  // スレーブアドレスをマスターからスレーブへの通信として送信
  I2C_Send7bitAddress(I2C1, address << 1, I2C_Direction_Transmitter);

  // トランスミッターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c address error\r\n");
    return -1;
  }

  for (int i = 0; i < length; i++)
  {
    // 送信バッファが空になるまで待つ
    while (timeout-- && I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE) == RESET)
      ;
    if (timeout < 0)
    {
      printf("i2c error: i2c tx empty error\r\n");
      return -1;
    }

    // 1バイトのデータ送信
    I2C_SendData(I2C1, data[i]);
  }

  // 送信完了イベントを待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c tx complete error\r\n");
    return -1;
  }

  // 通信の終了の送信
  I2C_GenerateSTOP(I2C1, ENABLE);

  return 0;
}

// 受信
int8_t read_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
  int32_t timeout;

  // I2Cのバスがビジーでなくなるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c bus busy\r\n");
    return -1;
  }

  // 通信の開始の送信
  I2C_GenerateSTART(I2C1, ENABLE);

  // マスターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c start error\r\n");
    return -1;
  }

  // スレーブアドレスをマスターからスレーブへの通信として送信
  I2C_Send7bitAddress(I2C1, address << 1, I2C_Direction_Receiver);

  // マスターモードに準備できるまで待つ
  timeout = TIMEOUT_MAX;
  while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    ;
  if (timeout < 0)
  {
    printf("i2c error: i2c address error\r\n");
    return -1;
  }

  for (int i = 0; i < length; i++)
  {
    // 受信完了イベントを待つ
    timeout = TIMEOUT_MAX;
    while (timeout-- && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
      ;
    if (timeout < 0)
    {
      printf("i2c error: i2c rx complete error\r\n");
      return -1;
    }

    // 受信データの受信
    data[i] = I2C_ReceiveData(I2C1);
  }

  // 通信の終了の送信
  I2C_GenerateSTOP(I2C1, ENABLE);
}

int main(void)
{
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  SystemCoreClockUpdate();
  Delay_Init();

  // SDI printf の有効化
  // ビルドフラグ SDI_PRINT=1 が必要
  // wlink sdi-print enable を実行し、USBシリアルをリスンする
  // SDI_Printf_Enable();

  // USART1 printf の有効化
  USART_Printf_Init(115200);

  Delay_Ms(100);

  printf("init\r\n");

  GPIO_InitTypeDef GPIO_InitStructure = {0};
  I2C_InitTypeDef I2C_InitTSturcture = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);
#if USE_REMAP
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
#endif

#if USE_REMAP
  // I2Cのリマップ
  // GPIO_PartialRemap_I2C1
  // GPIO_FullRemap_I2C1
  GPIO_PinRemapConfig(GPIO_FullRemap_I2C1, ENABLE);
  // PC6: SDA, PC5: SCL
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#else
  // PC1: SDA, PC2: SCL
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
#endif

  // I2C1の設定
  // クロック速度
  I2C_InitTSturcture.I2C_ClockSpeed = 400000;
  // モード（固定値）
  I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
  // デューティサイクル
  I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
  // Ackを有効にする
  I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
  // Ackアドレスのビット数
  I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  // 設定の有効化
  I2C_Init(I2C1, &I2C_InitTSturcture);

  // I2Cの有効化
  I2C_Cmd(I2C1, ENABLE);
  I2C_AcknowledgeConfig(I2C1, ENABLE);

  Delay_Ms(100);

  printf("first send\r\n");

  uint8_t buf1[2] = {0x30, 0xA2};
  send_i2c_data(SHT31_I2C_ADDR, buf1, 2);
  Delay_Ms(300);

  uint8_t buf2[2] = {0x30, 0x41};
  send_i2c_data(SHT31_I2C_ADDR, buf2, 2);
  Delay_Ms(300);

  printf("start\r\n");

  int count = 0;

  while (1)
  {
    printf("loop %d\r\n", count++);

    uint8_t dac[6];
    uint32_t t, h;
    uint32_t temperature, humidity;

    uint8_t buf[2] = {0x24, 0x00};
    send_i2c_data(SHT31_I2C_ADDR, buf, 2);

    Delay_Ms(300);

    read_i2c_data(SHT31_I2C_ADDR, dac, sizeof(dac));

    t = (dac[0] << 8) | dac[1];                       // 1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、tに代入
    temperature = (((uint32_t)(t) * 175) >> 16) - 45; // 温度の計算、temperatureに代入
    h = (dac[3] << 8) | dac[4];                       // 4Byte目のデータを8bit左にシフト、OR演算子で5Byte目のデータを結合して、hに代入
    humidity = (((uint32_t)(h) * 100) >> 16);         // 湿度の計算、humidityに代入

    printf("Temperature: %lu, Humidity: %lu\r\n", temperature, humidity);

    Delay_Ms(5000);
  }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
  while (1)
  {
  }
}