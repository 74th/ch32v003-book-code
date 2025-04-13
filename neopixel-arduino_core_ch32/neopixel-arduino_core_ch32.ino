#define LED PC0

#define LED_PIN_NO 0
#define LED_NUM 6

// NeoPixelの制御
// digital_pin: NeoPixelのデータピン
// data: NeoPixelのデータ
// led_num: NeoPixelのLEDの数
void write_neopixel(uint32_t digital_pin, uint8_t *data, uint32_t led_num)
{
  uint32_t pin_name = digitalPinToPinName(digital_pin);
  GPIO_TypeDef *gpio = get_GPIO_Port(CH_PORT(pin_name));
  uint32_t pin_mask = CH_GPIO_PIN(pin_name);
  uint32_t h = pin_mask;
  uint32_t l = pin_mask << 16;
  uint32_t data_size = led_num * 3;

  for (int i = 0; i < data_size; i++)
  {
    uint16_t c = data[i];
    for (int j = 0; j < 8; j++)
    {
      if (c & 0x1)
      {
        // 0.7us
        gpio->BSHR = h;
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        // 0.6us
        gpio->BSHR = l;
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
      }
      else
      {
        // 0.35us
        gpio->BSHR = h;
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        // 0.8us
        gpio->BSHR = l;
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
        asm("c.nop");
      }
      c = c >> 1;
    }
  }
}

void setup()
{
  Serial.begin(115200);

  Serial.println("init");

  // 出力
  pinMode(LED, OUTPUT);

  Serial.println("start");
}

int count = 0;

void loop()
{
  Serial.print("loop ");
  Serial.println(count++);

  uint8_t data[LED_NUM * 3];

  for (int i = 0; i < LED_NUM; i++)
  {
    switch ((count + i) % 6)
    {
    case 0:
      data[i * 3] = 0x20;
      data[i * 3 + 1] = 0x00;
      data[i * 3 + 2] = 0x00;
      break;
    case 1:
      data[i * 3] = 0x20;
      data[i * 3 + 1] = 0x20;
      data[i * 3 + 2] = 0x00;
      break;
    case 2:
      data[i * 3] = 0x00;
      data[i * 3 + 1] = 0x20;
      data[i * 3 + 2] = 0x00;
      break;
    case 3:
      data[i * 3] = 0x00;
      data[i * 3 + 1] = 0x20;
      data[i * 3 + 2] = 0x20;
      break;
    case 4:
      data[i * 3] = 0x00;
      data[i * 3 + 1] = 0x00;
      data[i * 3 + 2] = 0x20;
      break;
    case 5:
      data[i * 3] = 0x20;
      data[i * 3 + 1] = 0x00;
      data[i * 3 + 2] = 0x20;
      break;
    }
  }

  write_neopixel(LED, data, LED_NUM);
  delay(300);
}
