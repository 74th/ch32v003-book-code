#define LED C0
#define BUTTON C1

void setup() {
  Serial.begin(115200);

  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
}

void loop() {
  printf("loop\r\n");
  Serial.print("loop\r\n");

  bool btn = digitalRead(BUTTON);
  if(!btn){
    return;
  }

  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
