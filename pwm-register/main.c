#include "ch32fun.h"
#include <stdio.h>

#define USE_REMAP 1

void t1pwm_init(void)
{
	// TIM、GPIOのクロック供給の有効化
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC |
					  RCC_APB2Periph_TIM1;
#if USE_REMAP
	// AFIOのクロック供給の有効化
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
#endif

#if USE_REMAP
	// TIM1_CH1_1 -> PC6
	GPIOC->CFGLR &= ~(0xf << (4 * 6));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 6);

	// TIM1_CH4_1 -> PD3
	GPIOD->CFGLR &= ~(0xf << (4 * 3));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 3);
#else
	// TIM1_CH1 -> PD2
	GPIOD->CFGLR &= ~(0xf << (4 * 2));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 2);

	// TIM1_CH4 -> PC4
	GPIOC->CFGLR &= ~(0xf << (4 * 4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 4);
#endif

#if USE_REMAP
	// REMAPの有効化
	AFIO->PCFR1 |= AFIO_PCFR1_TIM1_REMAP_PARTIALREMAP1;
#endif
	// リセット
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	// CTLR1: default is up, events generated, edge align
	// SMCFGR: default clk input is CK_INT

	// Timerの設定
	// Timer1を256周期で有効化
	TIM1->PSC = 1;
	TIM1->ATRLR = 255;
	// 繰り返す
	TIM1->SWEVGR |= TIM_UG;

	// TIM1_CH1
	// CH1の比較アウトプットを有効化
	TIM1->CCER |= TIM_CC1E | TIM_CC1P;
	// PWM Mode (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;
	// CH1をアウトプットで有効化
	TIM1->CH1CVR = 0;

	// TIM2_CH2
	// CH4の比較アウトプットを有効化
	TIM1->CCER |= TIM_CC4E | TIM_CC4P;
	// PWM mode (CC1S = 00, OC1M = 110)
	TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;
	// CH4をアウトプットで有効化
	TIM1->CH4CVR = 0;

	// TIM1の出力を有効化
	TIM1->BDTR |= TIM_MOE;

	// TIM1を有効化
	TIM1->CTLR1 |= TIM_CEN;
}

int main()
{
	uint32_t count = 0;

	SystemInit();
	Delay_Ms(100);

	printf("init\n\r");

	t1pwm_init();

	printf("start\n\r");

	while (1)
	{
		Delay_Ms(100);

		printf("up\r\n");

		for (int i = 1; i <= 32; i++)
		{
			// Pulse幅の設定
			TIM1->CH1CVR = 8 * i - 1;	 // Chl 1
			TIM1->CH4CVR = 8 * (33 - i); // Chl 4
			Delay_Us(1000000 / 32);
		}

		printf("down\r\n");

		for (int i = 1; i <= 32; i++)
		{
			TIM1->CH1CVR = 8 * (33 - i); // Chl 1
			TIM1->CH4CVR = 8 * i - 1;	 // Chl 4
			Delay_Us(1000000 / 32);
		}
	}
}