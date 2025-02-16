#include "ch32fun.h"
#include <stdio.h>

#include "ch32v003_GPIO_branchless.h"

#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)
#define BUTTON_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)

uint32_t count;

int main()
{
	SystemInit();

	// GPIO有効化
	GPIO_port_enable(GPIO_port_C);
	GPIO_port_enable(GPIO_port_A);

	// PC0 ピンを出力に設定
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	// PA1 ピンを入力に設定
	GPIO_pinMode(BUTTON_PIN, GPIO_pinMode_I_pullUp, GPIO_Speed_10MHz);

	count = 0;

	while (1)
	{
		// PA1 のボタンが押されていれば点灯しない
		uint8_t btn = GPIO_digitalRead(BUTTON_PIN);
		if (btn == low)
		{
			continue;
		}

		GPIO_digitalWrite(LED_PIN, high);
		GPIOC->BSHR = 1 << 1;
		printf("H %lu\n", count++);
		Delay_Ms(500);
		GPIO_digitalWrite(LED_PIN, low);
		printf("L %lu\n", count++);
		Delay_Ms(500);
	}
}
