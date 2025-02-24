#include "ch32fun.h"
#include <stdio.h>

void TIM1_UP_IRQHandler(void) __attribute__((interrupt));

uint32_t count;

void TIM1_INT_Init(uint16_t arr, uint16_t psc)
{
	// 高機能タイマー TIM1

	TIM1->CTLR1 = 0;
	TIM1->ATRLR = arr;
	TIM1->PSC = psc;
	TIM1->RPTCR = 10;

	TIM1->SWEVGR = TIM_PSCReloadMode_Immediate;

	// 割り込みフラグクリア
	TIM1->INTFR = (uint16_t)~TIM_IT_Update;

	// 割り込み設定
	// PreemptionPriority = 0, SubPriority = 1
	NVIC->IPRIOR[TIM1_UP_IRQn] = 0 << 7 | 1 << 6;
	// 有効化
	NVIC->IENR[((uint32_t)(TIM1_UP_IRQn) >> 5)] = (1 << ((uint32_t)(TIM1_UP_IRQn) & 0x1F));

	// 有効化
	TIM1->DMAINTENR |= TIM_IT_Update;
}

uint8_t ledState = 0;

void TIM1_UP_IRQHandler(void)
{
	// フラグ確認
	if (((TIM1->INTFR & TIM_IT_Update) != (uint16_t)0) && ((TIM1->DMAINTENR & TIM_IT_Update) != (uint16_t)0))
	{
		printf("tim\r\n");
	}

	// フラグを降ろす
	TIM1->INTFR = (uint16_t)~TIM_IT_Update;

	// メインスレッドで動くので何もしない
}

int main()
{
	SystemInit();

	printf("wait\r\n");

	Delay_Ms(5000);

	printf("init\r\n");

	// Enable Clock
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
	RCC->APB1PCENR |= RCC_APB1Periph_PWR;
	// RCC_LSICmd(ENABLE);
	RCC->RSTSCKR |= (1 << 0);

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

	// EXTI
	EXTI->INTENR &= ~EXTI_Line9;
	EXTI->EVENR &= ~EXTI_Line9;
	EXTI->RTENR &= ~EXTI_Line9;
	EXTI->FTENR &= ~EXTI_Line9;
	EXTI->EVENR |= EXTI_Line9;
	EXTI->FTENR |= EXTI_Line9;

	while (!(RCC->RSTSCKR & (1 << 1)))
		;

	// PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
	uint32_t tmp = PWR->AWUPSC & AWUPSC_MASK;
	tmp |= PWR_AWU_Prescaler_10240;
	PWR->AWUPSC = tmp;
	// PWR_AWU_SetWindowValue(25);
	tmp = 0;
	tmp = PWR->AWUWR & AWUWR_MASK;
	tmp |= 25;
	PWR->AWUWR = tmp;
	// PWR_AutoWakeUpCmd(ENABLE);
	PWR->AWUCSR |= (1 << 1);

	printf("start\r\n");

	uint32_t count = 0;

	while (1)
	{
		printf("go to stanby %ld\r\n\r\n", count);

		// PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);
		PWR->CTLR &= CTLR_DS_MASK;
		PWR->CTLR |= PWR_CTLR_PDDS;

		NVIC->SCTLR |= (1 << 2);

		__WFE();

		NVIC->SCTLR &= ~(1 << 2);

		printf("awake %ld\r\n", count);

		if (ledState)
		{
			GPIOC->BSHR = 1 << 1;
		}
		else
		{
			GPIOC->BSHR = (1 << (16 + 1));
		}
		ledState ^= 1;

		count++;
	}
}
