#define LED C0
#define BUTTON C1

void setup()
{
  Serial.begin(115200);

  // minichlink -T
  printf("init\r\n");
  // USART
  Serial.println("init");

  // 出力
  pinMode(LED, OUTPUT);

  // 入力
  // INPUT: フローティング
  // INPUT_PULLUP: プルアップ
  // INPUT_PULLDOWN: プルダウン
  pinMode(BUTTON, INPUT_PULLUP);

  printf("start\r\n");
  Serial.println("start");
}

int count = 0;

void loop()
{
  printf("loop %d\r\n", count);
  Serial.print("loop ");
  Serial.println(count);
  count++;

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
