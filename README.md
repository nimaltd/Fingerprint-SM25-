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
 5) Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings.
 <br />
 6)Config your Board on "FingerprintConfig.h"
  <br />
 7)Put FP_RxCallBack() function to your Interrupt routin on "stm32fxxx_it.c".
  <br />
 8)Put FP_Loop() into your "while" in Main Function.
  <br />
 9)Call FP_Init().
  <br />
