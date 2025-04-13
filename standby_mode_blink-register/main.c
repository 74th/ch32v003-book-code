#include "ch32fun.h"
#include <stdio.h>

uint32_t count;

uint8_t ledState = 0;

int main()
{
  SystemInit();

  printf("wait\r\n");

  Delay_Ms(5000);

  printf("init\r\n");

  // Enable Clock
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO;
  RCC->APB1PCENR |= RCC_APB1Periph_PWR;
  // RCC_LSICmd(ENABLE);
  RCC->RSTSCKR |= (1 << 0);

  // GPIO C0 Push-Pull
  GPIOC->CFGLR &= ~(0xf << (4 * 1));
  GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

  // EXTI
  EXTI->INTENR &= ~EXTI_Line9;
  EXTI->EVENR &= ~EXTI_Line9;
  EXTI->RTENR &= ~EXTI_Line9;
  EXTI->FTENR &= ~EXTI_Line9;
  EXTI->EVENR |= EXTI_Line9;
  EXTI->FTENR |= EXTI_Line9;

  while (!(RCC->RSTSCKR & (1 << 1)))
    ;

  // PWR_AWU_SetPrescaler(PWR_AWU_Prescaler_10240);
  uint32_t tmp = PWR->AWUPSC & AWUPSC_MASK;
  tmp |= PWR_AWU_Prescaler_10240;
  PWR->AWUPSC = tmp;
  // PWR_AWU_SetWindowValue(25);
  tmp = 0;
  tmp = PWR->AWUWR & AWUWR_MASK;
  tmp |= 25;
  PWR->AWUWR = tmp;
  // PWR_AutoWakeUpCmd(ENABLE);
  PWR->AWUCSR |= (1 << 1);

  printf("start\r\n");

  uint32_t count = 0;

  while (1)
  {
    printf("go to stanby %ld\r\n\r\n", count);

    // PWR_EnterSTANDBYMode(PWR_STANDBYEntry_WFE);
    PWR->CTLR &= CTLR_DS_MASK;
    PWR->CTLR |= PWR_CTLR_PDDS;

    NVIC->SCTLR |= (1 << 2);

    __WFE();

    NVIC->SCTLR &= ~(1 << 2);

    printf("awake %ld\r\n", count);

    if (ledState)
    {
      GPIOC->BSHR = 1 << 1;
    }
    else
    {
      GPIOC->BSHR = (1 << (16 + 1));
    }
    ledState ^= 1;

    count++;
  }
}
