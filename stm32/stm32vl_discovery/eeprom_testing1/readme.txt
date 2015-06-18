
** TARGET **

The demo runs on a STM32VL_DISCOVERY board.

** The Program **

This Program flashes the on-board LEDs attached to PC9 in a thread.
This program also controls a 4-digit mulitiplexed 7-segment
display using a shift register, which is manipulated via SPI1,
and 4 NPN transistors triggered by PC0-3.
This test using a shell prompt and custom command to change the ouput on the seven segment display.
This is also testing code for reading and writing to an 93C46 1Kbit EEPROM.

** Build Procedure **

built using arm-none-eabi open source toolchain.

** Notes **

Some files used by the demo are not part of ChibiOS/RT but are copyright of
ST Microelectronics and are licensed under a different license.
Also note that not all the files present in the ST library are distributed
with ChibiOS/RT, you can find the whole library on the ST web site:

                             http://www.st.com
