#SM25 Fingerprint Library For STM32
 1)Open Stm32CubeMX.
 2)Enable a gpio as input for Finger detection pin.
 3)Enable USART and Enable RX interrupt(115200 default module).
 4)Change Selected USART to LL Library.
 5)Config your Board on "FingerprintConfig.h"
 6)Put FP_RxCallBack() function to your Interrupt routin on "stm32fxxx_it.c".
 7)Put FP_Loop() into your "while" on Mani Function.
 8)Call FP_Init().
