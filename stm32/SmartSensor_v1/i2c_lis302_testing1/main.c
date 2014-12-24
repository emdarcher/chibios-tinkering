#include "ch.h"
#include "hal.h"
//#include "test.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 


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

static const char newline[2] = "\n\r";


int main(void){
    
    halInit();
    chSysInit();
    
    sdStart(&SD1,&sd1conf);

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
     
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
