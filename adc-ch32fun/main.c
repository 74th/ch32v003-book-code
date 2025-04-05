#define USE_BRANCHLESS 0

#include "ch32fun.h"
#include <stdio.h>

#if USE_BRANCHLESS
#include "ch32v003_GPIO_branchless.h"
#endif

// ピンの定義
#if USE_BRANCHLESS
// PA1: A1
#define VRX_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 1)
// PA0: A2
#define VRY_PIN GPIOv_from_PORT_PIN(GPIO_port_A, 2)
#else
// PA1: A1
#define VRX_PIN PA1
// PA0: A2
#define VRY_PIN PA2
#endif

uint32_t count;

int main()
{
	SystemInit();

	printf("init\r\n");

#if USE_BRANCHLESS
	// GPIO有効化
	GPIO_port_enable(GPIO_port_C);
	GPIO_port_enable(GPIO_port_A);

	// PA1 アナログ入力
	GPIO_pinMode(VRX_PIN, GPIO_pinMode_I_analog, GPIO_Speed_In);
	GPIO_pinMode(VRY_PIN, GPIO_pinMode_I_analog, GPIO_Speed_In);

	// ADC初期化
	GPIO_ADCinit();

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		uint16_t x = GPIO_analogRead(GPIO_Ain1_A1);
		uint16_t y = GPIO_analogRead(GPIO_Ain0_A2);

		printf("x: %d, y: %d\r\n", x, y);

		Delay_Ms(500);
	}
#else
	// GPIO有効化
	funGpioInitA();
	funGpioInitC();

	// ピン設定
	funPinMode(VRX_PIN, GPIO_CNF_IN_ANALOG);
	funPinMode(VRY_PIN, GPIO_CNF_IN_ANALOG);

	// ADC初期化
	funAnalogInit();

	printf("start\r\n");

	int count = 0;

	while (1)
	{
		printf("loop %d\r\n", count++);

		// PA1: A1
		uint16_t x = funAnalogRead(1);
		// PA0: A2
		uint16_t y = funAnalogRead(0);

		printf("x: %d, y: %d\r\n", x, y);

		Delay_Ms(500);
	}
#endif
}
