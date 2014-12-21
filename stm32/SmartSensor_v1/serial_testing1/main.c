#include "ch.h"
#include "hal.h"
//#include "test.h"

//#define CH_NO_IDLE_THREAD TRUE

/*
 * Blinker thread.
 */

/*
 * Red LED blinker thread, times are in milliseconds.
 */
static WORKING_AREA(waThread1, 128);
static msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(GPIOA, GPIOA_PA2);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOA, GPIOA_PA2);
    chThdSleepMilliseconds(500);
  }
}

static SerialConfig sd1conf = {
9600,
0,
0,
0,
};

static WORKING_AREA(waThread2, 128);
static msg_t Thread2(void *arg) {

    (void)arg;
    chRegSetThreadName("messenger");
    static char hello_data[] = "hello\n\r";
    while(TRUE){
        sdWrite(&SD1,(uint8_t *) hello_data, sizeof(hello_data));
        //sdPut(&SD1,'h');
        //sdPut(&SD1,'\n');
        chThdSleepMilliseconds(1000);
    }

}


int main(void){
    
    halInit();
    chSysInit();
    
    sdStart(&SD1,&sd1conf);
    //sdStart(&SD1,NULL);

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);
    chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);

//    palSetPadMode(GPIOA, GPIOA_PA10, PAL_MODE_INPUT_ALTERNATE);
  //  palSetPadMode(GPIOA, GPIOA_PA9, PAL_MODE_OUTPUT_ALTERNATE);
    
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
