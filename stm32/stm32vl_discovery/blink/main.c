#include "ch.h"
#include "hal.h"
//#include "test.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 

/*
 * Red LED blinker thread, times are in milliseconds.
 */

static WORKING_AREA(waThread1, 32);
static __attribute__((noreturn))  msg_t Thread1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(GPIOC, 9);
    chThdSleepMilliseconds(500);
    palSetPad(GPIOC, 9);
    chThdSleepMilliseconds(500);
  }
}

static const char newline[2] = "\n\r";


#if 0
static WORKING_AREA(waPWMThread2, 32);
static __attribute__((noreturn)) msg_t PWMThread2(void *arg){
    (void)arg;
    chRegSetThreadName("PWM Thread 2");
    uint16_t pwmVal2 = 1;
    palSetPadMode(GPIOA, GPIOA_PA1, PAL_MODE_OUTPUT_PUSHPULL);
    pwmStart(&PWMD2,&pwmcfg2);
    while(TRUE){
        if(pwmVal2 >= 1024){ pwmVal2 = 1; }
        else { pwmVal2 <<= 1; }
        pwmEnableChannel(&PWMD2,1,pwmVal2);
        chThdSleepMilliseconds(100);
    }
}
#endif

int main(void){
    
    halInit();
    chSysInit();
    

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    //chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    //chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO+1, PWMThread2, NULL);    

    palSetPadMode(GPIOC, 8, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 9, PAL_MODE_OUTPUT_PUSHPULL);
//TestThread(&SD1);

    while (TRUE){
        palSetPad(GPIOC, 8);
        //TestThread(&SD1);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOC, 8);
        chThdSleepMilliseconds(500);
    }
}
