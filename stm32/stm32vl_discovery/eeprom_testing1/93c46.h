
#ifndef _93C46_H_
#define _93C46_H_

//set to "0" for 8-bit, "1" for 16-bit organization
#define 93C46_ORG 1

#if 93C46_ORG==0
    #error "there currently isn't support for 8-bit organization in code for 93c46"
#endif

#if 93C46_ORG==0
#define 93C46_ADDR_LEN 7
#else
#define 93C46_ADDR_LEN 6
#endif

#define 93C46_NUM_BYTES 128
#define 93C46_NUM_WORDS (93C46_NUM_BYTES>>1)

#define 93C46_SOFT_SPI TRUE
#if 93C46_SOFT_SPI==FALSE
    #error "only supports soft SPI for 93C46 eeprom currently"
#endif

#define 93C46_READ  0b10
#define 93C46_ERASE 0b11
#define 93C46_WRITE 0b01
#define 93C46_EWEN  0b00
#define 93C46_EWDS  0b00
#define 93C46_ERAL  0b00
#define 93C46_WRAL  0b00

#define 93C46_EWEN_ADDR 0b11
#define 93C46_EWDS_ADDR 0b00
#define 93C46_ERAL_ADDR 0b10
#define 93C46_WRAL_ADDR 0b01



#if 93C46_SOFT_SPI
#define 93C46_CS_GPIO   GPIOD
#define 93C46_CS_NUM    2
#define 93C46_SK_GPIO   GPIOC
#define 93C46_SK_NUM    12
#define 93C46_MOSI_GPIO GPIOC
#define 93C46_MOSI_NUM  11
#define 93C46_MISO_GPIO GPIOC
#define 93C46_MISO_NUM  10
#endif



#endif

