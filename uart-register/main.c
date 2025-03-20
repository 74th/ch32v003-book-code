#include "ch32fun.h"
#include <stdio.h>

#define I2C_SLAVE_ADDRESS 0x44
#define LOOP_MS 1000

uint8_t CMD_READ_CO2_CONNECTION[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
uint8_t CMD_TURN_ON_SELF_CALIBRATION[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};

void init_rcc(void)
{
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;
}

void setup_uart()
{
	// Push-Pull, 10MHz Output, GPIO D5, with AutoFunction
	GPIOD->CFGLR &= ~(0xf << (4 * 5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5);
	GPIOD->CFGLR &= ~(0xf << (4 * 6));
	GPIOD->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING) << (4 * 6);

	// 115200, 8n1.  Note if you don't specify a mode, UART remains off even when UE_Set.
	USART1->CTLR1 = USART_WordLength_8b | USART_Parity_No | USART_Mode_Tx | USART_Mode_Rx;
	USART1->CTLR2 = USART_StopBits_1;
	USART1->CTLR3 = USART_HardwareFlowControl_None;

	USART1->BRR = (((FUNCONF_SYSTEM_CORE_CLOCK) + (9600) / 2) / (9600));
	USART1->CTLR1 |= CTLR1_UE_Set;
}

int main()
{
	SystemInit();
	init_rcc();

	printf("init\r\n");

	setup_uart();

	printf("setup\r\n");

	Delay_Ms(100);

	for (int i = 0; i < sizeof(CMD_TURN_ON_SELF_CALIBRATION); i++)
	{
		while (!(USART1->STATR & USART_FLAG_TC))
			;
		USART1->DATAR = CMD_TURN_ON_SELF_CALIBRATION[i];
	}

	printf("test i2c done\r\n");

	uint32_t loop_count = 0;

	while (1)
	{
		uint8_t read_buf[9] = {0};
		printf("loop count: %u\r\n", loop_count);

		for (int i = 0; i < sizeof(CMD_READ_CO2_CONNECTION); i++)
		{
			while (!(USART1->STATR & USART_FLAG_TC))
				;
			USART1->DATAR = CMD_READ_CO2_CONNECTION[i];
		}

		for (int i = 0; i < 9; i++)
		{
			while (!(USART1->STATR & USART_FLAG_RXNE))
				;
			read_buf[i] = USART1->DATAR;
		}

		if (read_buf[0] != 0xFF)
		{
			printf("invalid start byte\r\n");
			Delay_Ms(1000);
			continue;
		}

		uint8_t c = 0;
		for (int i = 1; i <= 7; i++)
		{

			c += read_buf[i];
		}
		if (0xff - c + 1 != read_buf[8])
		{
			printf("checksum error\r\n");
			Delay_Ms(1000);
			continue;
		}

		// for (int i = 0; i < 9; i++)
		// {
		// 	printf("0x%02X ", read_buf[i]);
		// }
		// printf("\r\n");

		uint16_t co2_ppm = (read_buf[2] << 8) | read_buf[3];

		printf("CO2: %u ppm\r\n", co2_ppm);

		Delay_Ms(1000);
		loop_count++;
	}
}
