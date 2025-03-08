#include <ch32v00x.h>
#include <debug.h>

#define SHT31_I2C_ADDR 0x44

void NMI_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void HardFault_Handler(void) __attribute__((interrupt("WCH-Interrupt-fast")));
void Delay_Init(void);
void Delay_Ms(uint32_t n);

void send_spi_data(uint8_t address, uint8_t *data, uint8_t length)
{
    for (int i = 0; i < length; i++)
    {
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
            ;
        SPI_I2S_SendData(SPI1, data[i]);
    }
}

void read_spi_data(uint8_t *data, uint8_t length)
{
    for (int i = 0; i < length; i++)
    {
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET)
            ;
        SPI_I2S_SendData(SPI1, 0);

        uint8_t b = 1;
        while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET)
        {
            if (b)
            {
                printf("SPIx->STATR: %x\r\n", SPI1->STATR);
                b = 0;
            }
        }

        data[i] = SPI_I2S_ReceiveData(SPI1);
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
    SPI_InitTypeDef SPI_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO | RCC_APB2Periph_SPI1, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);

    SPI_Cmd(SPI1, ENABLE);

    // SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);

    GPIO_SetBits(GPIOC, GPIO_Pin_1);

    Delay_Ms(500);

    GPIO_ResetBits(GPIOC, GPIO_Pin_1);

    printf("software reset\r\n");

    uint8_t buf1[4] = {0xff, 0xff, 0xff, 0xff};
    send_spi_data(SHT31_I2C_ADDR, buf1, sizeof(buf1));
    Delay_Ms(100);

    uint8_t buf2[1] = {0x54};
    send_spi_data(SHT31_I2C_ADDR, buf2, sizeof(buf2));
    Delay_Ms(300);

    printf("start\r\n");

    int count = 0;

    while (1)
    {
        printf("loop %d\r\n", count++);

        uint8_t buf[2] = {0x00, 0x00};
        read_spi_data(buf, sizeof(buf));

        printf("Received data: %02x %02x\r\n", buf[0], buf[1]);

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