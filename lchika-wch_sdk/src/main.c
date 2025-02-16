#include <ch32v00x.h>
#include <debug.h>

#define LED_PIN GPIO_Pin_0
#define BUTTON_PIN GPIO_Pin_1

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // SDI printf の有効化
    // ビルドフラグ SDI_PRINT=1 が必要
    // wlink sdi-print enable を実行し、USBシリアルをリスンする
    SDI_Printf_Enable();

    // USART1 printf の有効化
    // USART_Printf_Init(115200);

    printf("init\r\n");

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = LED_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    printf("start\r\n");

    int count = 0;

    while (1)
    {
        printf("loop %d\r\n", count++);

        uint8_t btn = GPIO_ReadInputDataBit(GPIOA, BUTTON_PIN);
        if (btn == 0)
        {
            Delay_Ms(1000);
            continue;
        }

        Delay_Ms(500);
        GPIO_WriteBit(GPIOC, LED_PIN, Bit_SET);
        Delay_Ms(500);
        GPIO_WriteBit(GPIOC, LED_PIN, Bit_RESET);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}