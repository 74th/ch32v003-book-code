#include "ch32fun.h"
#include <stdio.h>
#include "ch32v003_SPI.h"

int main()
{
	SystemInit();

	printf("init\r\n");

#ifdef CH32V003_SPI_NSS_SOFTWARE_ANY_MANUAL
	funGpioInitC();

	funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_PP);
	funDigitalWrite(PC1, FUN_HIGH);
#endif

	SPI_init();

	printf("setup\r\n");

#ifdef CH32V003_SPI_NSS_SOFTWARE_ANY_MANUAL
	funDigitalWrite(PC1, FUN_LOW);
#endif

	SPI_begin_8();

	SPI_transfer_8(0xff);
	SPI_transfer_8(0xff);
	SPI_transfer_8(0xff);
	SPI_transfer_8(0xff);

	Delay_Ms(100);

	SPI_transfer_8(0x54);

	Delay_Ms(500);

	SPI_end();

	while (1)
	{
		printf("loop\r\n");

		SPI_begin_8();

		uint8_t b1 = SPI_transfer_8(0x00);
		uint8_t b2 = SPI_transfer_8(0x00);
		uint16_t raw = b1 << 8 | b2;

		printf("raw: %04x\r\n", raw);

		// 13bit
		raw = raw >> 3;

		int32_t raw_int;

		if (raw & 0x1000)
		{
			raw_int = raw - 0x2000;
		}
		else
		{
			raw_int = raw;
		}

		printf("raw_int: %d\r\n", raw_int);
		int32_t int_temp = raw_int >> 4;
		int32_t frac_temp = raw_int & 0x0f;
		printf("temp:%3d.%04d\r\n", int_temp, frac_temp * 625);

		SPI_end();

		Delay_Ms(1000);
	}
}
