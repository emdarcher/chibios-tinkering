*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M3 STM32F103.                            **
*****************************************************************************

** TARGET **

The demo runs on a SmartSensor_v1 board.

** The Demo **

The demo flashes and LED on PA3 within a thread.
And flashes an LED on PA2 within the main thread.
Prints the 12-bit analog value from PA1 over Serial 1 at 9600 baud.

** Build Procedure **

built using arm-none-eabi open source toolchain.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
