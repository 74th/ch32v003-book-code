#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

int main()
{
	SystemInit();

	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC;

	// GPIO C0 Push-Pull
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

	while (1)
	{
		GPIOC->BSHR = 1 << 1;
		printf("+%lu\n", count++);
		Delay_Ms(500);
		GPIOC->BSHR = (1 << (16 + 1));
		printf("-%lu\n", count++);
		Delay_Ms(500);
	}
}
