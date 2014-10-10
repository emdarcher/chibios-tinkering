/*
    ChibiOS/RT - Copyright (C) 2006-2014 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#ifndef _BOARD_H_
#define _BOARD_H_

/*
 * Setup for SmartSensor_v1.
 */

/*
 * Board identifier.
 */
#define BOARD_SMARTSENSOR_V1_STM32_F103C8
#define BOARD_NAME              "SmartSensor_v1"

/*
 * Board frequencies.
 */

#define STM32_LSECLK            32768

#if defined(SMARTSENSOR_EXTERNAL_OSCILLATOR)
#define STM32_HSECLK            8000000
#define STM32_HSE_BYPASS

#elif defined(SMARTSENSOR_HSE_CRYSTAL)
#define STM32_HSECLK            8000000

#else
#define STM32_HSECLK            0
#endif


//#define STM32_HSECLK            8000000

/*
 * MCU type, supported types are defined in ./os/hal/platforms/hal_lld.h.
 */
#define STM32F10X_MD

/*
 * IO pins assignments.
 */

enum {
GPIOA_PA0,GPIOA_PA1,GPIOA_PA2,GPIOA_PA3,GPIOA_PA4,GPIOA_PA5,GPIOA_PA6,GPIOA_PA7,
GPIOA_PA8,GPIOA_PA9,GPIOA_PA10,GPIOA_PA11,GPIOA_PA12,GPIOA_PA13,GPIOA_PA14,GPIOA_PA15,
};

enum {
GPIOB_PB0,GPIOB_PB1,GPIOB_PB2,GPIOB_PB3,GPIOB_PB4,GPIOB_PB5,GPIOB_PB6,GPIOB_PB7,
GPIOB_PB8,GPIOB_PB9,GPIOB_PB10,GPIOB_PB11,GPIOB_PB12,GPIOB_PB13,GPIOB_PB14,GPIOB_PB15,
};
#define GPIOC_PC13_TAMPER_RTC 	13
#define GPIOC_PC14_OSC32_IN 	14
#define GPIOC_PC15_OSC32_OUT 	15	

enum {
GPIOD_PD0_OSC_IN,
GPIOD_PD1_OSC_OUT,
};

/*
 * I/O ports initial setup, this configuration is established soon after reset
 * in the initialization code.
 *
 * The digits have the following meaning:
 *   0 - Analog input.
 *   1 - Push Pull output 10MHz.
 *   2 - Push Pull output 2MHz.
 *   3 - Push Pull output 50MHz.
 *   4 - Digital input.
 *   5 - Open Drain output 10MHz.
 *   6 - Open Drain output 2MHz.
 *   7 - Open Drain output 50MHz.
 *   8 - Digital input with PullUp or PullDown resistor depending on ODR.
 *   9 - Alternate Push Pull output 10MHz.
 *   A - Alternate Push Pull output 2MHz.
 *   B - Alternate Push Pull output 50MHz.
 *   C - Reserved.
 *   D - Alternate Open Drain output 10MHz.
 *   E - Alternate Open Drain output 2MHz.
 *   F - Alternate Open Drain output 50MHz.
 * Please refer to the STM32 Reference Manual for details.
 */

/*
 * Port A setup.
 * Everything input with pull-up except:
 * PA2  - Alternate output  (USART2 TX).
 * PA3  - Normal input      (USART2 RX).
 * PA9  - Alternate output  (USART1 TX).
 * PA10 - Normal input      (USART1 RX).
 * 
 *  * PA13 - Pull-up input             (GPIOA_SWDIO).
 * PA14 - Pull-down input           (GPIOA_SWCLK)
 */
#define VAL_GPIOACRL            0x88884B88      /*  PA7...PA0 */
#define VAL_GPIOACRH            0x888884B8      /* PA15...PA8 */
#define VAL_GPIOAODR            0xFFFFBFFF

/*
 * Port B setup.
 * Everything input with pull-up except:
 *  //PB1  - Push Pull output  (LED).
 *  PB3  - Pull-up input             (GPIOA_SWO).
 * should maybe make PB7 and PB6 open-drain for I2C
 */
#define VAL_GPIOBCRL            0x88888888      /*  PB7...PB0 */
#define VAL_GPIOBCRH            0x88888888      /* PB15...PB8 */
#define VAL_GPIOBODR            0xFFFFFFFF

/*
 * Port C setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIOCCRL            0x88888888      /*  PC7...PC0 */
#define VAL_GPIOCCRH            0x88888888      /* PC15...PC8 */
#define VAL_GPIOCODR            0xFFFFFFFF

/*
 * Port D setup.
 * Everything input with pull-up except:
 * PD0  - Normal input (XTAL).
 * PD1  - Normal input (XTAL).
 */
#define VAL_GPIODCRL            0x88888844      /*  PD7...PD0 */
#define VAL_GPIODCRH            0x88888888      /* PD15...PD8 */
#define VAL_GPIODODR            0xFFFFFFFF

/*
 * Port E setup.
 * Everything input with pull-up except:
 */
#define VAL_GPIOECRL            0x88888888      /*  PE7...PE0 */
#define VAL_GPIOECRH            0x88888888      /* PE15...PE8 */
#define VAL_GPIOEODR            0xFFFFFFFF

/*
 * USB bus activation macro, required by the USB driver.
 */
#define usb_lld_connect_bus(usbp) //palClearPad(GPIOC, GPIOC_USB_DISC)

/*
 * USB bus de-activation macro, required by the USB driver.
 */
#define usb_lld_disconnect_bus(usbp) //palSetPad(GPIOC, GPIOC_USB_DISC)

#if !defined(_FROM_ASM_)
#ifdef __cplusplus
extern "C" {
#endif
  void boardInit(void);
#ifdef __cplusplus
}
#endif
#endif /* _FROM_ASM_ */

#endif /* _BOARD_H_ */
