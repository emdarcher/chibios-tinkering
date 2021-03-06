#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "shell.h"
#include "chprintf.h"

#define USE_I2C_STUFF TRUE
#define USE_SER_OUT_THD FALSE
#define USE_I2C_POLL_THD FALSE

#if USE_I2C_STUFF 
#include "i2c_pns.h"
#include "lis3.h"
#include "vm6101.h"
#endif

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define SHELL_WA_SIZE THD_WA_SIZE(1024)


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

#if USE_I2C_STUFF 
#if USE_I2C_POLL_THD
/*
 * Accelerometer thread
 */
static WORKING_AREA(PollAccelThreadWA, 256);
static __attribute__((noreturn)) msg_t PollAccelThread(void *arg) {
  chRegSetThreadName("PollAccel");
  (void)arg;
  while (TRUE) {
    chThdSleepMilliseconds(20);
    request_acceleration_data();
  }
}

/*
 * VM6101 Color Sensor reading thread 
 */
static WORKING_AREA(PollColorThreadWA, 256);
static __attribute__((noreturn)) msg_t PollColorThread(void *arg) {
  chRegSetThreadName("PollColor");
  (void)arg;
  while (TRUE) {
    chThdSleepMilliseconds(20);
    request_color_data();
  }
}
#endif
#endif
#if USE_SER_OUT_THD 
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
        get_lx_from_cnts();
        chprintf((BaseSequentialStream *)&SD1, 
            "accel: x:\t%d\ty:\t%d\tz:\t%d\tcolor_lx: y=%U\tr=%U\tg=%U\tb=%U\n\r", 
            accel_x,accel_y,accel_z,
            y_lx_val,r_lx_val,g_lx_val,b_lx_val); 
    }
}
#endif

static void cmd_accel(BaseSequentialStream *chp, int argc, char *argv[]) {
    #if !USE_I2C_POLL_THD 
    request_acceleration_data();
    #endif
    int8_t accel_x,accel_y,accel_z;
    chSysLockFromIsr();
    accel_x = acceleration_x;
    accel_y = acceleration_y;
    accel_z = acceleration_z;
    chSysUnlockFromIsr();
    chprintf(chp, "accel: x:\t%d\ty:\t%d\tz:\t%d\n\r",
        accel_x,accel_y,accel_z);
}
static void cmd_color(BaseSequentialStream *chp, int argc, char *argv[]) {
    #if !USE_I2C_POLL_THD 
    request_color_data();
    #endif
    chSysLockFromIsr();
    get_lx_from_cnts();
    chSysUnlockFromIsr();
    chprintf(chp, "color_lx: y=%U\tr=%U\tg=%U\tb=%U\n\r",
        y_lx_val,r_lx_val,g_lx_val,b_lx_val);
}

static void cmd_led(BaseSequentialStream *chp, int argc, char *argv[]) {
    
    if((argc > 0) && (argc < 2)){
        if(argv[0][0] == '1'){
            palSetPad(GPIOA,3);
            chprintf(chp, "led ON\n\r");
        } else if(argv[0][0] == '0'){
            palClearPad(GPIOA,3);
            chprintf(chp, "led OFF\n\r");
        } else if(argv[0][0] == 't'){
            palTogglePad(GPIOA,3);
            chprintf(chp, "led TOGGLE\n\r");
        }else {
            chprintf(chp, "%c is not a valid value for led!\n\r",argv[1][0]);
        }
    } else {
        if(argc >= 2){chprintf(chp, "too many arguments!\n\r");}
        chprintf(chp, "usage: led <led_state>\n\r\tled_state: 0=OFF 1=ON t=TOGGLE\n\r");
    }
}

static const ShellCommand shCmds[] = {
    {"accel", cmd_accel},
    {"color", cmd_color},
    {"led", cmd_led},
    {NULL, NULL}
};
static const ShellConfig shCfg = {
(BaseSequentialStream *)&SD1,
shCmds
};

int main(void){
    
    halInit();
    chSysInit();
    
    Thread *sh = NULL;   
 
    sdStart(&SD1,&sd1conf);
    //sdStart(&SD1,NULL);
    
    chThdSleepMilliseconds(200);
    
    #if USE_I2C_STUFF 
    I2CInit_pns();
    
    #if USE_I2C_POLL_THD 
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
    #endif
    #endif

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    
    #if USE_SER_OUT_THD  
    chThdCreateStatic(waSerOutThr1, sizeof(waSerOutThr1), NORMALPRIO, SerOutThr1, NULL);
    #endif
    palSetPadMode(GPIOA, GPIOA_PA2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);
//TestThread(&SD1);


    shellInit();    

    while (TRUE){
        //palSetPad(GPIOA, 3);
        //TestThread(&SD1);
        //chThdSleepMilliseconds(500);
        //palClearPad(GPIOA, 3);
        //chThdSleepMilliseconds(500);
        if (!sh)
            sh = shellCreate(&shCfg, SHELL_WA_SIZE, NORMALPRIO+1);
        else if (chThdTerminated(sh)) {
            chThdRelease(sh);
            sh = NULL;
        }
    //    chThdSleepMilliseconds(1000);
    }
}   
