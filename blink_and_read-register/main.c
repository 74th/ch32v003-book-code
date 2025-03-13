#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

#define LED_PIN_NO 0
#define BUTTON_PIN_NO 1

int main()
{
	SystemInit();

	printf("init\r\n");

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA;

	// PC0 出力
	// 設定リセット
	GPIOC->CFGLR &= ~(0xf << (4 * LED_PIN_NO));
	// GPIOA->CFGLR GPIO設定
	//   GPIO_CNF_OUT_PP 出力プッシュプル
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * LED_PIN_NO);

	// PA1 入力
	// 設定リセット
	GPIOA->CFGLR &= ~(0xf << (4 * BUTTON_PIN_NO));
	// GPIOA->CFGLR GPIO設定
	//   GPIO_CNF_IN_FLOATING フローティング
	//   GPIO_CNF_IN_PUPD プルダウンorプルダウン
	GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_PUPD) << (4 * BUTTON_PIN_NO);

	// GPIOA->BSHR　プルアップ、プルダウン設定
	//   0: プルアップ
	//   1: プルダウン
	GPIOA->OUTDR = 0x1 << BUTTON_PIN_NO;

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		uint8_t btn = (GPIOA->INDR & (1 << BUTTON_PIN_NO)) == (1 << BUTTON_PIN_NO);
		if (btn == 0)
		{
			printf("button pressed\r\n");
			Delay_Ms(1000);
			continue;
		}

		// PC0 に1を出力
		GPIOC->BSHR |= 1 << LED_PIN_NO;
		Delay_Ms(500);
		// PC0 に0を出力
		GPIOC->BSHR |= 1 << (16 + LED_PIN_NO);
		Delay_Ms(500);
	}
}
