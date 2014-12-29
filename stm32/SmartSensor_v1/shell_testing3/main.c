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
/* SHELL_MAX_ARGUMENTS can be changed in the Makefile */
//#define     SHELL_MAX_ARGUMENTS   6

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

#define CMD_STR_ACCEL "accel"
#define CMD_STR_COLOR "color"
#define CMD_STR_LED "led"
#define CMD_STR_LOOP "loop"

enum{CMD_ACCEL,CMD_COLOR,CMD_LED,CMD_LOOP,  };
const char * cmd_name_strings[] = {
CMD_STR_ACCEL,CMD_STR_COLOR,CMD_STR_LED,CMD_STR_LOOP,
};
/* stores number of strings in the cmd_name_strings array */
const uint8_t cmd_name_strings_len = 
    sizeof(cmd_name_strings) / sizeof(cmd_name_strings[0]);

static void call_cmd_from_index( BaseSequentialStream *chp, int argc, char *argv[], uint8_t cmd_index);

static const char invalid_num_str[] = "%s is not a valid number!\n\r";
static const char invalid_arg_str[] = "%s is an invalid argument!\n\r";

static const char color_usage[] = "usage: color [ -h | -l <loop_times>]\n\r"
                    "\t-h: prints help\n\r"
                    "\t-l <loop_times> : loops the amount of times\n\r";

static const char loop_usage[] = "loop usage: loop <command> [ -h | -l <loop_times>]\n\r"
                    "\t-h: prints help\n\r"
                    "\t-l <loop_times> : loops the amount of times\n\r";
static uint8_t str_is_valid_num(char * str){
    uint8_t nI;
    for(nI=0;nI<strlen(str);nI++){
        char chk = str[nI];
        if(!((chk>=48)&&(chk<=57))){
            return 0;
        }
    }
    return 1;
}

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
    #define COLOR_USABLE_WITH_LOOP TRUE
    #if !COLOR_USABLE_WITH_LOOP 
    uint32_t loop_times=1;
    uint32_t loop_delay_ms=100;
    if(argc > 0){
        if(strcmp("-l", argv[0])==0){
            if(argc > 1){
                if(str_is_valid_num(argv[1])){ 
                    loop_times = atol(argv[1]);
                    if(argc > 2){
                        if(strcmp("-s", argv[2])==0){
                            if(argc > 3){
                                if(str_is_valid_num(argv[3])){
                                    loop_delay_ms = atol(argv[3]);
                                } else {
                                    chprintf(chp,invalid_num_str,argv[3]);
                                }
                            } else {
                                chprintf(chp,"please enter millisecond delay\n\r");
                            }
                        } else {
                            chprintf(chp,invalid_arg_str,argv[2]);
                        }
                    }  
                } else {
                    chprintf(chp,invalid_num_str, argv[1]);
                }
            } else {
                chprintf(chp, "please enter a number of times to loop\n\r");
            }
        } else if(strcmp("-h",argv[0])==0){
            chprintf(chp, "printing help:\n\r");
            chprintf(chp, color_usage);
        } else {
            chprintf(chp, invalid_arg_str, argv[0]);
            chprintf(chp, color_usage);
        }
    }
    chprintf(chp, "looping %U times with delay of %U milliseconds:\n\r", 
        loop_times,loop_delay_ms);
    while(loop_times--){
    #endif
        #if !USE_I2C_POLL_THD 
        request_color_data();
        #endif
        chSysLockFromIsr();
        get_lx_from_cnts();
        chSysUnlockFromIsr();
        chprintf(chp, "color_lx: y=%U\tr=%U\tg=%U\tb=%U\n\r",
            y_lx_val,r_lx_val,g_lx_val,b_lx_val);
    #if !COLOR_USABLE_WITH_LOOP 
        chThdSleepMilliseconds(loop_delay_ms);
    }
    #endif
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
static void cmd_loop(BaseSequentialStream *chp, int argc, char *argv[]) {
    uint32_t loop_times=1;
    uint32_t loop_delay_ms=100;
    uint8_t cmd_match=0;
    int pass_argc=0;
    char *pass_argv[0];
if(argc > 0){
    uint8_t cmd_i;
    for(cmd_i=0;cmd_i<cmd_name_strings_len;cmd_i++){
        if(strcmp(cmd_name_strings[cmd_i],argv[0])==0){
            cmd_match++;
            break;
        }
    }
    if(cmd_match==1){
    if(argc > 1){
        if(strcmp("-l", argv[1])==0){
            if(argc > 2){
                if(str_is_valid_num(argv[2])){ 
                    loop_times = atol(argv[2]);
                    if(argc > 3){
                        if(strcmp("-s", argv[3])==0){
                            if(argc > 4){
                                if(str_is_valid_num(argv[4])){
                                    loop_delay_ms = atol(argv[4]);
                                } else {
                                    chprintf(chp,invalid_num_str,argv[4]);
                                }
                            } else {
                                chprintf(chp,"please enter millisecond delay\n\r");
                            }
                        } else {
                            chprintf(chp,invalid_arg_str,argv[3]);
                        }
                    }  
                } else {
                    chprintf(chp,invalid_num_str, argv[2]);
                }
            } else {
                chprintf(chp, "please enter a number of times to loop\n\r");
            }
        } else if(strcmp("-h",argv[1])==0){
            //chprintf(chp, "printing help:\n\r");
            chprintf(chp, loop_usage);
        } else {
            chprintf(chp, invalid_arg_str, argv[1]);
            chprintf(chp, loop_usage);
        }
    }
    chprintf(chp, "looping %s command %U times with delay of %U milliseconds:\n\r", 
        argv[0],loop_times,loop_delay_ms);

    while(loop_times--){
        call_cmd_from_index(chp,pass_argc,pass_argv,cmd_i);        
        chThdSleepMilliseconds(loop_delay_ms);
    } 

    } else if(strcmp("-h",argv[0])==0){
        chprintf(chp, loop_usage);
    } else {
        chprintf(chp, "%s is not a designated command!\n\r", argv[0]);
        chprintf(chp, loop_usage);
    }


} else {
    chprintf(chp, "please enter a cmd to loop\n\r");
    chprintf(chp, loop_usage);
}

}

static void call_cmd_from_index( BaseSequentialStream *chp, int argc, char *argv[], uint8_t cmd_index){
    switch (cmd_index){
    case CMD_ACCEL:
        cmd_accel(chp, argc, argv);
        break;
    case CMD_COLOR:
        cmd_color(chp, argc, argv);
        break;
    case CMD_LED:
        cmd_led(chp, argc, argv);
        break;
    case CMD_LOOP:
        cmd_loop(chp, argc, argv);
        break;
    }
}

static const ShellCommand shCmds[] = {
    {CMD_STR_ACCEL, cmd_accel},
    {CMD_STR_COLOR, cmd_color},
    {CMD_STR_LED,cmd_led},   
    {CMD_STR_LOOP,cmd_loop},
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
