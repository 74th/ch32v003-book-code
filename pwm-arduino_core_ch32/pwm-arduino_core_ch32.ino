#define LED1 PD2
#define LED2 PD4

void setup() {
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
}

void loop() {
  printf("up\r\n");
  for(int i=0;i<16;i++){
    analogWrite(LED1, 256*i-1);
    analogWrite(LED2, 256*(15-i)-1);
    delay(62);
  }

  printf("down\r\n");
  for(int i=0;i<16;i++){
    analogWrite(LED1, 256*(15-i)-1);
    analogWrite(LED2, 256*i-1);
    delay(62);
  }
}
