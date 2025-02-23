#include <ch32v00x.h>
#include <debug.h>

#define SHT31_I2C_ADDR 0x44

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

void send_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
        ;

    I2C_GenerateSTART(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    I2C_Send7bitAddress(I2C1, address << 1, I2C_Direction_Transmitter);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        ;

    for (int i = 0; i < length; i++)
    {

        I2C_SendData(I2C1, data[i]);

        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
            ;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
}

void read_i2c_data(uint8_t address, uint8_t *data, uint8_t length)
{
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET)
        ;

    I2C_GenerateSTART(I2C1, ENABLE);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT))
        ;

    I2C_Send7bitAddress(I2C1, address << 1, I2C_Direction_Receiver);

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
        ;

    for (int i = 0; i < length; i++)
    {
        data[i] = I2C_ReceiveData(I2C1);

        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))
            ;
    }

    I2C_GenerateSTOP(I2C1, ENABLE);
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

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

    // PC1: SDA, PC2: SCL
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // I2C1 Init
    I2C_InitTSturcture.I2C_ClockSpeed = 100000;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitTSturcture.I2C_OwnAddress1 = SHT31_I2C_ADDR;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C1, &I2C_InitTSturcture);

    I2C_Cmd(I2C1, ENABLE);
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    Delay_Ms(100);

    printf("first send\r\n");

    uint8_t buf1[2] = {0x30, 0xA2};
    send_i2c_data(SHT31_I2C_ADDR, buf1, 2);
    Delay_Ms(300);

    uint8_t buf2[2] = {0x30, 0x41};
    send_i2c_data(SHT31_I2C_ADDR, buf2, 2);
    Delay_Ms(300);

    printf("start\r\n");

    int count = 0;

    while (1)
    {
        printf("loop %d\r\n", count++);

        uint8_t dac[6];
        uint32_t t, h;
        uint32_t temperature, humidity;

        uint8_t buf[2] = {0x24, 0x00};
        send_i2c_data(SHT31_I2C_ADDR, buf, 2);

        Delay_Ms(300);

        read_i2c_data(SHT31_I2C_ADDR, dac, sizeof(dac));

        t = (dac[0] << 8) | dac[1];                       // 1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、tに代入
        temperature = (((uint32_t)(t) * 175) >> 16) - 45; // 温度の計算、temperatureに代入
        h = (dac[3] << 8) | dac[4];                       // 4Byte目のデータを8bit左にシフト、OR演算子で5Byte目のデータを結合して、hに代入
        humidity = (((uint32_t)(h) * 100) >> 16);         // 湿度の計算、humidityに代入

        printf("Temperature: %lu, Humidity: %lu\r\n", temperature, humidity);

        Delay_Ms(5000);
    }
}

void NMI_Handler(void) {}
void HardFault_Handler(void)
{
    while (1)
    {
    }
}