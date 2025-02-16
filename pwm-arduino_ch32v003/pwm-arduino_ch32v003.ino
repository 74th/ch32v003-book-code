#define LED1 D2
#define LED2 D4

void setup() {
  USART_Printf_Init(115200);
  analogWriteResolution(8);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  printf("up\r\n");
  for(int i=1;i<=16;i++){
    analogWrite(LED1, 16*i-1);
    analogWrite(LED2, 16*(17-i)-1);
    delay(1000/16);
  }

  printf("down\r\n");
  for(int i=1;i<=16;i++){
    analogWrite(LED1, 16*(17-i)-1);
    analogWrite(LED2, 16*i-1);
    delay(1000/16);
  }
}
