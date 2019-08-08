#ifndef _FINGERPRINTCONFIG_H
#define _FINGERPRINTCONFIG_H

/*
+++   Nima Askari
+++   www.github.com/NimaLTD
+++   www.instagram.com/github.NimaLTD 
+++   Version: 1.0.0
*/

/*
SM25 Fingerprint Library For STM32
 1)Open CubeMX.
 2)Enable a gpio as input for Finger detection pin.
 3)Enable USART and Enable RX interrupt(115200 default module).
 4)Change Selected USART to LL Library.
 5)Select "General peripheral Initalizion as a pair of '.c/.h' file per peripheral" on project settings. 
 6)Config your Board on "FingerprintConfig.h"
 7)Put FP_RxCallBack() function to your Interrupt routin on "stm32fxxx_it.c".
 8)Put FP_Loop() into your "while" in Main Function.
 9)Call FP_Init().
*/

#define _FP_USART                     USART6
#define _FP_USE_FREERTOS              1
#define _FP_DEBUG                     2
#define _FP_DETECT_GPIO               GPIOA  
#define _FP_DETECT_PIN                GPIO_PIN_8

#endif
