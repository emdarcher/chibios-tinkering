*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M3 STM32F103.                            **
*****************************************************************************

** TARGET **

The demo runs on a SmartSensor_v1 board.

** The Demo **

The demo flashes and LED on PA3 within a thread.
And flashes an LED on PA2 within the main thread.
Also prints "hello" over Serial from UART1 at 9600 baud.
This test program also outputs an exponentially increasing then resetting
pwm output on PA1, best visualized with an LED.

** Build Procedure **

built using arm-none-eabi open source toolchain.

** Notes **

tutorials found through the following url were useful:
http://www.telecom-robotics.org/2013/08/robotics-101-pwm-1/
but take note that they are not suitable directly for this chip, as
they are written for an STM32 F4 line device.

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
