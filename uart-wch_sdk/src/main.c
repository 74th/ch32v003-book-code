#include <ch32v00x.h>
#include <debug.h>

#define SHT31_I2C_ADDR 0x44

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

uint8_t CMD_READ_CO2_CONNECTION[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
uint8_t CMD_TURN_ON_SELF_CALIBRATION[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();

    // SDI printf の有効化
    // ビルドフラグ SDI_PRINT=1 が必要
    // wlink sdi-print enable を実行し、USBシリアルをリスンする
    SDI_Printf_Enable();

    Delay_Ms(100);

    printf("init\r\n");

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

    // PD5: TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PD6: RX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 9600;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init(USART1, &USART_InitStructure);
    USART_Cmd(USART1, ENABLE);

    Delay_Ms(100);

    printf("send turn on self calibration\r\n");

    for (int i = 0; i < sizeof(CMD_TURN_ON_SELF_CALIBRATION); i++)
    {
        USART_SendData(USART1, CMD_TURN_ON_SELF_CALIBRATION[i]);
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    }

    printf("start\r\n");

    int count = 0;

    while (1)
    {
        uint8_t read_buf[9] = {0};
        printf("loop %d\r\n", count++);

        for (int i = 0; i < sizeof(CMD_READ_CO2_CONNECTION); i++)
        {
            USART_SendData(USART1, CMD_READ_CO2_CONNECTION[i]);
            while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
                ;
        }

        printf("@@1\r\n");

        for (int j = 0; j < sizeof(read_buf); j++)
        {
            while (USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
                ;
            read_buf[j] = USART_ReceiveData(USART1);
        }

        printf("@@2\r\n");

        printf("read_buf: %2x %2x\r\n", read_buf[0], read_buf[1]);

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