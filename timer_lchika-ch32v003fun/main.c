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

	printf("TIM1->CTLR1: %x\r\n", TIM1->CTLR1);
	printf("TIM1->ATRLR: %x\r\n", TIM1->ATRLR);
	printf("TIM1->PSC: %x\r\n", TIM1->PSC);
	printf("TIM1->RPTCR: %x\r\n", TIM1->RPTCR);
	printf("TIM1->SWEVGR: %x\r\n", TIM1->SWEVGR);
	// TIM1->CTLR1: 0
	// TIM1->ATRLR: 63
	// TIM1->PSC: bb7f
	// TIM1->RPTCR: a
	// TIM1->SWEVGR: 0

	// 割り込みフラグクリア
	TIM1->INTFR = (uint16_t)~TIM_IT_Update;

	printf("TIM1->INTFR: %x\r\n", TIM1->INTFR);

	// 割り込み設定
	// PreemptionPriority = 0, SubPriority = 1
	NVIC->IPRIOR[TIM1_UP_IRQn] = 0 << 7 | 1 << 6;
	// 有効化
	NVIC->IENR[((uint32_t)(TIM1_UP_IRQn) >> 5)] = (1 << ((uint32_t)(TIM1_UP_IRQn) & 0x1F));
	printf("NVIC->IPRIOR[TIM1_UP_IRQn]: %x\r\n", NVIC->IPRIOR[(uint32_t)(TIM1_UP_IRQn)]);
	printf("NVIC->IENR[TIM1_UP_IRQn]:%x\r\n", NVIC->IENR[(uint32_t)(TIM1_UP_IRQn)]);
	// NVIC->IPRIOR[TIM1_UP_IRQn]: 40
	// NVIC->IENR[TIM1_UP_IRQn]:0

	// 有効化
	TIM1->DMAINTENR |= TIM_IT_Update;
	printf("TIM1->DMAINTENR: %x\r\n", TIM1->DMAINTENR);
	// TIM1->DMAINTENR : 0
}

uint8_t ledState = 0;

void TIM1_UP_IRQHandler(void)
{
	// フラグ確認
	if (((TIM1->INTFR & TIM_IT_Update) != (uint16_t)0) && ((TIM1->DMAINTENR & TIM_IT_Update) != (uint16_t)0))
	{
		printf("1sec event\r\n");
	}

	// フラグを降ろす
	TIM1->INTFR = (uint16_t)~TIM_IT_Update;

	if (ledState)
	{
		GPIOC->BSHR = 1 << 1;
	}
	else
	{
		GPIOC->BSHR = (1 << (16 + 1));
	}
	ledState ^= 1;
}

int main()
{
	SystemInit();

	// Enable Clock
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_TIM1;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

	// C0=L
	GPIOC->BSHR = (1 << (16 + 1));

	Delay_Ms(100);

	TIM1_INT_Init(100 - 1, 48000 - 1);
	// Enable
	TIM1->CTLR1 |= TIM_CEN;

	printf("start\r\n");

	while (1)
	{
	}
}
