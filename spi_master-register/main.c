#include "ch32fun.h"
#include <stdio.h>

#define I2C_SLAVE_ADDRESS 0x44
#define LOOP_MS 1000

void init_rcc(void)
{
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_SPI1;
}

void init_spi_master()
{

  // PC1: NSS
  GPIOC->CFGLR &= ~(0xf << (4 * 1));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 1);

  // PC5: SCK
  GPIOC->CFGLR &= ~(0xf << (4 * 5));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5);

  // PC6: MOSI
  GPIOC->CFGLR &= ~(0xf << (4 * 6));
  GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP_AF) << (4 * 6);

  // PC7: MISO
  GPIOC->CFGLR &= ~(0xf << (4 * 7));
  GPIOC->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING) << (4 * 7);

  uint16_t tempreg;

  // SPI設定
  // SPI設定をリセット
  SPI1->CTLR1 = 0;
  SPI1->CTLR1 |=
      // プリスケーラ
      (SPI_CTLR1_BR & SPI_BaudRatePrescaler_32) |
      // SPIモード
      SPI_CPOL_Low | SPI_CPHA_1Edge |
      // NSSの制御モード
      SPI_NSS_Hard |
      SPI_Mode_Master;

  // SPIの開始
  SPI1->CTLR2 |= SPI_CTLR2_SSOE;
}

void spi_begin_8()
{
  SPI1->CTLR1 &= ~(SPI_CTLR1_DFF);
  SPI1->CTLR1 |= SPI_CTLR1_SPE;
}

void spi_end()
{
  SPI1->CTLR1 &= ~(SPI_CTLR1_SPE);
}

uint8_t spi_transfer(uint8_t data)
{
  while (!(SPI1->STATR & SPI_STATR_TXE))
    ;
  SPI1->DATAR = data;

  while (!(SPI1->STATR & SPI_STATR_RXNE))
    ;
  return SPI1->DATAR;
}

int main()
{
  SystemInit();

  printf("init\r\n");

  init_rcc();

  init_spi_master();

  printf("SPI1->CTLR1 %04x\r\n", SPI1->CTLR1);
  printf("SPI1->CTLR2 %04x\r\n", SPI1->CTLR2);
  printf("GPIOC->CFGLR %08x\r\n", GPIOC->CFGLR);

  printf("setup\r\n");

  Delay_Ms(100);

  spi_begin_8();

  spi_transfer(0xFF);
  spi_transfer(0xFF);
  spi_transfer(0xFF);
  spi_transfer(0xFF);

  Delay_Ms(100);

  spi_transfer(0x54);

  spi_end();

  Delay_Ms(500);

  while (1)
  {
    printf("loop\r\n");

    spi_begin_8();

    uint8_t b1 = spi_transfer(0x00);
    uint8_t b2 = spi_transfer(0x00);

    spi_end();

    uint16_t raw = b1 << 8 | b2;
    printf("raw: %04x\r\n", raw);

    raw = raw >> 3;

    int32_t raw_int;

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
