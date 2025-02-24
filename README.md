## testing code - テストコード

- [x] Simple LED Blink and button - 単純なLチカとボタン
  - [x] Arduino Core CH32
  - [x] Arduino wch32v003
  - [x] WCH SDK
  - [x] ch32v003fun extralibs
  - [x] register
- [x] Timer LED Blink - タイマーLチカ
  - [x] WCH SDK
  - [x] register
- [x] PWM - PWM制御
  - [x] Arduino Core CH32
  - [x] Arduino wch32v003 (Not Implemented)
  - [x] WCH SDK
  - [x] register
- [x] I2C Master - I2Cマスター
  - [x] Arduino Core CH32
  - [x] Arduino wch32v003
  - [x] WCH SDK
  - [x] register
- [ ] I2C Slave - I2Cスレーブ
  - [ ] Arduino Core CH32cp
  - [ ] Arduino wch32v003
  - [ ] WCH SDK
  - [ ] register
- [ ] SPI Master - SPIマスター
  - [ ] Arduino Core CH32
  - [ ] Arduino wch32v003
  - [ ] WCH SDK
  - [ ] register
- [ ] UART
  - [ ] Arduino Core CH32
  - [ ] Arduino wch32v003
  - [ ] WCH SDK
  - [ ] register
- [ ] Sleep Mode, Standby Mode - スリープモード、スタンバイモード
  - [ ] LED Blink with Sleep Mode during Delay - Delay中はSleep Modeに落とすLチカ
  - [ ] LED Blink with Standby Mode during Delay - Delay中はStanby Modeに落とすLチカ
  - [ ] Resume from Sleep Mode by pressing a button - ボタンを押すとSleep Modeから復帰する
  - [ ] Resume from Standby Mode by pressing a button - ボタンを押すとStandby Modeから復帰する
  - [ ] Standby Mode operates during PWM - PWM中はStandbyが動作すること
- [ ] WatchDogTimer
- [ ] DMA
  - [ ] DMA blink LED - DMAでLチカ
  - [ ] DMA SPI
- [ ] NeoPixel
  - [ ] CPU nop
  - [ ] DMA SPI

## ライブラリ・環境

### WCH公式SDK

https://github.com/openwch/ch32v003

WCH公式SDK。古いSTM32ライクになっている。

PlatformIOで環境を構築できるが、Linux aarch64環境では動作しない（公式環境に含まれるOpenOCD、GCCバイナリを含んでおり、この公式環境はx86_64向けしかない）。

https://github.com/Community-PIO-CH32V/platform-ch32v

### ch32v003fun

https://github.com/cnlohr/ch32v003fun

ミニマルな開発環境。基本レジスタアクセスする。

### arduino_core_ch32

https://github.com/openwch/arduino_core_ch32

WCH公式のArduino環境

2025/02/11現在、1.0.4以降のバージョンがリリースされていない。
1.0.4以降のmainの変更がなければクロックを内部発振器を使う設定に変更できない。
1.0.4移行のバージョンを利用できるようにした定義ファイルが以下に公開されている。
1.0.5-alphaをインストールする。

パッケージインストール用のJSONファイルは以下のURLを追加する。

```
https://raw.githubusercontent.com/robinjanssens/WCH32V_board_manager_files/main/package_ch32v_index.json
```

### arduino_wch32v003

https://github.com/AlexanderMandera/arduino-wch32v003

ch32v003funをベースにしたArduino環境。

パッケージインストール用のJSONファイルは以下のURLを追加する。

```
https://alexandermandera.github.io/arduino-wch32v003/package_ch32v003_index.json
```

### UIAPduino

https://www.uiap.jp/uiapduino/pro-micro/ch32v003/v1dot4

UIAPduinoというCH32V003ボードのための環境
WCH公式のArduino環境のforkになっている
USB経由で書き込めるbootloaderが入っている
このbootloaderに書き込むには[専用のforkのminichlink](https://github.com/YuukiUmeta-UIAP/ch32v003fun/tree/master/minichlink)が必要

パッケージインストール用のJSONファイルは以下のURLを追加する。

```
https://github.com/YuukiUmeta-UIAP/board_manager_files/raw/main/package_uiap.jp_index.json
```
