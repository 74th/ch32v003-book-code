#include <ch32v00x.h>
#include <debug.h>

#define I2C_ADDRESS 0x10

void I2C1_EV_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void I2C1_ER_IRQHandler(void) __attribute__((interrupt("WCH-Interrupt-fast")));

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

volatile uint8_t i2c_registers[0x30] = {0x00};
volatile uint8_t i2c_start_position = 0;
volatile uint8_t i2c_position = 0;
volatile uint8_t i2c_first_receive = 0;
uint8_t i2c_request_available = 0;
uint8_t i2c_receive_available = 0;

void I2C1_EV_IRQHandler(void)
{
    uint32_t event = I2C_GetLastEvent(I2C1);

    // これがないと動かない？？
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    if (event == I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED)
    {
        // 最初のイベント
        i2c_first_receive = 1;
    }
    if (event == I2C_EVENT_SLAVE_BYTE_RECEIVED)
    {
        // 1byte の受信イベント（slave -> master）
        uint8_t v = I2C_ReceiveData(I2C1);
        if (i2c_first_receive)
        {
            // 1バイト目の受信
            i2c_position = v;
            i2c_start_position = v;
            i2c_first_receive = 0;
        }
        else if (i2c_position < sizeof(i2c_registers))
        {
            // 2バイト目以降の受信
            i2c_registers[i2c_position] = v;
            i2c_position++;
            i2c_receive_available += 1;
        }
        else
        {
            // 2バイト目以降の受信、レジスタ範囲外
            // 何もしない
        }
    }
    if (event == I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED ||
        event == I2C_EVENT_SLAVE_BYTE_TRANSMITTED)
    {
        // 1byte の送信イベント（master -> slave）
        I2C_SendData(I2C1, i2c_registers[i2c_position]);
        i2c_position++;
        i2c_request_available++;
    }
}

void I2C1_ER_IRQHandler(void)
{
    if (I2C_GetITStatus(I2C1, I2C_IT_AF))
    {
        I2C_ClearITPendingBit(I2C1, I2C_IT_AF);
    }
}

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

    Delay_Ms(100);

    printf("init\r\n");

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    I2C_InitTypeDef I2C_InitTSturcture = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // PC1: SDA, PC2: SCL
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
    NVIC_Init(&NVIC_InitStructure);

    // I2C1 Init
    I2C_InitTSturcture.I2C_ClockSpeed = 2000000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = I2C_ADDRESS << 1;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitTSturcture);

    I2C_Cmd(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
    I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
    NVIC_EnableIRQ(I2C1_EV_IRQn);
    NVIC_SetPriority(I2C1_EV_IRQn, 2 << 4);
    NVIC_EnableIRQ(I2C1_ER_IRQn);

    // printf("I2C1->CTLR1: %4x\r\n", I2C1->CTLR1);
    // printf("I2C1->CTLR2: %4x\r\n", I2C1->CTLR2);
    // printf("I2C1->CKCFGR: %4x\r\n", I2C1->CKCFGR);
    // printf("I2C1->OADDR1: %4x\r\n", I2C1->OADDR1);

    for (int i = 0; i < 4; i++)
    {
        i2c_registers[0x20 + i] = 0x10 * i;
    }

    Delay_Ms(100);

    printf("start\r\n");

    while (1)
    {
        if (i2c_request_available > 0)
        {
            printf("request: count=%d, start=%x, length=%d\r\n", i2c_request_available, i2c_start_position, i2c_position - i2c_start_position);
            printf("reg[0x20]: %x %x %x %x\r\n", i2c_registers[0x20], i2c_registers[0x21], i2c_registers[0x22], i2c_registers[0x23]);
            i2c_request_available = 0;
        }

        if (i2c_receive_available > 0)
        {
            printf("receive: count=%d, start=%x, length=%d\r\n", i2c_receive_available, i2c_start_position, i2c_position - i2c_start_position);
            printf("reg[0x10]: %x %x %x %x\r\n", i2c_registers[0x10], i2c_registers[0x11], i2c_registers[0x12], i2c_registers[0x13]);
            i2c_receive_available = 0;
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