#include <ch32v00x.h>
#include <debug.h>

#define LED1_PIN GPIO_Pin_1
#define LED2_PIN GPIO_Pin_2
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
    GPIO_InitStructure.GPIO_Pin = LED1_PIN | LED2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BUTTON_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    printf("start\r\n");

    // Independent Watch Dog Timerを2sのカウンタでセット
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    // LSIは128kHzにつき、128分周でミリ秒になる
    IWDG_SetPrescaler(IWDG_Prescaler_128);
    // 2,000ms = 2s でリセット
    IWDG_SetReload(2000);
    IWDG_ReloadCounter();
    IWDG_Enable();

    int count = 0;

    // リセットされたことが分かるようにLED2を点滅
    GPIO_WriteBit(GPIOC, LED2_PIN, Bit_SET);
    Delay_Ms(500);
    GPIO_WriteBit(GPIOC, LED2_PIN, Bit_RESET);

    printf("start loop\r\n");

    while (1)
    {
        printf("loop %d\r\n", count++);

        uint8_t btn = GPIO_ReadInputDataBit(GPIOA, BUTTON_PIN);
        if (btn == 0)
        {
            // ボタンが押された場合、LED1を点灯せず、Watch Dog Timerのカウンタもリロードしない
            // これによりWatch Dog Timerによるリセットが発生する
            printf("Button pressed %d\r\n", count);
            Delay_Ms(500);
            continue;
        }

        // ボタンが押されない場合、LED1を点滅
        GPIO_WriteBit(GPIOC, LED1_PIN, Bit_SET);
        Delay_Ms(250);
        GPIO_WriteBit(GPIOC, LED1_PIN, Bit_RESET);
        Delay_Ms(250);

        // Watch Dog Timerをリロード
        IWDG_ReloadCounter();
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}