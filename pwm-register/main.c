// fork from https://github.com/cnlohr/ch32v003fun/blob/master/examples/tim1_pwm/tim1_pwm.c
/*
 * Example for using Advanced Control Timer (TIM1) for PWM generation
 * 03-28-2023 E. Brombaugh
 */

#include "ch32fun.h"
#include <stdio.h>

/*
 * initialize TIM1 for PWM
 */
void t1pwm_init(void)
{
	// Enable GPIOC, GPIOD and TIM1
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC |
					  RCC_APB2Periph_TIM1;

	// PD0 is T1CH1N, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf << (4 * 0));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 0);

	// PC4 is T1CH4, 10MHz Output alt func, push-pull
	GPIOC->CFGLR &= ~(0xf << (4 * 4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 4);

	// Reset TIM1 to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_TIM1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_TIM1;

	// CTLR1: default is up, events generated, edge align
	// SMCFGR: default clk input is CK_INT

	// Prescaler
	TIM1->PSC = 0x0000;

	// Auto Reload - sets period
	TIM1->ATRLR = 255;

	// Reload immediately
	// 繰り返す
	TIM1->SWEVGR |= TIM_UG;

	// Enable CH1N output, positive pol
	// CH1Nの比較アウトプットを有効化
	TIM1->CCER |= TIM_CC1NE | TIM_CC1NP;

	// Enable CH4 output, positive pol
	// CH4の比較アウトプットを有効化
	TIM1->CCER |= TIM_CC4E | TIM_CC4P;

	// CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	// CH1をアウトプットで有効化
	TIM1->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1;

	// CH2 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	// CH2をアウトプットで有効化
	TIM1->CHCTLR2 |= TIM_OC4M_2 | TIM_OC4M_1;

	// Set the Capture Compare Register value to 50% initially
	TIM1->CH1CVR = 128;
	TIM1->CH4CVR = 128;

	// Enable TIM1 outputs
	TIM1->BDTR |= TIM_MOE;

	// Enable TIM1
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