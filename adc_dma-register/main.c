#include "ch32fun.h"
#include <stdio.h>

#define ADC_NUM_CHANNEL 2

// ピンの定義
#define VRX_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#define VRY_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)

volatile uint16_t adc_buf[ADC_NUM_CHANNEL];

int main()
{
  SystemInit();

  printf("init\r\n");

  // GPIOA、ADC1、DMA1にクロック供給
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;
  RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

  // PA1 アナログ入力
  GPIOA->CFGLR &= ~(0xf << (4 * 1));
  GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 1);

  // PA2 アナログ入力
  GPIOA->CFGLR &= ~(0xf << (4 * 2));
  GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 2);

  // ADC初期化
  ADC1->RSQR1 = (ADC_NUM_CHANNEL - 1) << 20;
  ADC1->RSQR2 = 0;
  // 1番目にチャンネル1、2番目にチャンネル0を指定
  ADC1->RSQR3 = (ADC_Channel_1 << (5 * 0)) | (ADC_Channel_0 << (5 * 1));
  // 1番目、2番目のサンプルレートを設定
  ADC1->SAMPTR2 = (ADC_SampleTime_241Cycles << (3 * 0)) | (ADC_SampleTime_241Cycles << (3 * 1));
  // 連続スキャンモード
  ADC1->CTLR1 |= ADC_SCAN;
  ADC1->CTLR2 |=
      // 連続変換
      ADC_CONT |
      // ソフトウェアトリガを有効化
      ADC_EXTSEL |
      // DMAのトリガを有効化
      ADC_DMA;

  // ADCの有効化
  ADC1->CTLR2 |= ADC_ADON;

  // ADCキャリブレーション
  ADC1->CTLR2 |= ADC_RSTCAL;
  while (ADC1->CTLR2 & ADC_RSTCAL)
    ;
  ADC1->CTLR2 |= ADC_CAL;
  while (ADC1->CTLR2 & ADC_CAL)
    ;

  // DMA1_Channel1
  // ADC1の変換結果 → メモリ
  DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
  DMA1_Channel1->MADDR = (uint32_t)adc_buf;
  // チャンネルの数だけインクリメントする
  DMA1_Channel1->CNTR = ADC_NUM_CHANNEL;
  DMA1_Channel1->CFGR =
      // Peripheralからメモリへの転送
      DMA_DIR_PeripheralSRC |
      DMA_M2M_Disable |
      // 優先度高
      DMA_Priority_VeryHigh |
      // メモリのデータサイズは16bit
      DMA_MemoryDataSize_HalfWord |
      // Peripheralのデータサイズは16bit
      DMA_PeripheralDataSize_HalfWord |
      // メモリのみインクリメント
      DMA_MemoryInc_Enable |
      // 繰り返す
      DMA_Mode_Circular;

  // DMA1_Channel1有効化
  DMA1_Channel1->CFGR |= DMA_CFGR1_EN;
  // 最初の変換をトリガ
  ADC1->CTLR2 |= ADC_SWSTART;

  printf("start\r\n");

  int count = 0;

  while (1)
  {
    printf("loop %d\r\n", count++);

    // DMAで自動的にバッファの値が更新されるので、読み取りのみ
    printf("x: %d, y: %d\r\n", adc_buf[0], adc_buf[1]);

    Delay_Ms(500);
  }
}
