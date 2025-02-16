#define LED PC0
#define BUTTON PA1

void setup()
{
  // 動作を確認できなかった
  USART_Printf_Init(115200);

  printf("init\r\n");

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  printf("start\r\n");
}

void loop()
{
  printf("loop\r\n");
  
  bool btn = digitalRead(BUTTON);

  if(!btn){
    return;
  }

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
