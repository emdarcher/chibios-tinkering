*****************************************************************************
** ChibiOS/RT port for ARM-Cortex-M3 STM32F103.                            **
*****************************************************************************

** TARGET **

The demo runs on a SmartSensor_v1 board.

** The Demo **

The demo flashes and LED on PA2 within a thread.
This also uses the Command Shell on Serial 1,
type "help" in the shell prompt to see available commands,
most commands will give you help on how to use them if you
type the command line argument "-h" after the command.

** Build Procedure **

built using arm-none-eabi open source toolchain.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
