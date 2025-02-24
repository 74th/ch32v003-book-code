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

int _main(void)
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

    EXTI_INT_INIT();

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

void EXTI_INT_INIT(void)
{
    EXTI_InitTypeDef EXTI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    EXTI_InitStructure.EXTI_Line = EXTI_Line9;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Event;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    Delay_Ms(1000);
    Delay_Ms(1000);
    EXTI_INT_INIT();

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);

#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("ChipID:%08x\r\n", DBGMCU_GetCHIPID());
    printf("Standby Mode Test\r\n");
    printf("0x1FFFF800-%08x\r\n", *(u32 *)0x1FFFF800);

    RCC_LSICmd(ENABLE);
    while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)
        ;
    PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
    PWR_AWU_SetWindowValue(25);
    PWR_AutoWakeUpCmd(ENABLE);
    PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);

#if (SDI_PRINT == SDI_PR_OPEN)
    SDI_Printf_Enable();
#else
    USART_Printf_Init(115200);
#endif
    printf("\r\n Auto wake up \r\n");
    while (1)
    {
        Delay_Ms(1000);
        printf("Run in main\r\n");
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}