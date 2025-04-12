#include "ch32fun.h"
#include <stdio.h>

int main()
{
  SystemInit();

  printf("init\r\n");

  // Enable GPIOs
  RCC->APB2PCENR |= RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOA;

  // GPIO C1 Push-Pull
  GPIOC->CFGLR &= ~(0xf << (4 * 1));
  GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 1);

  // GPIO C2 Push-Pull
  GPIOC->CFGLR &= ~(0xf << (4 * 2));
  GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP) << (4 * 2);

  // GPIO A1 Input-Pullup
  GPIOA->CFGLR &= ~(0xf << (4 * 1));
  GPIOA->CFGLR |= (GPIO_Speed_In | GPIO_CNF_IN_PUPD) << (4 * 1);
  // Pull up
  GPIOA->OUTDR &= ~(1 << 1);
  GPIOA->OUTDR |= (1 << 1);

  // Independent Watch Dog Timerを2sのカウンタでセット
  IWDG->CTLR = IWDG_WriteAccess_Enable;
  // LSIは128kHzにつき、128分周でミリ秒になる
  IWDG->PSCR = IWDG_Prescaler_128;
  // 2,000ms = 2s でリセット
  IWDG->RLDR = 2000 & 0xfff;

  // Watch Dog Timerの開始
  IWDG->CTLR = CTLR_KEY_Enable;

  printf("start\r\n");

  // リセットされたことが分かるようにLED2を点滅
  GPIOC->BSHR = 1 << 2;
  Delay_Ms(500);
  GPIOC->BSHR = (1 << (16 + 2));

  uint32_t count = 0;

  while (1)
  {
    printf("loop %lu\n", count++);

    GPIOA->BSHR = 1 << 1;

    if ((GPIOA->INDR & (1 << 1)) == 0)
    {
      printf("Button pressed %d\r\n", count);
      Delay_Ms(500);
      continue;
    }

    GPIOC->BSHR = 1 << 1;
    Delay_Ms(250);
    GPIOC->BSHR = (1 << (16 + 1));
    Delay_Ms(250);

    // Watch Dog Timerをリロード
    IWDG->CTLR = CTLR_KEY_Reload;
  }
}
