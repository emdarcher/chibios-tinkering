#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "chprintf.h"
#include "i2c_pns.h"
#include "lis3.h"
#include "vm6101.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 

static const char newline[2] = "\n\r";

static char * fourDig_to_str(uint16_t fourDig){
    static char out_str[4]="    ";
    static uint8_t digs[4];
    uint16_t val = fourDig;
    digs[0] = val / 1000U;
    val -= digs[0] * 1000U;
    digs[1] = val / 100U;
    val -= digs[1] * 100U;
    digs[2] = val / 10U;
    val -= digs[2] * 10U;
    digs[3] = val;
    uint8_t i=4;
    while(i--){
        out_str[i] = DIG_TO_CHAR(digs[i]);
    } 
    return (char *)out_str;
}

/* config for the serial stuff */
static SerialConfig sd1conf = {
38400, /* baud */
0,0,0, 
};

/*
 * Red LED blinker thread, times are in milliseconds.
 */

static WORKING_AREA(waThread1, 32);
static __attribute__((noreturn))  msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(GPIOA, GPIOA_PA2);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOA, GPIOA_PA2);
    chThdSleepMilliseconds(500);
  }
}

/*
 * Accelerometer thread
 */
static WORKING_AREA(PollAccelThreadWA, 256);
static msg_t PollAccelThread(void *arg) {
  chRegSetThreadName("PollAccel");
  (void)arg;
  while (TRUE) {
    /*chThdSleepMilliseconds(rand() & 31);*/
    chThdSleepMilliseconds(20);
    request_acceleration_data();
  }
  return 0;
}
static WORKING_AREA(PollColorThreadWA, 256);
static msg_t PollColorThread(void *arg) {
  chRegSetThreadName("PollColor");
  (void)arg;
  while (TRUE) {
    /*chThdSleepMilliseconds(rand() & 31);*/
    chThdSleepMilliseconds(20);
    request_color_data();
  }
  return 0;
}

static WORKING_AREA(waSerOutThr1,128);
static __attribute__((noreturn)) msg_t SerOutThr1(void *arg){
    chRegSetThreadName("serial_out");
    int8_t accel_x,accel_y,accel_z;
    
    (void)arg;
    while(TRUE){
        chThdSleepMilliseconds(100);
        chSysLockFromIsr();
        accel_x = acceleration_x;
        accel_y = acceleration_y;
        accel_z = acceleration_z;
        chSysUnlockFromIsr();
        chprintf((BaseSequentialStream *)&SD1, 
            "accel: x:\t%d\ty:\t%d\tz:\t%d\tcolor: y=%d  \tr=%d  \tg=%d  \tb=%d  \n\r", 
            accel_x,accel_y,accel_z,
            y_cnt_val,r_cnt_val,g_cnt_val,b_cnt_val); 
    }
}


int main(void){
    
    halInit();
    chSysInit();
    
    sdStart(&SD1,&sd1conf);
    
    chThdSleepMilliseconds(200);
    I2CInit_pns();
    
    /* Create accelerometer thread */
    chThdCreateStatic(PollAccelThreadWA,
        sizeof(PollAccelThreadWA),
        NORMALPRIO,
        PollAccelThread,
        NULL);
    chThdCreateStatic(PollColorThreadWA,
        sizeof(PollColorThreadWA),
        NORMALPRIO,
        PollColorThread,
        NULL);


    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);

    chThdCreateStatic(waSerOutThr1, sizeof(waSerOutThr1), NORMALPRIO, SerOutThr1, NULL);
     
    palSetPadMode(GPIOA, GPIOA_PA2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);
//TestThread(&SD1);

    while (TRUE){
        palSetPad(GPIOA, 3);
        //TestThread(&SD1);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
    }
}
