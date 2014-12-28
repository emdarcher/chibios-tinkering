

#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "vm6101.h"

#define vm6101_addr 0x10

static uint8_t color_tx_data[COLOR_TX_DEPTH];
static uint8_t color_rx_data[COLOR_RX_DEPTH];

static i2cflags_t errors = 0;

uint32_t y_cnt_val,r_cnt_val,g_cnt_val,b_cnt_val;


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
    
  /* get the vals */
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
}


