

#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "vm6101.h"

#define vm6101_addr 0x10

static uint8_t color_tx_data[COLOR_TX_DEPTH];
static uint8_t color_rx_data[COLOR_RX_DEPTH];

static i2cflags_t errors = 0;

uint32_t y_cnt_val,r_cnt_val,g_cnt_val,b_cnt_val;
uint32_t y_lx_val,r_lx_val,g_lx_val,b_lx_val;

/**
 * Init function. Here we will also start personal serving thread.
 */
int init_vm6101(void){
  msg_t status = RDY_OK;
  systime_t tmo = MS2ST(4);

    /* config */
    //color_tx_data[0] =  

  /* sending */
  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, vm6101_addr, color_tx_data, 4, color_rx_data, 0, tmo);
  i2cReleaseBus(&I2CD1);

  if (status != RDY_OK){
    errors = i2cGetErrors(&I2CD1);
  }

  return 0;
}


/**
 *
 */
void request_color_data(void){
  msg_t status = RDY_OK;
  systime_t tmo = MS2ST(4);

  color_tx_data[0] = COLOR_Y_STATUS_REG; /* register address */
  i2cAcquireBus(&I2CD1);
  status = i2cMasterTransmitTimeout(&I2CD1, vm6101_addr, color_tx_data, 1, color_rx_data, 20, tmo);
  i2cReleaseBus(&I2CD1);

  if (status != RDY_OK){
    errors = i2cGetErrors(&I2CD1);
  }

#define USE_GET_24BIT_CNT_FUNC TRUE    
  /* get the vals */
#if !USE_GET_24BIT_CNT_FUNC 
    y_cnt_val = (color_rx_data[Y_CNT3]<<24) |
                (color_rx_data[Y_CNT2]<<16) |  
                (color_rx_data[Y_CNT1]<<8) |  
                (color_rx_data[Y_CNT0]<<0) ;  
    r_cnt_val = (color_rx_data[R_CNT3]<<24) |
                (color_rx_data[R_CNT2]<<16) |  
                (color_rx_data[R_CNT1]<<8) |  
                (color_rx_data[R_CNT0]<<0) ;  
    g_cnt_val = (color_rx_data[G_CNT3]<<24) |
                (color_rx_data[G_CNT2]<<16) |  
                (color_rx_data[G_CNT1]<<8) |  
                (color_rx_data[G_CNT0]<<0) ;  
    b_cnt_val = (color_rx_data[B_CNT3]<<24) |
                (color_rx_data[B_CNT2]<<16) |  
                (color_rx_data[B_CNT1]<<8) |  
                (color_rx_data[B_CNT0]<<0) ;  
#else
    y_cnt_val = get_24bit_cnt(Y_STATUS,color_rx_data);
    r_cnt_val = get_24bit_cnt(R_STATUS,color_rx_data);
    g_cnt_val = get_24bit_cnt(G_STATUS,color_rx_data);
    b_cnt_val = get_24bit_cnt(B_STATUS,color_rx_data);
#endif 

    

}

uint32_t get_24bit_cnt(uint8_t status_index, uint8_t * rx_array){
    return      (rx_array[status_index+1]<<24) |
                (rx_array[status_index+2]<<16) |
                (rx_array[status_index+3]<<8) |
                (rx_array[status_index+4]<<0) ;

}

/* gets ls value from the count values using 
 * approximations of the formulas in the datasheet
 * which are:

Y channel: EvY = 1.58e6 x count^-0.960
R channel: EvR = 3.34e6 x count^-0.902
G channel: EvG = 4.92e6 x count^-0.944
B channel: EvB = 8.03e6 x count^-0.973

we will change the exponent to be -1 for simplification so 
EvY = 1580000 / count
EvR = 3340000 / count
EvG = 4920000 / count
EvB = 8030000 / count

 */
inline void get_lx_from_cnts(void){
    
    y_lx_val = 1580000UL / y_cnt_val;
    r_lx_val = 3340000UL / r_cnt_val;
    g_lx_val = 4920000UL / g_cnt_val;
    b_lx_val = 8030000UL / b_cnt_val;
    
}
