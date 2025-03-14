#include <ch32v00x.h>
#include <debug.h>

#define VRX_PIN GPIO_Pin_1
#define VRY_PIN GPIO_Pin_2

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
    // SDI_Printf_Enable();

    // USART1 printf の有効化
    USART_Printf_Init(115200);

    printf("init\r\n");

    uint16_t adc_buf[2] = {0};

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    ADC_InitTypeDef ADC_InitStructure = {0};
    DMA_InitTypeDef DMA_InitStructure = {0};

    // GPIOにクロック供給
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA, ENABLE);
    // ADCにクロック供給
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    // DMAにクロック供給
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // VRX PA1 - ADC1 CH1
    GPIO_InitStructure.GPIO_Pin = VRX_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // VRX PA2 - ADC1 CH0
    GPIO_InitStructure.GPIO_Pin = VRY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ADC初期化
    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    // スキャンモード有効（分割スキャンモードでない場合は有効化する）
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    // 連続モード有効
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    // チャンネル数
    ADC_InitStructure.ADC_NbrOfChannel = 2;
    ADC_Init(ADC1, &ADC_InitStructure);

    // CH1→CH0の順で変換させる
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_241Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 2, ADC_SampleTime_241Cycles);

    // DMA初期化
    DMA_DeInit(DMA1_Channel1);

    // ADCのデータレジスタ→バッファへ転送
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->RDATAR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)adc_buf;
    // ADC2個分のサイズ
    DMA_InitStructure.DMA_BufferSize = sizeof(adc_buf) / sizeof(adc_buf[0]);
    // バッファのみチャンネル分インクリメントする
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    // 16bitサイズ
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    // 繰り返実行させる
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    // 他
    DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);

    // ADCを有効化し、キャリブレーション
    ADC_Calibration_Vol(ADC1, ADC_CALVOL_50PERCENT);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;

    printf("start\r\n");

    // ADCをDMAに接続
    ADC_DMACmd(ADC1, ENABLE);
    // DMA有効化
    DMA_Cmd(DMA1_Channel1, ENABLE);

    // 初回ADC実行をトリガ（後は自動で繰り返される）
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    int count = 0;

    while (1)
    {
        printf("loop %d\r\n", count++);

        // DMAで自動的にバッファの値が更新されるので、読み取りのみ
        printf("x: %d, y: %d\r\n", adc_buf[0], adc_buf[1]);

        Delay_Ms(200);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}