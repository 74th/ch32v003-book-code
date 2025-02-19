#define LED PC0
#define BUTTON PA1

void setup()
{
  Serial.begin(115200);

  // not working - 動作を確認できなかった
  // USART_Printf_Init(115200);

  Serial.println("init");
  // printf("init\r\n");

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);

  Serial.println("start");
  // printf("start\r\n");
}

int count = 0;

void loop()
{
  Serial.print("loop ");
  Serial.println(count++);
  // printf("loop %d\r\n", count);

  bool btn = digitalRead(BUTTON);
  if (!btn)
  {
    delay(1000);
    return;
  }

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
