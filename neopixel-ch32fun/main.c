#include "ch32fun.h"
#include <stdio.h>
#include <string.h>

#define WS2812DMA_IMPLEMENTATION
// #define WSRBG //For WS2816C's.
#define WSGRB // For SK6805-EC15
#define LED_NUM 6

// SPI DMA LEDドライバ
// https://github.com/cnlohr/ch32fun/blob/master/extralibs/ws2812b_dma_spi_led_driver.h
#include "ws2812b_dma_spi_led_driver.h"

// HSV を RGB に変換する関数（EHStoHEX）が、以下に収録されている
// https://github.com/cnlohr/ch32fun/blob/master/examples/ws2812bdemo/color_utilities.h
#include "color_utilities.h"

// red | (blue<<8) | (green<<16) と指定する
uint32_t data[LED_NUM];

// 色の指定を返すコールバック
uint32_t WS2812BLEDCallback(int ledno)
{
	return data[ledno];
}

int main()
{
	int k;
	SystemInit();

	// SPI DMAを利用するため、MOSI PC6で出力される
	WS2812BDMAInit();

	uint32_t count = 0;

	while (1)
	{
		// 転送終了まで待つ
		while (WS2812BLEDInUse)
			;

		for (int i = 0; i < LED_NUM; i++)
		{
			// HSVカラー指定
			data[i] = EHSVtoHEX((count + i) * 16 & 0xff, 255, 3);
		}

		// 転送開始
		WS2812BDMAStart(LED_NUM);

		count++;
		Delay_Ms(100);
	}
}