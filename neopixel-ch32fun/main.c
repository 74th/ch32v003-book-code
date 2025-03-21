#include "ch32fun.h"
#include <stdio.h>
#include <string.h>

#define WS2812DMA_IMPLEMENTATION
// #define WSRBG //For WS2816C's.
#define WSGRB // For SK6805-EC15
#define LED_NUM 6

#include "ws2812b_dma_spi_led_driver.h"

uint32_t data[LED_NUM];

uint32_t WS2812BLEDCallback(int ledno)
{
	return data[ledno];
}

int main()
{
	int k;
	SystemInit();

	WS2812BDMAInit();

	uint32_t count = 0;

	while (1)
	{

		while (WS2812BLEDInUse)
			;

		for (int i = 0; i < LED_NUM; i++)
		{
			switch ((count + i) % 6)
			{
			case 0:
				data[i] = 0x20 << 16 | 0x00 << 8 | 0x00;
				break;
			case 1:
				data[i] = 0x20 << 16 | 0x20 << 8 | 0x00;
				break;
			case 2:
				data[i] = 0x00 << 16 | 0x20 << 8 | 0x00;
				break;
			case 3:
				data[i] = 0x00 << 16 | 0x20 << 8 | 0x20;
				break;
			case 4:
				data[i] = 0x00 << 16 | 0x00 << 8 | 0x20;
				break;
			case 5:
				data[i] = 0x20 << 16 | 0x00 << 8 | 0x20;
				break;
			}
		}

		WS2812BDMAStart(LED_NUM);
		count++;
		Delay_Ms(300);
	}
}