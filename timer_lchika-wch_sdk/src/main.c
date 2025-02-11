#include <ch32v00x.h>
#include <debug.h>

#define BLINKY_GPIO_PORT GPIOC
#define BLINKY_GPIO_PIN GPIO_Pin_1

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM1_UP_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void TIM2_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void Delay_Init(void);
void Delay_Ms(uint32_t n);

void TIM1_INT_Init(uint16_t arr, uint16_t psc)
{
    // 高機能タイマー TIM1

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    // クロックを有効化する
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

    // カウントするクロックの周期
    // クロックが 48MHz のため、48,000-1 を設定しておけば
    // 1kHz(1ms) でカウントすることになる
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;

    // カウンターをリセットする周期
    // 100-1 をセットすれば、上と併せて、10Hz(100ms) でリセットされる
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    // リセットされて、さらにイベント発生させる周期
    // 50 をセットすれば、上と併せて、1Hz(1s) でイベントが発生する
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 5 - 1;

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    printf("TIM1->CTLR1: %x\r\n", TIM1->CTLR1);
    printf("TIM1->ATRLR: %x\r\n", TIM1->ATRLR);
    printf("TIM1->PSC: %x\r\n", TIM1->PSC);
    printf("TIM1->RPTCR: %x\r\n", TIM1->RPTCR);
    printf("TIM1->SWEVGR: %x\r\n", TIM1->SWEVGR);
    // TIM1->CTLR1: 0
    // TIM1->ATRLR: 63
    // TIM1->PSC: bb7f
    // TIM1->RPTCR: a
    // TIM1->SWEVGR: 0

    // 割り込みフラグのクリア
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

    printf("TIM1->INTFR: %x\r\n", TIM1->INTFR);
    // TIM1->INTFR: 0

    // 割り込みの設定
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    printf("NVIC->IPRIOR[TIM1_UP_IRQn]: %x\r\n", NVIC->IPRIOR[(uint32_t)(TIM1_UP_IRQn)]);
    printf("NVIC->IENR[TIM1_UP_IRQn]:%x\r\n", NVIC->IENR[(uint32_t)(TIM1_UP_IRQn)]);
    // NVIC->IPRIOR[TIM1_UP_IRQn]: 40
    // NVIC->IENR[TIM1_UP_IRQn]:0

    TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    printf("TIM1->DMAINTENR: %x\r\n", TIM1->DMAINTENR);
    // TIM1->DMAINTENR: 1
}

void TIM2_INT_Init(uint16_t arr, uint16_t psc)
{
    // 汎用タイマー TIM2

    NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    // クロックを有効化する
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // カウントするクロックの周期
    // クロックが 48MHz のため、48,000-1 を設定しておけば
    // 1kHz(1ms) でカウントすることになる
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;

    // カウンターをリセットする周期
    // 1000-1 をセットすれば、上と併せて、1Hz(1s) でリセットされる
    TIM_TimeBaseInitStructure.TIM_Period = arr;
    // TIM2 には RepetitionCounter はない

    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

    // 割り込みフラグのクリア
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    // 割り込みの設定
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
}

uint8_t ledState = 0;

void TIM1_UP_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET)
    {
        printf("1sec event\r\n");
    }
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

    GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, ledState);
    ledState ^= 1;
}

void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        printf("500ms event\r\n");
    }
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

    GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, ledState);
    ledState ^= 1;
}

// 利用するタイマー
#define USE_TIM1 1

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    USART_Printf_Init(115200);
    Delay_Init();

    GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    GPIO_InitStructure.GPIO_Pin = BLINKY_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BLINKY_GPIO_PORT, &GPIO_InitStructure);

    GPIO_WriteBit(BLINKY_GPIO_PORT, BLINKY_GPIO_PIN, 0);

    Delay_Ms(100);

    printf("start\r\n");

#if USE_TIM1 == 1
    TIM1_INT_Init(100 - 1, 48000 - 1);
    TIM_Cmd(TIM1, ENABLE); // 1S
#else
    TIM2_INT_Init(500 - 1, 48000 - 1);
    TIM_Cmd(TIM2, ENABLE); // 1S
#endif

    while (1)
    {
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}