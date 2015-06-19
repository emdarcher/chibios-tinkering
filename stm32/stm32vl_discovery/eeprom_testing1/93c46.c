
#include "93c46.h"

inline void init_93c46_pins(void){
    /* initializes the pins to use for soft SPI to the 93c46 */
    palSetPadMode(E_93C46_CS_GPIO,E_93C46_CS_NUM,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(E_93C46_SK_GPIO,E_93C46_SK_NUM,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(E_93C46_MOSI_GPIO,E_93C46_MOSI_NUM,PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(E_93C46_MISO_GPIO,E_93C46_MISO_NUM,PAL_MODE_INPUT);
}

void shift_out_bit_93c46(uint8_t bit){
    /* shifts out the bit value in "bit" to MOSI */
    E_93C46_CLR_SK();
    palWritePad(E_93C46_MISO_GPIO,E_93C46_MOSI_NUM,bit);
    //chThdSleepMicroseconds(E_93C46_PULSE_DELAY_US);
    int i;for(i=0;i<1000;i++);
    E_93C46_SET_SK();
}

uint8_t shift_in_bit_93c46(void){
    /* toggles the SK and reads in the bit value on MISO */
    E_93C46_CLR_SK();
    //chThdSleepMicroseconds(E_93C46_PULSE_DELAY_US);
    E_93C46_SET_SK();
    return palReadPad(E_93C46_MISO_GPIO,E_93C46_MISO_NUM);
}
void send_cmd_93c46(uint8_t opcode, uint8_t addr){
//sends an opcode and address in the command format for the 93c46
    shift_out_bit_93c46(E_93C46_START_BIT); //start bit
    uint16_t temp_bits = ((opcode<<E_93C46_ADDR_LEN) | addr);
    uint8_t temp_len = E_93C46_ADDR_LEN + 2;
    while(temp_len--){
        shift_out_bit_93c46((temp_bits>>temp_len) & 1);
    }
}

void shift_word_93c46(uint16_t out_word){
//shifts out a word (16-bits) to the 93c46
    uint8_t temp_len = 16;
    while(temp_len--){
        shift_out_bit_93c46((out_word>>temp_len) & 1);
    }
}

void write_enable_93c46(void){
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_EWEN,E_93C46_EWEN_ADDR);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
}
void write_disable_93c46(void){
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_EWDS,E_93C46_EWDS_ADDR);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
}
void erase_all_93c46(void){
//erases all addresses
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_ERAL,E_93C46_ERAL_ADDR);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
    poll_93c46();
}
                                               
void write_93c46(uint8_t addr, uint16_t data){
//writes "data" to an "addr" 
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_WRITE,addr);
    shift_word_93c46(data);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
    poll_93c46();
}

void write_all_93c46(uint16_t data){
//writes "data" to all addresses
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_WRAL,E_93C46_WRAL_ADDR);
    shift_word_93c46(data);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
    poll_93c46();
}

void erase_addr_93c46(uint8_t addr){
//erases from an "addr" 
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_ERASE,addr);
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
    poll_93c46();
}

uint16_t read_word_93c46(uint8_t addr){
    uint16_t temp_store = 0;
    E_93C46_SET_CS();
    send_cmd_93c46(E_93C46_READ,addr);
    shift_in_bit_93c46();//get the dummy bit out of there
    uint8_t temp_len = 16;
    while(temp_len--){
        temp_store |= (uint16_t)(shift_in_bit_93c46()<<temp_len);
    }
    E_93C46_CLR_CS();
    E_93C46_CLR_SK();
    return temp_store;
}
uint8_t poll_93c46(void){
    //polls the MISO (DO on 93c46) to see whan it's ready for more input
    //returns 1 (TRUE) if succesful within amount of poll times
    uint8_t temp_poll_num = E_93C46_POLL_TIMES;
    E_93C46_SET_CS();
    while(temp_poll_num--){
        if(palReadPad(E_93C46_MISO_GPIO,E_93C46_MISO_NUM)){
            E_93C46_CLR_CS();
            return 1;
        }
        chThdSleepMicroseconds(E_93C46_POLL_DELAY_US);
    }
    E_93C46_CLR_CS();
    return 0;
}


