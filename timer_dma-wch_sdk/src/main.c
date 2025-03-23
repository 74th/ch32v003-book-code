#include <ch32v00x.h>
#include <debug.h>

#define SEGMENT_LED_NUM 4
#define ANODE_NUM 8

typedef struct
{
    GPIO_TypeDef *gpio;
    uint8_t pin_no;
    uint32_t *buffer;
} PinGPIOMap;

uint32_t gpioa_bshr_buf[SEGMENT_LED_NUM] = {0};
uint32_t gpioc_bshr_buf[SEGMENT_LED_NUM] = {0};
uint32_t gpiod_bshr_buf[SEGMENT_LED_NUM] = {0};

PinGPIOMap anode_pins[ANODE_NUM] = {
    {GPIOC, GPIO_Pin_4, gpioc_bshr_buf}, // SA
    {GPIOC, GPIO_Pin_3, gpioc_bshr_buf}, // SB
    {GPIOD, GPIO_Pin_3, gpiod_bshr_buf}, // SC
    {GPIOD, GPIO_Pin_2, gpiod_bshr_buf}, // SD
    {GPIOC, GPIO_Pin_7, gpioc_bshr_buf}, // SE
    {GPIOC, GPIO_Pin_5, gpioc_bshr_buf}, // SF
    {GPIOC, GPIO_Pin_6, gpioc_bshr_buf}, // SG
    {GPIOA, GPIO_Pin_1, gpioa_bshr_buf}, // SDP
};

PinGPIOMap cathode_common_pins[SEGMENT_LED_NUM] = {
    {GPIOD, GPIO_Pin_6, gpiod_bshr_buf}, // SEL1
    {GPIOD, GPIO_Pin_5, gpiod_bshr_buf}, // SEL2
    {GPIOD, GPIO_Pin_4, gpiod_bshr_buf}, // SEL3
    {GPIOC, GPIO_Pin_0, gpioc_bshr_buf}, // SEL4
};

#define SEGPATTERN_0 0b00111111
#define SEGPATTERN_1 0b00000110
#define SEGPATTERN_2 0b01011011
#define SEGPATTERN_3 0b01001111
#define SEGPATTERN_4 0b01100110
#define SEGPATTERN_5 0b01101101
#define SEGPATTERN_6 0b01111101
#define SEGPATTERN_7 0b00000111
#define SEGPATTERN_8 0b01111111
#define SEGPATTERN_9 0b01101111
#define SEGPATTERN_d 0b10000000

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

void init(void)
{
    printf("init\r\n");

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    // GPIO、DMA、TIMにクロック供給
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_SRAM, ENABLE);

    // GPIO出力
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;

    for (int i = 0; i < sizeof(anode_pins) / sizeof(PinGPIOMap); i++)
    {
        GPIO_InitStructure.GPIO_Pin = anode_pins[i].pin_no;
        GPIO_Init(anode_pins[i].gpio, &GPIO_InitStructure);
    }

    for (int i = 0; i < sizeof(cathode_common_pins) / sizeof(PinGPIOMap); i++)
    {
        GPIO_InitStructure.GPIO_Pin = cathode_common_pins[i].pin_no;
        GPIO_Init(cathode_common_pins[i].gpio, &GPIO_InitStructure);
    }

    // 48MHzで動作するため、48MHz / 4800 = 10kHz (100us) でカウント
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

    // TIM1_CH1 DMA1_Channel2
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOA->BSHR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gpioa_bshr_buf;
    DMA_InitStructure.DMA_BufferSize = sizeof(gpioa_bshr_buf) / sizeof(gpioa_bshr_buf[0]);
    DMA_DeInit(DMA1_Channel2);
    DMA_Init(DMA1_Channel2, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel2, ENABLE);

    // TIM1_CH2 DMA1_Channel3
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOC->BSHR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gpioc_bshr_buf;
    DMA_InitStructure.DMA_BufferSize = sizeof(gpioc_bshr_buf) / sizeof(gpioc_bshr_buf[0]);
    DMA_DeInit(DMA1_Channel3);
    DMA_Init(DMA1_Channel3, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel3, ENABLE);

    // TIM1_CH3 DMA1_Channel6
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&GPIOD->BSHR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)gpiod_bshr_buf;
    DMA_InitStructure.DMA_BufferSize = sizeof(gpiod_bshr_buf) / sizeof(gpiod_bshr_buf[0]);
    DMA_DeInit(DMA1_Channel6);
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel6, ENABLE);

    // 48MHzで動作するため、48MHz / 4800 = 10kHz (100us) でカウント
    TIM_TimeBaseInitStructure.TIM_Prescaler = 4800 - 1;
    // 4つのLEDを点灯
    TIM_TimeBaseInitStructure.TIM_Period = 4;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // 48MHzで動作するため、48MHz / 480 = 100kHz (10us) でカウント
    TIM_TimeBaseInitStructure.TIM_Prescaler = 480;
    // 10us ごとにトリガ
    TIM_TimeBaseInitStructure.TIM_Period = 10 - 1;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    // PWMではないが、PWMモードで出力設定する
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStructure.TIM_Pulse = 6;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);
    TIM_CtrlPWMOutputs(TIM1, ENABLE);

    TIM_GenerateEvent(TIM1, TIM_EventSource_Trigger);

    // TIM1_CH1 DMA1_Channel2
    TIM_DMACmd(TIM1, TIM_DMA_CC1, ENABLE);
    // TIM1_CH2 DMA1_Channel3
    TIM_DMACmd(TIM1, TIM_DMA_CC2, ENABLE);
    // TIM1_CH3 DMA1_Channel6
    TIM_DMACmd(TIM1, TIM_DMA_CC3, ENABLE);
    TIM_DMACmd(TIM1, TIM_DMA_Update, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

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

    printf("start\r\n");

    Delay_Ms(100);

    int count = 0;
    for (int b = 0; b < SEGMENT_LED_NUM; b++)
    {
        gpioa_bshr_buf[b] = 0;
        gpioc_bshr_buf[b] = 0;
        gpiod_bshr_buf[b] = 0;

        // 点灯対象のLEDのカソードをLow、それ以外をHighにする
        for (int j = 0; j < SEGMENT_LED_NUM; j++)
        {
            if (b == j)
            {
                cathode_common_pins[j].buffer[b] |= cathode_common_pins[j].pin_no << 16;
            }
            else
            {
                cathode_common_pins[j].buffer[b] |= cathode_common_pins[j].pin_no;
            }
        }

        uint8_t pattern = 0;
        switch (b)
        {
        case 0:
            pattern = SEGPATTERN_0;
            break;
        case 1:
            pattern = SEGPATTERN_1;
            break;
        case 2:
            pattern = SEGPATTERN_2;
            break;
        case 3:
            pattern = SEGPATTERN_3;
            break;
        }

        for (int j = 0; j < ANODE_NUM; j++)
        {
            if (pattern & (1 << j))
            {
                // 点灯
                anode_pins[j].buffer[b] |= anode_pins[j].pin_no;
            }
            else
            {
                // 消灯
                anode_pins[j].buffer[b] |= anode_pins[j].pin_no << 16;
            }
        }
    }

    init();

    while (1)
    {
        printf("Loop %d", count++);

        Delay_Ms(1000);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}