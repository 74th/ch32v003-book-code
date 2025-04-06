#include "ch32fun.h"
#include <stdio.h>

#define USE_REMAP 1

#define TIMEOUT_MAX 100000

uint8_t CMD_READ_CO2_CONNECTION[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
uint8_t CMD_TURN_ON_SELF_CALIBRATION[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};

void init_rcc(void)
{
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1;
#if USE_REMAP
	RCC->APB2PCENR |= RCC_APB2Periph_AFIO;
#endif
}

void setup_uart()
{
#if USE_REMAP
	// REMAPの有効化
	AFIO->PCFR1 &= ~(AFIO_PCFR1_USART1_REMAP | AFIO_PCFR1_USART1_HIGH_BIT_REMAP);
	AFIO->PCFR1 |= AFIO_PCFR1_USART1_REMAP | AFIO_PCFR1_USART1_HIGH_BIT_REMAP;

	// PC0: TX
	GPIOC->CFGLR &= ~(0xf << (4 * 0));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 0);
	// PC1: RX
	GPIOC->CFGLR &= ~(0xf << (4 * 1));
	GPIOC->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING) << (4 * 1);
#else
	// PD5: TX
	GPIOD->CFGLR &= ~(0xf << (4 * 5));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF) << (4 * 5);
	// PD6: RX
	GPIOD->CFGLR &= ~(0xf << (4 * 6));
	GPIOD->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_FLOATING) << (4 * 6);
#endif

	// 115200, 8n1.  Note if you don't specify a mode, UART remains off even when UE_Set.
	USART1->CTLR1 =
		// バイト長 8bit
		USART_WordLength_8b |
		// パリティなし
		USART_Parity_No |
		// TX有効化
		USART_Mode_Tx |
		// RX有効化
		USART_Mode_Rx;
	// ストップビット 1
	USART1->CTLR2 = USART_StopBits_1;
	// フロー制御なし
	USART1->CTLR3 = USART_HardwareFlowControl_None;
	// ボーレート 9600
	USART1->BRR = (((FUNCONF_SYSTEM_CORE_CLOCK) + (9600) / 2) / (9600));
	// 有効化
	USART1->CTLR1 |= CTLR1_UE_Set;
}

uint16_t read_uart_with_timeout(uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		int32_t timeout = TIMEOUT_MAX;
		// 受信完了まで待つ
		while (timeout-- && !(USART1->STATR & USART_FLAG_RXNE))
			;

		if (timeout < 0)
		{

			return i;
		}

		// 受信データを読み込む
		buf[i] = USART1->DATAR;
	}

	return len;
}

void write_uart(uint8_t *buf, uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		// 準備完了まで待つ
		while (!(USART1->STATR & USART_FLAG_TC))
			;

		// 送信データをセット
		USART1->DATAR = buf[i];

		// 送信完了まで待つ
		while (!(USART1->STATR & USART_FLAG_TXE))
			;
	}
}

int loop(uint32_t loop_count)
{
	uint8_t read_buf[9] = {0};
	printf("loop %d\r\n", loop_count++);

	write_uart(CMD_READ_CO2_CONNECTION, sizeof(CMD_READ_CO2_CONNECTION));

	uint16_t read_len = 0;

	// 0xff 0x86 が読めるまで読み進める
	while (1)
	{
		read_len = read_uart_with_timeout(&read_buf[0], 1);
		if (read_len == 0)
		{
			printf("read timeout\r\n");
			Delay_Ms(1000);

			return 1;
		}
		if (read_buf[0] != 0xff)
		{
			// printf("invalid start byte: %0x\r\n", read_buf[0]);
			continue;
		}

		read_len = read_uart_with_timeout(&read_buf[1], 1);
		if (read_len == 0)
		{
			printf("read timeout\r\n");
			Delay_Ms(1000);
			return 1;
		}
		if (read_buf[1] != 0x86)
		{
			// printf("invalid command byte: %02x\r\n", read_buf[1]);
			continue;
		}

		break;
	}

	read_len = read_uart_with_timeout(&read_buf[2], 7);
	if (read_len < 7)
	{
		printf("timeout\r\n");
		Delay_Ms(1000);
	}

	// for (int i = 0; i < sizeof(read_buf); i++)
	// {
	//     printf("0x%02X ", read_buf[i]);
	// }
	// printf("\r\n");

	// チェックサム
	uint8_t c = 0;
	for (int i = 1; i < 8; i++)
	{
		c += read_buf[i];
	}
	if (0xff - c + 1 != read_buf[8])
	{
		printf("invalid checksum\r\n");
	}

	uint16_t co2 = read_buf[2] * 256 + read_buf[3];
	printf("CO2: %5d ppm\r\n", co2);

	return 0;
}

int main()
{
	SystemInit();
	init_rcc();

	printf("init\r\n");

	setup_uart();

	printf("setup\r\n");

	Delay_Ms(100);

	// キャリブレーション命令
	write_uart(CMD_TURN_ON_SELF_CALIBRATION, sizeof(CMD_TURN_ON_SELF_CALIBRATION));

	Delay_Ms(1);

	// キャリブレーション命令のレスポンスを読み捨てる
	uint8_t read_buf[9] = {0};
	read_uart_with_timeout(read_buf, 9);

	printf("test i2c done\r\n");

	uint32_t loop_count = 0;

	while (1)
	{
		loop(loop_count++);
		Delay_Ms(1000);
	}
}
