
#ifndef _93C46_H_
#define _93C46_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

//set to "0" for 8-bit, "1" for 16-bit organization
#define E_93C46_ORG 1

#if E_93C46_ORG==0
    #error "there currently isn't support for 8-bit organization in code for 93c46"
#endif

#if E_93C46_ORG==0
#define E_93C46_ADDR_LEN 7
#else
#define E_93C46_ADDR_LEN 6
#endif

#define E_93C46_NUM_BYTES 128
#define E_93C46_NUM_WORDS (E_93C46_NUM_BYTES>>1)

#define E_93C46_SOFT_SPI 1
#if E_93C46_SOFT_SPI==0
    #error "only supports soft SPI for 93C46 eeprom currently"
#endif

#define E_93C46_START_BIT 0b1

#define E_93C46_READ  0b10
#define E_93C46_ERASE 0b11
#define E_93C46_WRITE 0b01
#define E_93C46_EWEN  0b00
#define E_93C46_EWDS  0b00
#define E_93C46_ERAL  0b00
#define E_93C46_WRAL  0b00

#define E_93C46_EWEN_ADDR (0b11<<(E_93C46_ADDR_LEN-2))
#define E_93C46_EWDS_ADDR (0b00<<(E_93C46_ADDR_LEN-2))
#define E_93C46_ERAL_ADDR (0b10<<(E_93C46_ADDR_LEN-2))
#define E_93C46_WRAL_ADDR (0b01<<(E_93C46_ADDR_LEN-2))




//#if E_93C46_SOFT_SPI
#define E_93C46_CS_GPIO   GPIOD
#define E_93C46_CS_NUM    2
#define E_93C46_SK_GPIO   GPIOC
#define E_93C46_SK_NUM    12
#define E_93C46_MOSI_GPIO GPIOC
#define E_93C46_MOSI_NUM  11
#define E_93C46_MISO_GPIO GPIOC
#define E_93C46_MISO_NUM  10
//#endif

#define E_93C46_POLL_TIMES 10
#define E_93C46_POLL_DELAY_US 1000 

//#define E_93C46_PULSE_DELAY_US 1
#define E_93C46_PULSE_DELAY_US 10

#define E_93C46_SET_CS() palSetPad(E_93C46_CS_GPIO,E_93C46_CS_NUM)
#define E_93C46_CLR_CS() palClearPad(E_93C46_CS_GPIO,E_93C46_CS_NUM)
#define E_93C46_SET_SK() palSetPad(E_93C46_SK_GPIO,E_93C46_SK_NUM)
#define E_93C46_CLR_SK() palClearPad(E_93C46_SK_GPIO,E_93C46_SK_NUM)


extern inline void init_93c46_pins(void);

extern void shift_out_bit_93c46(uint8_t bit);
extern uint8_t shift_in_bit_93c46(void);
extern void write_enable_93c46(void);
extern void write_disable_93c46(void);
extern void erase_all_93c46(void);
extern void write_all_93c46(uint16_t data);

extern void write_93c46(uint8_t addr, uint16_t data);
extern void erase_addr_93c46(uint8_t addr);
extern uint16_t read_word_93c46(uint8_t addr);
extern uint8_t poll_93c46(void);

extern void send_cmd_93c46(uint8_t opcode, uint8_t addr);
extern void shift_word_93c46(uint16_t out_word);

#endif

