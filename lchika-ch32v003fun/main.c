#include "ch32fun.h"
#include <stdio.h>

#include "ch32v003_GPIO_branchless.h"

#define LED_PIN GPIOv_from_PORT_PIN(GPIO_port_C, 0)

uint32_t count;

int main()
{
	SystemInit();

	// GPIO有効化
	GPIO_port_enable(GPIO_port_C);

	// PC0 ピンを出力に設定
	GPIO_pinMode(LED_PIN, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);

	count = 0;

	while (1)
	{
		GPIO_digitalWrite(LED_PIN, high);
		GPIOC->BSHR = 1 << 1;
		printf("+%lu\n", count++);
		Delay_Ms(500);
		GPIO_digitalWrite(LED_PIN, low);
		printf("-%lu\n", count++);
		Delay_Ms(500);
	}
}
