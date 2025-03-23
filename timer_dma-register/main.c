#include "ch32fun.h"
#include <stdio.h>

#define SEGMENT_LED_NUM 4
#define ANODE_NUM 8

typedef struct
{
	GPIO_TypeDef *gpio;
	uint8_t pin_no;
	uint8_t pin_no_bit;
	uint32_t *buffer;
} PinGPIOMap;

uint32_t gpioa_bshr_buf[SEGMENT_LED_NUM] = {0};
uint32_t gpioc_bshr_buf[SEGMENT_LED_NUM] = {0};
uint32_t gpiod_bshr_buf[SEGMENT_LED_NUM] = {0};

PinGPIOMap anode_pins[ANODE_NUM] = {
	{GPIOC, 4, GPIO_Pin_4, gpioc_bshr_buf}, // SA
	{GPIOC, 3, GPIO_Pin_3, gpioc_bshr_buf}, // SB
	{GPIOD, 3, GPIO_Pin_3, gpiod_bshr_buf}, // SC
	{GPIOD, 2, GPIO_Pin_2, gpiod_bshr_buf}, // SD
	{GPIOC, 7, GPIO_Pin_7, gpioc_bshr_buf}, // SE
	{GPIOC, 5, GPIO_Pin_5, gpioc_bshr_buf}, // SF
	{GPIOC, 6, GPIO_Pin_6, gpioc_bshr_buf}, // SG
	{GPIOA, 1, GPIO_Pin_1, gpioa_bshr_buf}, // SDP
};

PinGPIOMap cathode_common_pins[SEGMENT_LED_NUM] = {
	{GPIOD, 6, GPIO_Pin_6, gpiod_bshr_buf}, // SEL1
	{GPIOD, 5, GPIO_Pin_5, gpiod_bshr_buf}, // SEL2
	{GPIOD, 4, GPIO_Pin_4, gpiod_bshr_buf}, // SEL3
	{GPIOC, 0, GPIO_Pin_0, gpioc_bshr_buf}, // SEL4
};

#define SEGPATTERN_0 0b00111111
#define SEGPATTERN_1 0b00000110
#define SEGPATTERN_2 0b01011011
#define SEGPATTERN_3 0b01001111
#define SEGPATTERN_4 0b01100110
#define SEGPATTERN_5 0b01101101
#define SEGPATTERN_6 0b01111101
#define SEGPATTERN_7 0b00000111
#define SEGPATTERN_8 0b01111111
#define SEGPATTERN_9 0b01101111
#define SEGPATTERN_d 0b10000000

