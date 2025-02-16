#include <ch32v00x.h>
#include <debug.h>

#define LED_PIN GPIO_Pin_2

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

void TIM1_PWMOut_Init()
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    TIM_OCInitTypeDef TIM_OCInitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_TIM1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    TIM_TimeBaseInitStructure.TIM_Period = 255;
    TIM_TimeBaseInitStructure.TIM_Prescaler = 1;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseInitStructure);

    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 128;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM1, &TIM_OCInitStructure);

    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Disable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_Cmd(TIM1, ENABLE);
}

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    SDI_Printf_Enable();

    printf("init\r\n");

    TIM1_PWMOut_Init(100, 48000 - 1, 50);

    printf("start\r\n");

    while (1)
    {
        Delay_Ms(100);
        printf("up\r\n");
        for (int i = 1; i <= 32; i++)
        {
            TIM_SetCompare1(TIM1, 8 * i - 1);
            // TIM_SetCompare2(TIM1, 128 * (33 - i) - 1);
            Delay_Us(1000000 / 32);
        }

        printf("down\r\n");
        for (int i = 1; i <= 32; i++)
        {
            TIM_SetCompare1(TIM1, 8 * (33 - i) - 1);
            // TIM_SetCompare2(TIM1, 8 * i - 1);
            Delay_Us(1000000 / 32);
        }
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}