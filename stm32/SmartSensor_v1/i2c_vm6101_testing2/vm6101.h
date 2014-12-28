#ifndef _VM6101_H_
#define _VM6101_H_

#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#define COLOR_TX_DEPTH 8
#define COLOR_RX_DEPTH 24

#define COLOR_CONTROL_REG 0x02
#define COLOR_Y_STATUS_REG 0x04
#define COLOR_R_STATUS_REG 0x09
#define COLOR_G_STATUS_REG 0x0e
#define COLOR_B_STATUS_REG 0x13

enum{
Y_STATUS,Y_CNT3,Y_CNT2,Y_CNT1,Y_CNT0,
R_STATUS,R_CNT3,R_CNT2,R_CNT1,R_CNT0,
G_STATUS,G_CNT3,G_CNT2,G_CNT1,G_CNT0,
B_STATUS,B_CNT3,B_CNT2,B_CNT1,B_CNT0,
};

extern uint32_t y_cnt_val,r_cnt_val,g_cnt_val,b_cnt_val;
extern uint32_t y_lx_val,r_lx_val,g_lx_val,b_lx_val;

int init_vm6101(void);
void request_color_data(void);
uint32_t get_24bit_cnt(uint8_t status_index, uint8_t * rx_array);
extern inline void get_lx_from_cnts(void);
#endif
