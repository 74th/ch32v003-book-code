#include "ch32fun.h"
#include <stdio.h>

#define I2C_ADDRESS 0x10

void I2C1_EV_IRQHandler(void) __attribute__((interrupt));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt));

volatile uint8_t i2c_registers[0x30] = {0x00};
volatile uint8_t i2c_start_position = 0;
volatile uint8_t i2c_position = 0;
volatile uint8_t i2c_first_receive = 0;
uint8_t i2c_request_available = 0;
uint8_t i2c_receive_available = 0;

void I2C1_EV_IRQHandler(void)
{
	uint16_t STAR1, STAR2 __attribute__((unused));
	STAR1 = I2C1->STAR1;
	STAR2 = I2C1->STAR2;

	I2C1->CTLR1 |= I2C_CTLR1_ACK;

	if (STAR1 & I2C_STAR1_ADDR) // 0x0002
	{
		// 最初のイベント
		i2c_first_receive = 1;
	}

	if (STAR1 & I2C_STAR1_RXNE) // 0x0040
	{
		// 1byte の受信イベント（master -> slave）
		uint8_t v = I2C1->DATAR;
		if (i2c_first_receive)
		{
			// 最初の1バイトはレジスタアドレスとする
			i2c_start_position = v;
			i2c_position = v;
			i2c_first_receive = 0;
		}
		else if (i2c_position < sizeof(i2c_registers))
		{
			// 2バイト目以降
			i2c_registers[i2c_position] = v;
			i2c_position++;
			i2c_receive_available += 1;
		}
		else
		{
			// 何もしない
		}
	}

	if (STAR1 & I2C_STAR1_TXE) // 0x0080
	{
		// 1byte の送信イベント（slave -> master）
		if (i2c_position < sizeof(i2c_registers))
		{
			I2C1->DATAR = i2c_registers[i2c_position];
			i2c_position++;
			i2c_request_available += 1;
		}
		else
		{
			// ゼロ値を送る
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

	// I2Cモジュール初期化のためにI2C1をリセット
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

	I2C1->CTLR1 |= I2C_CTLR1_SWRST;
	I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

	// I2Cモジュールクロック周波数設定
	uint32_t prerate = 2000000; // I2C Logic clock rate, must be higher than the bus clock rate
	I2C1->CTLR2 |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;

	// 割り込み
	I2C1->CTLR2 |= I2C_CTLR2_ITBUFEN;
	I2C1->CTLR2 |= I2C_CTLR2_ITEVTEN; // イベント割り込み
	I2C1->CTLR2 |= I2C_CTLR2_ITERREN; // エラー割り込み
	NVIC_EnableIRQ(I2C1_EV_IRQn);	  // イベント割り込み
	NVIC_SetPriority(I2C1_EV_IRQn, 2 << 4);
	NVIC_EnableIRQ(I2C1_ER_IRQn); // エラー割り込み

	// I2Cクロック設定
	uint32_t clockrate = 1000000;
	I2C1->CKCFGR = ((FUNCONF_SYSTEM_CORE_CLOCK / (3 * clockrate)) & I2C_CKCFGR_CCR) | I2C_CKCFGR_FS; // Fast mode 33% duty cycle

	// I2Cアドレス設定
	I2C1->OADDR1 = I2C_ADDRESS << 1;

	// I2C有効化
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// ACK有効化
	I2C1->CTLR1 |= I2C_CTLR1_ACK;

	for (int i = 0; i < 4; i++)
	{
		i2c_registers[0x20 + i] = 0x10 * i;
	}

	// printf("I2C1->CTLR1: %4x\r\n", I2C1->CTLR1);
	// printf("I2C1->CTLR2: %4x\r\n", I2C1->CTLR2);
	// printf("I2C1->CKCFGR: %4x\r\n", I2C1->CKCFGR);
	// printf("I2C1->OADDR1: %4x\r\n", I2C1->OADDR1);

	printf("start\r\n");

	while (1)
	{
		if (i2c_request_available > 0)
		{
			printf("request: count=%d, start=%x, length=%d\r\n", i2c_request_available, i2c_start_position, i2c_position - i2c_start_position);
			printf("reg[0x20]: %x %x %x %x\r\n", i2c_registers[0x20], i2c_registers[0x21], i2c_registers[0x22], i2c_registers[0x23]);
			i2c_request_available = 0;
		}

		if (i2c_receive_available > 0)
		{
			printf("receive: count=%d, start=%x, length=%d\r\n", i2c_receive_available, i2c_start_position, i2c_position - i2c_start_position);
			printf("reg[0x10]: %x %x %x %x\r\n", i2c_registers[0x10], i2c_registers[0x11], i2c_registers[0x12], i2c_registers[0x13]);
			i2c_receive_available = 0;
		}
	}
}