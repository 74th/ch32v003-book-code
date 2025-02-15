#define LED1 PC1
#define LED2 PC2

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  printf("up\r\n");
  for(int i=0;i<16;i++){
    analogWrite(LED1, 16*i-1);
    delay(62);
  }
  digitalWrite(LED2, HIGH);

  printf("down\r\n");
  for(int i=16;i>0;i--){
    analogWrite(LED1, 16*i-1);
    delay(62);
  }
  digitalWrite(LED2, LOW);
}
