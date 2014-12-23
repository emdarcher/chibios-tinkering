*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M3 STM32F100.                            **
*****************************************************************************

** TARGET **

The demo runs on a STM32VL_DISCOVERY board.

** The Demo **

This demo flashes the two on-board LEDs attached to PC8 and PC9
from within separate threads
This program also controls a 4-digit mulitiplexed 7-segment
display using a shift register, which is manipulated via SPI1,
and 4 NPN transistors triggered by PC0-3.

** Build Procedure **

built using arm-none-eabi open source toolchain.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
