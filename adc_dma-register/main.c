#include "ch32fun.h"
#include <stdio.h>

#include "ch32v003_GPIO_branchless.h"

#define ADC_NUM_CHANNEL 2

// ピンの定義
#define VRX_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#define VRY_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)

volatile uint16_t adc_buf[ADC_NUM_CHANNEL];

int main()
{
	SystemInit();

	printf("init\r\n");

	// GPIOA、ADC1、DMA1にクロック供給
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1;
	RCC->AHBPCENR |= RCC_AHBPeriph_DMA1;

	// PA1 アナログ入力
	GPIOA->CFGLR &= ~(0xf << (4 * 1));
	GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 1);

	// PA2 アナログ入力
	GPIOA->CFGLR &= ~(0xf << (4 * 2));
	GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_ANALOG) << (4 * 2);

	// ADC初期化
	ADC1->RSQR1 = (ADC_NUM_CHANNEL - 1) << 20;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = (1 << (5 * 0)) | (0 << (5 * 1));
	ADC1->SAMPTR2 = (7 << (3 * 0)) | (7 << (3 * 1));
	ADC1->CTLR1 |= ADC_SCAN;

	// ADC ON
	ADC1->CTLR2 |= ADC_ADON;

	// ADCキャリブレーション
	ADC1->CTLR2 |= ADC_RSTCAL;
	while (ADC1->CTLR2 & ADC_RSTCAL)
		;
	ADC1->CTLR2 |= ADC_CAL;
	while (ADC1->CTLR2 & ADC_CAL)
		;

	// DMA1_Channel1
	DMA1_Channel1->PADDR = (uint32_t)&ADC1->RDATAR;
	DMA1_Channel1->MADDR = (uint32_t)adc_buf;
	DMA1_Channel1->CNTR = ADC_NUM_CHANNEL;
	DMA1_Channel1->CFGR =
		DMA_M2M_Disable |
		DMA_Priority_VeryHigh |
		DMA_MemoryDataSize_HalfWord |
		DMA_PeripheralDataSize_HalfWord |
		DMA_MemoryInc_Enable |
		DMA_Mode_Circular |
		DMA_DIR_PeripheralSRC;

	// DMA1_Channel1有効化
	DMA1_Channel1->CFGR |= DMA_CFGR1_EN;

	// ADC1を有効にし、連続変換とDMAを有効化
	ADC1->CTLR2 |= ADC_CONT | ADC_DMA | ADC_EXTSEL;

	// ADC1開始
	ADC1->CTLR2 |= ADC_SWSTART;

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		// DMAで自動的にバッファの値が更新されるので、読み取りのみ
		printf("x: %d, y: %d\r\n", adc_buf[0], adc_buf[1]);

		Delay_Ms(500);
	}
}
