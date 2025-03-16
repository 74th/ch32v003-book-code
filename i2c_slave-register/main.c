#include "ch32fun.h"
#include <stdio.h>

#define I2C_ADDRESS 0x10

void I2C1_EV_IRQHandler(void) __attribute__((interrupt));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt));

volatile uint8_t i2c_registers[0x30] = {0x00};
volatile uint8_t i2c_position = 0;
volatile uint8_t i2c_first_receive = 0;

void I2C1_EV_IRQHandler(void)
{
	uint16_t STAR1, STAR2 __attribute__((unused));
	STAR1 = I2C1->STAR1;
	STAR2 = I2C1->STAR2;

	// #ifdef FUNCONF_USE_UARTPRINTF
	// 	printf("EV STAR1: 0x%04x STAR2: 0x%04x\r\n", STAR1, STAR2);
	// #endif

	I2C1->CTLR1 |= I2C_CTLR1_ACK;

	if (STAR1 & I2C_STAR1_ADDR) // 0x0002
	{
		// #ifdef FUNCONF_USE_UARTPRINTF
		// 		printf("ADDR\r\n");
		// #endif
		// 最初のイベント
		// read でも write でも必ず最初に呼ばれる
		i2c_position = 0;
	}

	if (STAR1 & I2C_STAR1_RXNE) // 0x0040
	{
		// #ifdef FUNCONF_USE_UARTPRINTF
		// 		printf("RXNE write event: pos:%d\r\n", i2c_position);
		// #endif
		// 1byte 受信
		I2C1->DATAR;
	}

	if (STAR1 & I2C_STAR1_TXE) // 0x0080
	{
		// 1byte の read イベント（slave -> master）
		// #ifdef FUNCONF_USE_UARTPRINTF
		// 		printf("TXE write event: pos:%d\r\n", i2c_position);
		// #endif
		if (i2c_position < 5)
		{
			// 1byte 送信
			uint8_t data = 0x74;
			I2C1->DATAR = data;
			i2c_position++;
		}
		else
		{
			// 1byte 送信
			I2C1->DATAR = 0x00;
		}
	}
}

void I2C1_ER_IRQHandler(void)
{
	uint16_t STAR1 = I2C1->STAR1;

	if (STAR1 & I2C_STAR1_BERR)			  // 0x0100
	{									  // Bus error
		I2C1->STAR1 &= ~(I2C_STAR1_BERR); // Clear error
	}

	if (STAR1 & I2C_STAR1_ARLO)			  // 0x0200
	{									  // Arbitration lost error
		I2C1->STAR1 &= ~(I2C_STAR1_ARLO); // Clear error
	}

	if (STAR1 & I2C_STAR1_AF)			// 0x0400
	{									// Acknowledge failure
		I2C1->STAR1 &= ~(I2C_STAR1_AF); // Clear error
	}
}

int main()
{
	SystemInit();
	printf("init\r\n");

	RCC->CFGR0 &= ~(0x1F << 11);

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_ADC1;
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

	// PC1 is SDA, 10MHz Output, alt func, open-drain
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * 1);

	// PC2 is SCL, 10MHz Output, alt func, open-drain
	GPIOC->CFGLR &= ~(0xf << (4 * 2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * 2);

	// Reset I2C1 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

	I2C1->CTLR1 |= I2C_CTLR1_SWRST;
	I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

	// Set module clock frequency
	uint32_t prerate = 2000000; // I2C Logic clock rate, must be higher than the bus clock rate
	I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;

	// Enable interrupts
	I2C1->CTLR2 |= I2C_CTLR2_ITBUFEN;
	I2C1->CTLR2 |= I2C_CTLR2_ITEVTEN; // Event interrupt
	I2C1->CTLR2 |= I2C_CTLR2_ITERREN; // Error interrupt

	NVIC_EnableIRQ(I2C1_EV_IRQn); // Event interrupt
	NVIC_SetPriority(I2C1_EV_IRQn, 2 << 4);
	NVIC_EnableIRQ(I2C1_ER_IRQn); // Error interrupt

	// Set clock configuration
	uint32_t clockrate = 1000000;																	 // I2C Bus clock rate, must be lower than the logic clock rate
	I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK / (3 * clockrate)) & I2C_CKCFGR_CCR) | I2C_CKCFGR_FS; // Fast mode 33% duty cycle

	// Set I2C address
	I2C1->OADDR1 = I2C_ADDRESS << 1;

	// Enable I2C
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// Acknowledge the first address match event when it happens
	I2C1->CTLR1 |= I2C_CTLR1_ACK;

	printf("start\r\n");

	while (1)
	{
		// loop();
		Delay_Ms(100);
		// raw_adc_test();
		// Delay_Ms(500);
	}
}