void init()
{
	printf("init\r\n");

	// Enable GPIOs
	RCC->AHBPCENR |= RCC_AHBPeriph_SRAM | RCC_AHBPeriph_DMA1;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1;

	for (int i = 0; i < sizeof(anode_pins) / sizeof(PinGPIOMap); i++)
	{
		anode_pins[i].gpio->CFGLR &= ~(0xf << (4 * anode_pins[i].pin_no));
		anode_pins[i].gpio->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * anode_pins[i].pin_no);
	}

	for (int i = 0; i < sizeof(cathode_common_pins) / sizeof(PinGPIOMap); i++)
	{
		cathode_common_pins[i].gpio->CFGLR &= ~(0xf << (4 * cathode_common_pins[i].pin_no));
		cathode_common_pins[i].gpio->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * cathode_common_pins[i].pin_no);
	}

	DMA1_Channel2->CNTR = sizeof(gpioa_bshr_buf) / sizeof(gpioa_bshr_buf[0]);
	DMA1_Channel2->MADDR = (uint32_t)gpioa_bshr_buf;
	DMA1_Channel2->PADDR = (uint32_t)&GPIOA->BSHR; // This is the output register for out buffer.
	DMA1_Channel2->CFGR = DMA_CFGR1_DIR |		   // MEM2PERIPHERAL
						  DMA_CFGR1_PL |		   // High priority.
						  DMA_CFGR1_MSIZE_1 |	   // 8-bit memory
						  DMA_CFGR1_PSIZE_1 |	   // 8-bit peripheral
						  DMA_CFGR1_MINC |		   // Increase memory.
						  DMA_CFGR1_CIRC |		   // Circular mode.
						  DMA_CFGR1_EN;			   // Enable
	DMA1_Channel2->CFGR |= DMA_CFGR1_EN;

	DMA1_Channel3->CNTR = sizeof(gpioc_bshr_buf) / sizeof(gpioc_bshr_buf[0]);
	DMA1_Channel3->MADDR = (uint32_t)gpioc_bshr_buf;
	DMA1_Channel3->PADDR = (uint32_t)&GPIOC->BSHR; // This is the output register for out buffer.
	DMA1_Channel3->CFGR = DMA_CFGR1_DIR |		   // MEM2PERIPHERAL
						  DMA_CFGR1_PL |		   // High priority.
						  DMA_CFGR1_MSIZE_1 |	   // 8-bit memory
						  DMA_CFGR1_PSIZE_1 |	   // 8-bit peripheral
						  DMA_CFGR1_MINC |		   // Increase memory.
						  DMA_CFGR1_CIRC |		   // Circular mode.
						  DMA_CFGR1_EN;			   // Enable
	DMA1_Channel3->CFGR |= DMA_CFGR1_EN;

	DMA1_Channel6->CNTR = sizeof(gpiod_bshr_buf) / sizeof(gpiod_bshr_buf[0]);
	DMA1_Channel6->MADDR = (uint32_t)gpiod_bshr_buf;
	DMA1_Channel6->PADDR = (uint32_t)&GPIOD->BSHR; // This is the output register for out buffer.
	DMA1_Channel6->CFGR = DMA_CFGR1_DIR |		   // MEM2PERIPHERAL
						  DMA_CFGR1_PL |		   // High priority.
						  DMA_CFGR1_MSIZE_1 |	   // 8-bit memory
						  DMA_CFGR1_PSIZE_1 |	   // 8-bit peripheral
						  DMA_CFGR1_MINC |		   // Increase memory.
						  DMA_CFGR1_CIRC |		   // Circular mode.
						  DMA_CFGR1_EN;			   // Enable
	DMA1_Channel6->CFGR |= DMA_CFGR1_EN;

	RCC->APB2PRSTR = RCC_APB2Periph_TIM1; // Reset Timer
	RCC->APB2PRSTR = 0;

	TIM1->PSC = 480;											   // Prescaler 10us
	TIM1->ATRLR = 10 - 1;										   // 100us
	TIM1->SWEVGR = TIM_UG | TIM_TG;								   // Reload immediately + Trigger DMA
	TIM1->CCER = TIM_CC1E | TIM_CC1P;							   // Enable CH1 output, positive pol
	TIM1->CHCTLR1 = TIM_OC1M_2 | TIM_OC1M_1;					   // CH1 Mode is output, PWM1 (CC1S = 00, OC1M = 110)
	TIM1->CH1CVR = 6;											   // Set the Capture Compare Register value to 50% initially
	TIM1->BDTR = TIM_MOE;										   // Enable TIM1 outputs
	TIM1->CTLR1 = TIM_CEN;										   // Enable TIM1
	TIM1->DMAINTENR = TIM_UDE | TIM_CC1DE | TIM_CC2DE | TIM_CC3DE; // Trigger DMA on TC match 1 (DMA Ch2) and TC match 2 (DMA Ch3)

	printf("RCC->AHBPCENR, 0x%lx\r\n", RCC->AHBPCENR);
	printf("RCC->APB2PCENR, 0x%lx\r\n", RCC->APB2PCENR);
	printf("GPIOC->CFGLR, 0x%lx\r\n", GPIOC->CFGLR);
	printf("DMA1_Channel2->CFGR, 0x%lx\r\n", DMA1_Channel2->CFGR);
	printf("DMA1_Channel2->CNTR, 0x%lx\r\n", DMA1_Channel2->CNTR);
	printf("DMA1_Channel2->MADDR, 0x%lx\r\n", DMA1_Channel2->MADDR);
	printf("DMA1_Channel2->PADDR, 0x%lx\r\n", DMA1_Channel2->PADDR);
	printf("TIM1->CTLR1, 0x%x\r\n", TIM1->CTLR1);
	printf("TIM1->CTLR2, 0x%x\r\n", TIM1->CTLR2);
	printf("TIM1->SMCFGR, 0x%x\r\n", TIM1->SMCFGR);
	printf("TIM1->DMAINTENR, 0x%x\r\n", TIM1->DMAINTENR);
	printf("TIM1->INTFR, 0x%x\r\n", TIM1->INTFR);
	printf("TIM1->SWEVGR, 0x%x\r\n", TIM1->SWEVGR);
	printf("TIM1->CHCTLR1, 0x%x\r\n", TIM1->CHCTLR1);
	printf("TIM1->CHCTLR2, 0x%x\r\n", TIM1->CHCTLR2);
	printf("TIM1->CCER, 0x%x\r\n", TIM1->CCER);
	printf("TIM1->PSC, 0x%x\r\n", TIM1->PSC);
	printf("TIM1->ATRLR, 0x%x\r\n", TIM1->ATRLR);
	printf("TIM1->RPTCR, 0x%x\r\n", TIM1->RPTCR);
	printf("TIM1->CH1CVR, 0x%lx\r\n", TIM1->CH1CVR);
	printf("TIM1->CH2CVR, 0x%lx\r\n", TIM1->CH2CVR);
	printf("TIM1->CH3CVR, 0x%lx\r\n", TIM1->CH3CVR);
	printf("TIM1->CH4CVR, 0x%lx\r\n", TIM1->CH4CVR);
	printf("TIM1->BDTR, 0x%x\r\n", TIM1->BDTR);
	printf("TIM1->DMACFGR, 0x%x\r\n", TIM1->DMACFGR);
	printf("TIM1->DMAADR, 0x%x\r\n", TIM1->DMAADR);
}

int main()
{
	SystemInit();

	for (int b = 0; b < SEGMENT_LED_NUM; b++)
	{
		gpioa_bshr_buf[b] = 0;
		gpioc_bshr_buf[b] = 0;
		gpiod_bshr_buf[b] = 0;

		// 点灯対象のLEDのカソードをLow、それ以外をHighにする
		for (int j = 0; j < SEGMENT_LED_NUM; j++)
		{
			if (b == j)
			{
				cathode_common_pins[j].buffer[b] |= cathode_common_pins[j].pin_no_bit << 16;
			}
			else
			{
				cathode_common_pins[j].buffer[b] |= cathode_common_pins[j].pin_no_bit;
			}
		}

		uint8_t pattern = 0;
		switch (b)
		{
		case 0:
			pattern = SEGPATTERN_0;
			break;
		case 1:
			pattern = SEGPATTERN_1;
			break;
		case 2:
			pattern = SEGPATTERN_2;
			break;
		case 3:
			pattern = SEGPATTERN_3;
			break;
		}

		for (int j = 0; j < ANODE_NUM; j++)
		{
			if (pattern & (1 << j))
			{
				// 点灯
				anode_pins[j].buffer[b] |= anode_pins[j].pin_no_bit;
			}
			else
			{
				// 消灯
				anode_pins[j].buffer[b] |= anode_pins[j].pin_no_bit << 16;
			}
		}
	}

	init();

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		Delay_Ms(1000);
		printf("Step %d %d\n", TIM1->CNT, DMA1_Channel2->CNTR);
	}
}