#include <ch32v00x.h>
#include <debug.h>

#define BLINKY_GPIO_PORT GPIOC
#define BLINKY_GPIO_PIN GPIO_Pin_1

#define ENABLE_UART_PRINT 0

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void Delay_Init(void);
void Delay_Ms(uint32_t n);

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

#if ENABLE_UART_PRINT
    USART_Printf_Init(115200);

    printf("wait\r\n");
#endif

    Delay_Ms(5000);

#if ENABLE_UART_PRINT
    printf("init\r\n");
#endif

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_LSICmd(ENABLE);

    GPIO_InitStructure.GPIO_Pin = BLINKY_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_Init(BLINKY_GPIO_PORT, &GPIO_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;

    // LSI 128kHz
    // Prescaler 10240 -> 128kHz / 10240 = 12.5Hz = 80ms
    // window value 31
    // 80ms * 31 = 2000ms = 2s
    PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
    PWR_AWU_SetWindowValue(25);
    PWR_AutoWakeUpCmd(ENABLE);

    GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, 1);

#if ENABLE_UART_PRINT
    printf("start\r\n");
#endif

    int count = 0;
    uint8_t ledState = 0;

    while (1)
    {
#if ENABLE_UART_PRINT
        printf("go to standby: %d\r\n\r\n", count);
#endif

        PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);

        USART_Printf_Init(115200);

#if ENABLE_UART_PRINT
        printf("\r\nAuto wake up: %d \r\n", count);
#endif

        GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, ledState);
        ledState ^= 1;

        count++;
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}