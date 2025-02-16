#define LED1 PD2
#define LED2 PD4

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
