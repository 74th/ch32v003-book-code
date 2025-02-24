#include "ch32fun.h"
#include <stdio.h>

#define SHT31_I2C_ADDR 0x44
#define I2C_SPEED 1000000
#define I2C_TIMEOUT_MAX 100000

uint32_t count;

void init_i2c()
{
	// Enable RCC
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
	RCC->APB1PCENR |= RCC_APB1Periph_I2C1;

	// PC1, PC2 for I2C SCL, SDA
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * 1);
	GPIOC->CFGLR &= ~(0xf << (4 * 2));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_OD_AF) << (4 * 2);

	// Reset I2C1 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_I2C1;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_I2C1;

	I2C1->CTLR1 |= I2C_CTLR1_SWRST;
	I2C1->CTLR1 &= ~I2C_CTLR1_SWRST;

	uint32_t prerate = 200000;
	uint32_t clkrate = 100000;

	uint16_t tempreg;

	// I2C_InitTSturcture.I2C_ClockSpeed = 100000;
	tempreg = I2C1->CTLR2;
	// tempreg &= CTLR2_FREQ_Reset;
	tempreg &= ~I2C_CTLR2_FREQ;
	tempreg |= (FUNCONF_SYSTEM_CORE_CLOCK / prerate) & I2C_CTLR2_FREQ;
	I2C1->CTLR2 = tempreg;

	// I2C1->CTLR1 &= CTLR1_PE_Reset;

	tempreg = 0;
	// I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
	tempreg = (FUNCONF_SYSTEM_CORE_CLOCK / (25 * clkrate)) & I2C_CKCFGR_CCR;
	tempreg |= I2C_CKCFGR_DUTY;
	tempreg |= I2C_CKCFGR_FS;
	// uint16_t result = (FUNCONF_SYSTEM_CORE_CLOCK / (I2C_SPEED * 3));
	// tempreg |= result | CKCFGR_FS_Set;
	I2C1->CKCFGR = tempreg;

	// tempreg = I2C1->CTLR1;
	// tempreg &= I2C_CTLR1_CLEAR_Mask;
	// I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
	// I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
	// tempreg |= I2C_Mode_I2C | I2C_Ack_Enable;
	// I2C1->CTLR1 = tempreg;

	// I2C_InitTSturcture.I2C_OwnAddress1 = SHT31_I2C_ADDR;
	// I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C1->OADDR1 = I2C_AcknowledgedAddress_7bit | SHT31_I2C_ADDR;

	// I2C_Cmd(I2C1, ENABLE);
	I2C1->CTLR1 |= I2C_CTLR1_PE;

	// I2C_AcknowledgeConfig(I2C1, ENABLE);
	I2C1->CTLR1 |= I2C_CTLR1_ACK;
}

uint8_t i2c_chk_evt(uint32_t event_mask)
{
	/* read order matters here! STAR1 before STAR2!! */
	uint32_t status = I2C1->STAR1 | (I2C1->STAR2 << 16);
	return (status & event_mask) == event_mask;
}

void send_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
	int32_t timeout;

	I2C1->CTLR1 |= I2C_CTLR1_ACK;

	printf("@@11\r\n");

	// wait for not busy
	timeout = I2C_TIMEOUT_MAX;
	// while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
	// 	;
	while ((I2C1->STAR2 & I2C_STAR2_BUSY) && (timeout--))
		;
	if (timeout < 0)
	{
		printf("i2c error: waiting for not BUSY is timeout\r\n");
		I2C1->CTLR1 |= I2C_CTLR1_STOP;
		return;
	}

	printf("@@12\r\n");
	// I2C_GenerateSTART(I2C1, ENABLE);
	I2C1->CTLR1 |= I2C_CTLR1_START;

	printf("@@13\r\n");
	// while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
	// 	;
	timeout = I2C_TIMEOUT_MAX;
	while ((!i2c_chk_evt(I2C_EVENT_MASTER_MODE_SELECT)) && (timeout--))
		;
	if (timeout == -1)
	{
		printf("i2c error: waiting for master select is timeout\r\n");
		I2C1->CTLR1 |= I2C_CTLR1_STOP;
		return;
	}

	printf("@@14\r\n");
	// I2C_Send7bitAddress(I2C1, address << 1, I2C_Direction_Transmitter);
	// I2C1->DATAR = (SHT31_I2C_ADDR << 1) & OADDR1_ADD0_Reset;
	I2C1->DATAR = (SHT31_I2C_ADDR << 1);

	printf("@@15\r\n");
	// while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
	// 	;
	timeout = I2C_TIMEOUT_MAX;
	while ((!i2c_chk_evt(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED)) && (timeout--))
		;
	if (timeout == -1)
	{
		printf("i2c error: waiting for transmit condition is timeout\r\n");
		I2C1->CTLR1 |= I2C_CTLR1_STOP;
		return;
	}

	for (int i = 0; i < length; i++)
	{
		printf("@@16\r\n");

		// wait for TX Empty
		timeout = I2C_TIMEOUT_MAX;
		while (!(I2C1->STAR1 & I2C_STAR1_TXE) && (timeout--))
			;
		if (timeout == -1)
		{
			printf("i2c error: waiting for data send is timeout\r\n");
			I2C1->CTLR1 |= I2C_CTLR1_STOP;
			return;
		}

		// I2C_SendData(I2C1, data[i]);
		I2C1->DATAR = data[i];
	}

	printf("@@17\r\n");

	// wait for tx complete
	timeout = I2C_TIMEOUT_MAX;
	while ((!i2c_chk_evt(I2C_EVENT_MASTER_BYTE_TRANSMITTED)) && (timeout--))
		;
	if (timeout == -1)
	{
		printf("i2c error: waiting for tx complete is timeout\r\n");
		I2C1->CTLR1 |= I2C_CTLR1_STOP;
		return;
	}

	printf("@@18\r\n");

	// I2C_GenerateSTOP(I2C1, ENABLE);
	I2C1->CTLR1 |= CTLR1_START_Reset;
}

int main()
{
	SystemInit();

	printf("init\r\n");

	init_i2c();

	printf("setup\r\n");

	uint8_t buf1[2] = {0x30, 0xA2};
	send_i2c_data(SHT31_I2C_ADDR, buf1, 2);
	Delay_Ms(300);

	uint8_t buf2[2] = {0x30, 0x41};
	send_i2c_data(SHT31_I2C_ADDR, buf2, 2);
	Delay_Ms(300);

	printf("start\r\n");

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
