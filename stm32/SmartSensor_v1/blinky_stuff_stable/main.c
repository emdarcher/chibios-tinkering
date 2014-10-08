#include "ch.h"
#include "hal.h"
#include "test.h"

//#define CH_NO_IDLE_THREAD

/*
 * Blinker thread.
 */
static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

  (void)arg;

  chRegSetThreadName("blinker");
  while (true) {
    palSetPad(GPIOC, 2);
    chThdSleepMilliseconds(500);
    palClearPad(GPIOC, 2);
    chThdSleepMilliseconds(500);
  }
}


int main(void){
    
    halInit();
    chSysInit();
    
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);


    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);

    while (true){
        palSetPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
    }
}
