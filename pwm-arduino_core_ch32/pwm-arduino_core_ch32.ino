#define PATTERN 1

#if PATTERN==1
// TIM1とTIM2を併用
// TIM1_CH1
#define LED1 PD2
// TIM2_CH1
#define LED2 PD4

#elif PATTERN==2
// TIM1の複数チャンネルを利用
// TIM1_CH1
#define LED1 PD2
// TIM1_CH4
#define LED2 PC4

#elif PATTERN==3
// TIM1、TIM2のリマップを利用（動作せず）
// TIM1_CH1_1
#define LED1 PC6
// TIM2_CH2_1
#define LED2 PC5
#endif

void setup() {
  USART_Printf_Init(115200);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

// analogWrite(Pin, 0-4095);

void loop() {
  printf("up\r\n");
  for(int i=1;i<=32;i++){
    analogWrite(LED1, 128*i-1);
    analogWrite(LED2, 128*(33-i)-1);
    delay(31);
  }

  printf("down\r\n");
  for(int i=1;i<=32;i++){
    analogWrite(LED1, 128*(33-i)-1);
    analogWrite(LED2, 128*i-1);
    delay(31);
  }
}
