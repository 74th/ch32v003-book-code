#include "Wire.h"

#define I2C_ADDRESS 0x10

volatile uint8_t i2c_registers[0x30] = { 0x00 };
volatile uint8_t position = 0;

uint8_t on_request_available = 0;
uint8_t on_receive_available = 0;
uint8_t on_receive_length = 0;

void on_receive(int length) {
  for (int i = 0; i < length; i++) {
    if (i == 0) {
      position = Wire.read();
    } else {
      if (position + (i - 1) < sizeof(i2c_registers)) {
        i2c_registers[position + (i - 1)] = Wire.read();
      } else {
        Wire.read();
      }
    }
  }

  on_receive_available += 1;
}

void on_request() {
  int i;

  for (i = 0; i < 4; i++) {
    if (position + (i - 1) < sizeof(i2c_registers)) {
      Wire.write(i2c_registers[position + i]);
    } else {
      Wire.write(0x00);
    }
  }

  on_request_available += 1;
}

void setup() {
  Serial.begin(115200);

  // not working - 動作を確認できなかった
  // USART_Printf_Init(115200);

  Serial.println("init");
  // printf("init\r\n");

  Wire.onReceive(on_receive);
  Wire.onRequest(on_request);
  Wire.begin(I2C_ADDRESS);

  for (int i = 0; i < 4; i++) {
    i2c_registers[0x20 + i] = 0x10 * i;
  }

  Serial.println("start");
  // printf("start\r\n");
}

uint32_t count = 0;

void loop() {
  if (on_request_available > 0) {
    Serial.print("on_request: count=");
    Serial.print(on_request_available);
    Serial.print(" position=");
    Serial.println(position, HEX);

    Serial.print("reg[0x10]: ");
    Serial.print(i2c_registers[0x20], HEX);
    Serial.print(" ");
    Serial.print(i2c_registers[0x21], HEX);
    Serial.print(" ");
    Serial.print(i2c_registers[0x22], HEX);
    Serial.print(" ");
    Serial.println(i2c_registers[0x23], HEX);

    on_request_available = 0;
  }
  if (on_receive_available > 0) {
    Serial.print("on_receive: count=");
    Serial.print(on_receive_available);
    Serial.print(" position=");
    Serial.print(position, HEX);
    Serial.print(" length=");
    Serial.println(on_receive_length);

    Serial.print("reg[0x10]: ");
    Serial.print(i2c_registers[0x10], HEX);
    Serial.print(" ");
    Serial.print(i2c_registers[0x11], HEX);
    Serial.print(" ");
    Serial.print(i2c_registers[0x12], HEX);
    Serial.print(" ");
    Serial.println(i2c_registers[0x13], HEX);

    on_receive_available = 0;
  }
}
