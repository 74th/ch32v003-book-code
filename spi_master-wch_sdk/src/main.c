#include <ch32v00x.h>
#include <debug.h>

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

uint8_t transfer_spi(uint8_t data)
{
  // 常に書き込みと読み込みを両方行う
  // 送信バッファが空になるのを待つ
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
    ;
  // 1バイトの送信
  SPI_I2S_SendData(SPI1, data);

  // 受信バッファにデータが入るのを待つ
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
    ;

  // 1バイトの受信
  return SPI_I2S_ReceiveData(SPI1);
}

// 送信
void send_spi_data(uint8_t *data, uint8_t length)
{
  for (int i = 0; i < length; i++)
  {
    transfer_spi(data[i]);
  }
}

// 受信
void read_spi_data(uint8_t *data, uint8_t length)
{
  for (int i = 0; i < length; i++)
  {
    data[i] = transfer_spi(0);
  }
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
  SPI_InitTypeDef SPI_InitStructure = {0};

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

  // PC1: NSS(CS)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // PC5: SCK
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // PC6: MISO
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // PC7: MOSI
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
  GPIO_Init(GPIOC, &GPIO_InitStructure);

  // SPI1の設定

  // 通信方向の指定
  // 全2重: SPI_Direction_2Lines_FullDuplex
  // 全2重受信のみ: SPI_Direction_2Lines_RxOnly
  // 受信のみ: SPI_Direction_1Line_Rx
  // 送信のみ: SPI_Direction_1Line_Tx
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  // マスターモード
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  // データサイズ（8bit or 16bit）
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  // SPIモード
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  // NSSピンの制御方法
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(SPI1, &SPI_InitStructure);

  SPI_Cmd(SPI1, ENABLE);

  GPIO_SetBits(GPIOC, GPIO_Pin_1);

  Delay_Ms(500);

  GPIO_ResetBits(GPIOC, GPIO_Pin_1);

  printf("software reset\r\n");

  uint8_t buf1[4] = {0xff, 0xff, 0xff, 0xff};
  send_spi_data(buf1, sizeof(buf1));
  Delay_Ms(100);

  uint8_t buf2[1] = {0x54};
  send_spi_data(buf2, sizeof(buf2));
  Delay_Ms(300);

  printf("start\r\n");

  int count = 0;

  while (1)
  {
    printf("loop %d\r\n", count++);

    uint8_t buf[2] = {0x00, 0x00};
    read_spi_data(buf, sizeof(buf));

    uint16_t raw = (buf[0] << 8) | buf[1];
    int32_t raw_int = 0;

    printf("raw: %04x\r\n", raw);

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

    printf("raw_int: %d\r\n", raw_int);
    int32_t int_temp = raw_int >> 4;
    int32_t frac_temp = raw_int & 0x0f;
    printf("temp:%3d.%04d\r\n", int_temp, frac_temp * 625);

    Delay_Ms(1000);
  }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
  while (1)
  {
  }
}