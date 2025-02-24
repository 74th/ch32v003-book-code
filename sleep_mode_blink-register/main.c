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
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

	// C0=L
	GPIOC->BSHR = (1 << (16 + 1));

	PFIC->SCTLR &= ~(1 << 2);	// Sleep
	PWR->CTLR &= PWR_CTLR_PDDS; // Sleep Mode

	Delay_Ms(100);

	TIM1_INT_Init(100 - 1, 48000 - 1);
	// Enable
	TIM1->CTLR1 |= TIM_CEN;

	printf("start\r\n");

	uint32_t count = 0;

	while (1)
	{
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

		printf("go to sleep %ld\r\n", count);

		// __WFI();
		NVIC->SCTLR &= ~(1 << 3);
		__ASM volatile("wfi");

		count++;
	}
}
