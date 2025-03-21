#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

#define LED_PIN_NO 0
#define LED_NUM 6
#define DATA_SIZE LED_NUM * 3

void task(int n)
{
	uint32_t h = 1 << LED_PIN_NO;
	uint32_t l = 1 << (16 + LED_PIN_NO);
	uint8_t data[DATA_SIZE];

	for (int i = 0; i < LED_NUM; i++)
	{
		switch ((n + i) % 6)
		{
		case 0:
			data[i * 3] = 0x20;
			data[i * 3 + 1] = 0x00;
			data[i * 3 + 2] = 0x00;
			break;
		case 1:
			data[i * 3] = 0x20;
			data[i * 3 + 1] = 0x20;
			data[i * 3 + 2] = 0x00;
			break;
		case 2:
			data[i * 3] = 0x00;
			data[i * 3 + 1] = 0x20;
			data[i * 3 + 2] = 0x00;
			break;
		case 3:
			data[i * 3] = 0x00;
			data[i * 3 + 1] = 0x20;
			data[i * 3 + 2] = 0x20;
			break;
		case 4:
			data[i * 3] = 0x00;
			data[i * 3 + 1] = 0x00;
			data[i * 3 + 2] = 0x20;
			break;
		case 5:
			data[i * 3] = 0x20;
			data[i * 3 + 1] = 0x00;
			data[i * 3 + 2] = 0x20;
			break;
		}
	}

	for (int i = 0; i < DATA_SIZE; i++)
	{
		uint16_t c = data[i];
		for (int j = 0; j < 8; j++)
		{
			if (c & 0x1)
			{
				// 0.7us
				GPIOC->BSHR = h;
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				// 0.6us
				GPIOC->BSHR = l;
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
			}
			else
			{
				// 0.35us
				GPIOC->BSHR = h;
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				// 0.8us
				GPIOC->BSHR = l;
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
				asm("c.nop");
			}
			c = c >> 1;
		}
	}
}

int main()
{
	SystemInit();

	printf("init\r\n");

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// PC0 出力
	// 設定リセット
	GPIOC->CFGLR &= ~(0xf << (4 * LED_PIN_NO));
	// GPIO_CNF_OUT_PP 出力プッシュプル
	GPIOC->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (4 * LED_PIN_NO);

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		task(count);
		count++;
		Delay_Ms(300);
	}
}
