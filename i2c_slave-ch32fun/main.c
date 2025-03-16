#include "ch32fun.h"
#include "i2c_slave.h"
#include <stdio.h>
#include <stdbool.h>

#define COL1 GPIOv_from_PORT_PIN(GPIO_port_D, 0)
#define COL2 GPIOv_from_PORT_PIN(GPIO_port_D, 3)
#define COL3 GPIOv_from_PORT_PIN(GPIO_port_D, 2)
#define COL4 GPIOv_from_PORT_PIN(GPIO_port_C, 7)
#define COL5 GPIOv_from_PORT_PIN(GPIO_port_D, 4)
#define COL6 GPIOv_from_PORT_PIN(GPIO_port_D, 5)
#define COL7 GPIOv_from_PORT_PIN(GPIO_port_D, 6)

#define ROW1 GPIOv_from_PORT_PIN(GPIO_port_C, 3)
#define ROW2 GPIOv_from_PORT_PIN(GPIO_port_C, 4)
#define ROW3 GPIOv_from_PORT_PIN(GPIO_port_C, 5)
#define ROW4 GPIOv_from_PORT_PIN(GPIO_port_A, 1)
#define ROW5 GPIOv_from_PORT_PIN(GPIO_port_C, 6)

#define I2C_ADDRESS 0x70

#define COLS_SIZE 7
#define ROWS_SIZE 5

volatile uint8_t i2c_registers[32] = {0x00};

uint8_t on_write_available = 0;
uint8_t on_write_reg = 0;
uint8_t on_write_length = 0;

void on_write(uint8_t reg, uint8_t length)
{
	on_write_reg = reg;
	on_write_length = length;
	on_write_available += 1;
}

uint8_t on_read_available = 0;
uint8_t on_read_reg = 0;

void on_read(uint8_t reg)
{
	on_read_reg = reg;
	on_read_available += 1;
}

int main()
{
	SystemInit();
	funGpioInitAll();

	funPinMode(PC1, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SDA
	funPinMode(PC2, GPIO_CFGLR_OUT_10Mhz_AF_OD); // SCL
	SetupI2CSlave(I2C_ADDRESS, i2c_registers, sizeof(i2c_registers), on_write, on_read, false);

	Delay_Ms(1);

	for (int i = 0x20; i < 0x20; i++)
	{
		i2c_registers[i] = 0x10 * i;
	}
	printf("Start!\r\n");

	while (1)
	{
		if (on_write_available > 0)
		{
			printf("on_write: count=%d, reg=%2x, length=%d\r\n", on_write_available, on_write_reg, on_write_length);
			printf("reg[0x10]: %02x %02x %02x %02x\r\n", i2c_registers[0x10], i2c_registers[0x11], i2c_registers[0x12], i2c_registers[0x13]);

			on_write_available = 0;
		}
		if (on_read_available > 0)
		{
			printf("on_read: count=%d reg=%2x\r\n", on_read_available, on_read_reg);
			printf("reg[0x20]: %02x %02x %02x %02x\r\n", i2c_registers[0x20], i2c_registers[0x21], i2c_registers[0x22], i2c_registers[0x23]);

			on_read_available = 0;
		}
	}
}