#include <ch32v00x.h>
#include <debug.h>

#define TIMEOUT_MAX 100000

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

uint8_t CMD_READ_CO2_CONNECTION[9] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
uint8_t CMD_TURN_ON_SELF_CALIBRATION[9] = {0xFF, 0x01, 0x79, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xE6};

uint16_t read_uart_with_timeout(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        int32_t timeout = TIMEOUT_MAX;
        while (timeout-- && USART_GetFlagStatus(USART1, USART_FLAG_RXNE) == RESET)
            ;
        ;

        if (timeout < 0)
        {
            return i;
        }

        buf[i] = USART_ReceiveData(USART1);
    }

    return len;
}

void send_uart(uint8_t *buf, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
            ;

        USART_SendData(USART1, buf[i]);

        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
            ;
    }
}

int loop(uint32_t loop_count)
{
    uint8_t read_buf[9] = {0};
    printf("loop %d\r\n", loop_count++);

    send_uart(CMD_READ_CO2_CONNECTION, sizeof(CMD_TURN_ON_SELF_CALIBRATION));

    uint16_t read_len = 0;

    // 0xff 0x86 が読めるまで読み進める
    while (1)
    {
        read_len = read_uart_with_timeout(&read_buf[0], 1);
        if (read_len == 0)
        {
            printf("read timeout\r\n");
            Delay_Ms(1000);

            return 1;
        }
        if (read_buf[0] != 0xff)
        {
            // printf("invalid start byte: %0x\r\n", read_buf[0]);
            continue;
        }

        read_len = read_uart_with_timeout(&read_buf[1], 1);
        if (read_len == 0)
        {
            printf("read timeout\r\n");
            Delay_Ms(1000);
            return 1;
        }
        if (read_buf[1] != 0x86)
        {
            // printf("invalid command byte: %02x\r\n", read_buf[1]);
            continue;
        }

        break;
    }

    read_len = read_uart_with_timeout(&read_buf[2], 7);
    if (read_len < 7)
    {
        printf("timeout\r\n");
        Delay_Ms(1000);
    }

    // for (int i = 0; i < sizeof(read_buf); i++)
    // {
    //     printf("0x%02X ", read_buf[i]);
    // }
    // printf("\r\n");

    // チェックサム
    uint8_t c = 0;
    for (int i = 1; i < 8; i++)
    {
        c += read_buf[i];
    }
    if (0xff - c + 1 != read_buf[8])
    {
        printf("invalid checksum\r\n");
    }

    uint16_t co2 = read_buf[2] * 256 + read_buf[3];
    printf("CO2: %5d ppm\r\n", co2);

    return 0;
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

    Delay_Ms(100);

    printf("init\r\n");

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);

    // PD5: TX
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

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

    // セルフキャリブレーションの有効化
    send_uart(CMD_TURN_ON_SELF_CALIBRATION, sizeof(CMD_TURN_ON_SELF_CALIBRATION));

    Delay_Ms(1);

    // キャリブレーション命令の応答を読み捨てる
    uint8_t read_buf[9] = {0};
    uint32_t read_len = read_uart_with_timeout(read_buf, 9);
    if (read_len < 9)
    {
        printf("timeout %d\r\n", read_len);
    }

    printf("start\r\n");

    int count = 0;

    while (1)
    {
        loop(count++);
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