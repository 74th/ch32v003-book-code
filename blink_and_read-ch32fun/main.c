#include "ch32fun.h"
#include <stdio.h>

#include "ch32v003_GPIO_branchless.h"

// ピンの定義
#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define BUTTON_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)

uint32_t count;

int main()
{
	SystemInit();

	printf("init\r\n");

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

		GPIO_digitalWrite(LED_PIN, high);
		Delay_Ms(500);
		GPIO_digitalWrite(LED_PIN, low);
		Delay_Ms(500);
	}
}
