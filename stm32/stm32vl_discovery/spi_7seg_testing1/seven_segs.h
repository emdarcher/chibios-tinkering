
#ifndef _SEVEN_SEGS_H_
#define _SEVEN_SEGS_H_

//header file with seven segment stuff

#include <stdint.h>

/*
segments
 ---A---
|       |
F       B
|___G___|
|       |
E       C
|___D___|
*/
/*
pinout of display
        ________     
C1    --| ---- |--  G1
E1    --|digit1|--  A1
D1    --| ---- |--  F1
Vcc1  --|      |--  B1
Vcc0  --| ---- |--  B0
D0    --|digit0|--  F0
E0    --| ---- |--  A0
C0    --|______|--  G0

common adode for each digit
pull segment's pin LOW to turn it on
*/

//these are the bits of the shift register
//that go to particular segments
#define SEG_A (1<<7)
#define SEG_B (1<<6)
#define SEG_C (1<<5)
#define SEG_D (1<<4)
#define SEG_E (1<<3)
#define SEG_F (1<<2)
#define SEG_G (1<<1)

#define ALL_SEGS ( SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G )

//make sure to handle digit selection in software

#define DIG_0 (1<<0)
#define DIG_1 (1<<1)
#define DIG_2 (1<<2)
#define DIG_3 (1<<3)

//remember to add any newly defines digits here
#define ALL_DIGS ( DIG_0 | DIG_1 | DIG_2 | DIG_3 )

const uint16_t digit_bits[] = { DIG_0, DIG_1, DIG_2, DIG_3 };
const uint8_t  num_digits = sizeof(digit_bits)/2;

const uint8_t number_seg_bytes[] = {
//       unconfigured
//ABCDEFG^
0b11111100,//0
0b01100000,//1
0b11011010,//2
0b11110010,//3
0b01100110,//4
0b10110110,//5
0b10111110,//6
0b11100000,//7
0b11111110,//8
0b11100110,//9
0b10011110, //'E' for error
};

#endif
