#define USE_BRANCHLESS 0

#include "ch32fun.h"
#include <stdio.h>

#if USE_BRANCHLESS
#include "ch32v003_GPIO_branchless.h"
#endif

// ピンの定義
#if USE_BRANCHLESS
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define BUTTON_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#else
#define LED_PIN PC0
#define BUTTON_PIN PA1
#endif

uint32_t count;

int main()
{
	SystemInit();

	printf("init\r\n");

#if USE_BRANCHLESS
	// GPIO有効化
	GPIO_port_enable(GPIO_port_C);
	GPIO_port_enable(GPIO_port_A);

	// PC0 出力
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);

	// PA1 入力
	// GPIO_pinMode_I_floating: フローティング入力
	// GPIO_pinMode_I_pullUp: プルアップ
	// GPIO_pinMode_I_pullDown: プルダウン
	GPIO_pinMode(BUTTON_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_In);
	printf("GPIOA->CFGLR: %08X\r\n", GPIOA->CFGLR);
	printf("GPIOA->OUTDR: %08X\r\n", GPIOA->OUTDR);
	printf("GPIOA->BSHR: %08X\r\n", GPIOA->BSHR);

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		// PA1 のボタンが押されていれば点灯しない
		uint8_t btn = GPIO_digitalRead(BUTTON_PIN);
		if (btn == low)
		{
			Delay_Ms(1000);
			continue;
		}

		GPIO_digitalWrite_high(LED_PIN);
		Delay_Ms(500);
		GPIO_digitalWrite_low(LED_PIN);
		Delay_Ms(500);
	}
#else
	// GPIO有効化
	funGpioInitAll();

	// PC0 出力
	funPinMode(LED_PIN, GPIO_Speed_10MHz | GPIO_CNF_OUT_PP);

	// PA1 入力
	funPinMode(BUTTON_PIN, GPIO_CNF_IN_PUPD);
	// プルアップの設定は、funDigitalWrite() で行う
	funDigitalWrite(BUTTON_PIN, FUN_HIGH);
	printf("GPIOA->CFGLR: %08X\r\n", GPIOA->CFGLR);
	printf("GPIOA->OUTDR: %08X\r\n", GPIOA->OUTDR);
	printf("GPIOA->BSHR: %08X\r\n", GPIOA->BSHR);

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		// PA1 のボタンが押されていれば点灯しない
		uint8_t btn = funDigitalRead(BUTTON_PIN);
		if (btn == FUN_LOW)
		{
			Delay_Ms(1000);
			continue;
		}

		funDigitalWrite(LED_PIN, FUN_HIGH);
		Delay_Ms(500);
		funDigitalWrite(LED_PIN, FUN_LOW);
		Delay_Ms(500);
	}
#endif
}
