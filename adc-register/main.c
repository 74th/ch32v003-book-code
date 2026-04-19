#include "ch32fun.h"
#include <stdio.h>

// ピンの定義
#define VRX_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#define VRY_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)

// ADCへの切り替え時にADCの高い入力インピーダンスを打ち消すためのディレイ
// https://github.com/cnlohr/ch32fun/blob/cfffff6d6bd9bd97d7348d044a1de145f4548072/extralibs/ch32v003_GPIO_branchless.h#L54
#define GPIO_ADC_MUX_DELAY 200

uint32_t count;

int main()
{
  SystemInit();

  printf("init\r\n");

  // GPIOA、ADC1 にクロック供給
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;

  // PA1 アナログ入力
  GPIOA->CFGLR &= ~(0xf << (4 * 1));
  GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 1);

  // PA2 アナログ入力
  GPIOA->CFGLR &= ~(0xf << (4 * 2));
  GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 2);

  // ADC初期化
  ADC1->RSQR1 = 0;
  ADC1->RSQR2 = 0;
  ADC1->RSQR3 = 0;
  // ソフトウェアトリガ
  ADC1->CTLR2 |= ADC_EXTSEL;

  // ADC ON
  ADC1->CTLR2 |= ADC_ADON;

  // ADCキャリブレーション
  ADC1->CTLR2 |= ADC_RSTCAL;
  while (ADC1->CTLR2 & ADC_RSTCAL)
    ;
  ADC1->CTLR2 |= ADC_CAL;
  while (ADC1->CTLR2 & ADC_CAL)
    ;

  printf("start\r\n");

  int count = 0;

  while (1)
  {
    printf("loop %d\r\n", count++);

    // CH1、CH0の連続変換を利用せず、
    // CH1で設定、変換し、その後再設定してCH0で設定、変換する

    // 1番目の変換に、CH1を選択
    ADC1->RSQR3 = ADC_Channel_1 << (5 * 0);
    // CH1のサンプルレート
    ADC1->SAMPTR2 = ADC_SampleTime_241Cycles << (3 * 1);

    Delay_Us(GPIO_ADC_MUX_DELAY);
    // 変換をトリガ
    ADC1->CTLR2 |= ADC_SWSTART;
    while (!(ADC1->STATR & ADC_EOC))
      ;
    uint16_t x = ADC1->RDATAR;

    // 1番目の変換に、CH0を選択
    ADC1->RSQR3 = ADC_Channel_0 << (5 * 0);
    // CH0のサンプルレート
    ADC1->SAMPTR2 = ADC_SampleTime_241Cycles << (3 * 0);

    Delay_Us(GPIO_ADC_MUX_DELAY);
    // 変換をトリガ
    ADC1->CTLR2 |= ADC_SWSTART;
    while (!(ADC1->STATR & ADC_EOC))
      ;
    uint16_t y = ADC1->RDATAR;

    printf("x: %d, y: %d\r\n", x, y);

    Delay_Ms(500);
  }
}
