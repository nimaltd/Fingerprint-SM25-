# SM25 Fingerprint Library For STM32
 <br />
 1)Open Stm32CubeMX.
  <br />
 2)Enable a gpio as input for Finger detection pin.
  <br />
 3)Enable USART and Enable RX interrupt(115200 default module).
  <br />
 4)Change Selected USART to LL Library.
  <br />
 5)Config your Board on "FingerprintConfig.h"
  <br />
 6)Put FP_RxCallBack() function to your Interrupt routin on "stm32fxxx_it.c".
  <br />
 7)Put FP_Loop() into your "while" on Mani Function.
  <br />
 8)Call FP_Init().
  <br />
