#include "ch.h"
#include "hal.h"
//#include "test.h"
/*
* Red LED blinker thread, times are in milliseconds.
*/
static WORKING_AREA(waThread1, 32);
static __attribute__((noreturn)) msg_t Thread1(void *arg) {
    (void)arg;
    chRegSetThreadName("blinker");
    while (TRUE) {
        palClearPad(GPIOA, GPIOA_PA2);
        chThdSleepMilliseconds(500);
        palSetPad(GPIOA, GPIOA_PA2);
        chThdSleepMilliseconds(500);
    }
}
/* config for the serial stuff */
static SerialConfig sd1conf = {
    9600,
    0,
    0,
    0,
};
static WORKING_AREA(waThread2, 32);
static __attribute__((noreturn)) msg_t Thread2(void *arg) {
    (void)arg;
    chRegSetThreadName("messenger");
    static char hello_data[] = "hello\n\r";
    while(TRUE) {
        sdWrite(&SD1,(uint8_t *) hello_data, sizeof(hello_data));
        chThdSleepMilliseconds(1000);
    }
}
/* pwm callback funtions */
static void pwm2pcb(PWMDriver *pwmp) {
    (void)pwmp;
    palSetPad(GPIOA, GPIOA_PA1);
}
static void pwm2c2cb(PWMDriver *pwmp) {
    (void)pwmp;
    palClearPad(GPIOA, GPIOA_PA1);
}
static PWMConfig pwmcfg2 = {
    200000UL,/* 200kHz pwm freq */
    1024, /* period 1024 ticks */
    pwm2pcb, /* callback */
    /* channel 2 only */
    {
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_ACTIVE_HIGH, pwm2c2cb},
        {PWM_OUTPUT_DISABLED, NULL},
        {PWM_OUTPUT_DISABLED, NULL},
    },
    0,
    0,
};
static WORKING_AREA(waPWMThread2, 32);
static __attribute__((noreturn)) msg_t PWMThread2(void *arg) {
    (void)arg;
    chRegSetThreadName("PWM Thread 2");
    uint16_t pwmVal2 = 1;
    palSetPadMode(GPIOA, GPIOA_PA1, PAL_MODE_OUTPUT_PUSHPULL);
    pwmStart(&PWMD2,&pwmcfg2);
    while(TRUE) {
        if(pwmVal2 >= 1024) {
            pwmVal2 = 1;
        }
        else {
            pwmVal2 <<= 1;
        }
        pwmEnableChannel(&PWMD2,1,pwmVal2);
        chThdSleepMilliseconds(100);
    }
}
int main(void) {
    halInit();
    chSysInit();
    sdStart(&SD1,&sd1conf);
    /* start PWM on TIM2 with our config */
    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);
    chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO, PWMThread2, NULL);
    palSetPadMode(GPIOA, GPIOA_PA2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);
//TestThread(&SD1);
    while (TRUE) {
        palSetPad(GPIOA, 3);
//TestThread(&SD1);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
    }
}
