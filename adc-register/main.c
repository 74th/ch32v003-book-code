#include "ch32fun.h"
#include <stdio.h>

#include "ch32v003_GPIO_branchless.h"

// ピンの定義
#define VRX_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#define VRY_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)

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

		// CH1を選択
		ADC1->RSQR3 = ADC_Channel_1;
		Delay_Us(GPIO_ADC_MUX_DELAY);
		// 読み取りをトリガ
		ADC1->CTLR2 |= ADC_SWSTART;
		while (!(ADC1->STATR & ADC_EOC))
		{
		}
		uint16_t x = ADC1->RDATAR;

		// CH0を選択
		ADC1->RSQR3 = ADC_Channel_0;
		Delay_Us(GPIO_ADC_MUX_DELAY);
		// 読み取りをトリガ
		ADC1->CTLR2 |= ADC_SWSTART;
		while (!(ADC1->STATR & ADC_EOC))
		{
		}
		uint16_t y = GPIO_analogRead(GPIO_Ain0_A2);

		printf("x: %d, y: %d\r\n", x, y);

		Delay_Ms(500);
	}
}
