#include <ch32v00x.h>
#include <debug.h>

#define BLINKY_GPIO_PORT GPIOC
#define BLINKY_GPIO_PIN GPIO_Pin_1
#define ENABLE_UART_PRINT 1

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void Delay_Init(void);
void Delay_Ms(uint32_t n);

uint8_t ledState = 0;

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
#if ENABLE_UART_PRINT == 1
    USART_Printf_Init(115200);
#endif
    Delay_Init();

#if ENABLE_UART_PRINT == 1
    printf("init\r\n");
#endif

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);

    GPIO_InitStructure.GPIO_Pin = BLINKY_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLINKY_GPIO_PORT, &GPIO_InitStructure);

    // PFIC->SCTLR &= ~(1 << 2);   // Sleep
    // PWR->CTLR &= PWR_CTLR_PDDS; // Sleep Mode

    GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, 0);

    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;

    PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
    PWR_AWU_SetWindowValue(25);
    PWR_AutoWakeUpCmd(ENABLE);

    Delay_Ms(100);

#if ENABLE_UART_PRINT == 1
    printf("start\r\n");
#endif

    while (1)
    {

#if ENABLE_UART_PRINT == 1
        printf("awake\r\n");
#endif
        Delay_Ms(1000);
        GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, ledState);
        ledState ^= 1;
#if ENABLE_UART_PRINT == 1
        printf("go to standby %d\r\n", ledState);
#endif

        PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}