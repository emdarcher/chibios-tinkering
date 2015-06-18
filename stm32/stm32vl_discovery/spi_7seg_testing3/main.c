/*
 *@author Ethan Durrant 2014
 *@file main.c
 *@breif main code source
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ch.h"
#include "hal.h"
#include "test.h"
#include "shell.h"
#include "chprintf.h"
#include "seven_segs.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 

#define DIG_SWITCH_DELAY_MS 4
#define DIG_SWITCH_DELAY_US 250

#define USE_SPI_CB FALSE

#define SHELL_WA_SIZE THD_WA_SIZE(1024)
/* SHELL_MAX_ARGUMENTS can be changed in the Makefile */
//#define     SHELL_MAX_ARGUMENTS   6

#define CMD_LED_GPIO GPIOC
#define CMD_LED_NUM  8
#define BLINKER_LED_GPIO GPIOC
#define BLINKER_LED_NUM 9

volatile uint16_t glbl_digs_var = 1234;

static inline void init_SPI1(void);
//void init_digit_pins(void);
void write_SPI1(uint8_t out_byte);
void msg_error(void);

void write_number(int16_t number);
void write_digit(int8_t num, uint8_t dig);
//volatile  bool_t SPI_done = FALSE;

static inline void init_digit_pins(void){
//    palSetGroupMode(GPIOC, ALL_DIGS, 0,PAL_MODE_OUTPUT_PUSHPULL );
    palSetPadMode(GPIOC, 0, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 1, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 2, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(GPIOC, 3, PAL_MODE_OUTPUT_PUSHPULL);
}

static inline void init_SPI1(void){
    //palSetPadMode(GPIOA, 4, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(GPIOA, 5, PAL_MODE_STM32_ALTERNATE_PUSHPULL);
    palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);

    palSetPadMode(GPIOA,3,PAL_MODE_OUTPUT_PUSHPULL);
}

/* spi callback proto */
#if USE_SPI_CB
static void spicb(SPIDriver *spip);
#endif
/*
 * SPI configuration structure.
 * speed clk/div4, CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOA_SPI1NSS on the port GPIOA.
 */
static const SPIConfig spicfg = {
  #if USE_SPI_CB
    spicb,
  #else
    NULL,
  #endif
  /* HW dependent part.*/
  GPIOA,
  GPIOA_SPI1NSS,
  SPI_CR1_BR_2,//cr1 set baud to clk/div4
};

/*
 * SPI end transfer callback.
 */
#if USE_SPI_CB
static void spicb(SPIDriver *spip) {
 
    if(spip->state == SPI_COMPLETE){
    /* On transfer end just releases the slave select line.*/
    chSysLockFromIsr();
    //SPI_done = TRUE;
    spiUnselectI(spip);
//    palSetPad(GPIOC,4);
    chSysUnlockFromIsr();
    }
}
#endif


/* function checks if a string is a valid number, if it finds
no non-digit characters, then it returns 1 for true, else 0 for false */
static uint8_t str_is_valid_num(char * str){
    uint8_t nI;
    for(nI=0;nI<strlen(str);nI++){
        char chk = str[nI];
        if(!((chk>=48)&&(chk<=57))){ return 0; }
    } return 1;
}

void write_digit(int8_t num, uint8_t dig){
     
    uint8_t out_bytes[1]; /*= {
        (((num<10)&&(num>=0)) ? number_seg_bytes[num] : number_seg_bytes[10]),
    };*/
    out_bytes[0] =(((num<10)&&(num>=0)) ? number_seg_bytes[num] : number_seg_bytes[10]); 
    chSysLockFromIsr();
    palClearPad(GPIOA,3);
    /* SPI slave selection and transmission start.*/
    spiSelectI(&SPID1);
    //spiStartSendI(&SPID1, 1, out_bytes);
    spiPolledExchange(&SPID1, out_bytes[0]);
    uint8_t nd = NUM_DIGS;
    while(nd--){
        if(nd == dig){
            palSetPad(GPIOC,nd); 
        } else {
            palClearPad(GPIOC,nd);
        }
    }
    spiUnselectI(&SPID1);
    palSetPad(GPIOA, 3);
    chSysUnlockFromIsr();
    //while(SPI_done == FALSE);
    chThdSleepMicroseconds(DIG_SWITCH_DELAY_US);
    //chThdSleepMilliseconds(DIG_SWITCH_DELAY_MS); 
}

void msg_error(void){
    write_digit(10, 0);
}

void write_number(int16_t number){
        uint8_t h;
        int16_t format_num = number;
        //check if number is too big ot not
        if ((number < 10000) && (number >= 0)){
            //formats number based on digits to correct digits on display
            //for(h=0;h < num_digits;h++){
            h = NUM_DIGS;
            while(h--){
                write_digit(format_num % 10, h);
                format_num /= 10;
            }
        } else {
            msg_error();
        }
}



static WORKING_AREA(waSegThread1,32);
static __attribute__((noreturn)) msg_t SegThread1(void *arg){
    (void)arg;
    chRegSetThreadName("seg thread");
    while(TRUE){
        //write_digit(1,0);
        //write_digit(2,1);
        //write_digit(3,2);
        //write_digit(4,3);
        write_number(glbl_digs_var);
    }
}

/*
 * Red LED blinker thread, times are in milliseconds.
 */

static WORKING_AREA(waBlinker1, 32);
static __attribute__((noreturn))  msg_t Blinker1(void *arg) {

  (void)arg;
  chRegSetThreadName("blinker");
  while (TRUE) {
    palClearPad(BLINKER_LED_GPIO, BLINKER_LED_NUM);
    chThdSleepMilliseconds(500);
    palSetPad(BLINKER_LED_GPIO, BLINKER_LED_NUM);
    chThdSleepMilliseconds(500);
  }
}

static const char newline[2] = "\n\r";

#define CMD_STR_LED "led"
#define CMD_STR_LOOP "loop"

enum{CMD_LED,CMD_LOOP};
const char * cmd_name_strings[] = {
CMD_STR_LED, CMD_STR_LOOP,
};


/* stores number of strings in the cmd_name_strings array */
const uint8_t cmd_name_strings_len = 
    sizeof(cmd_name_strings) / sizeof(cmd_name_strings[0]);

static void call_cmd_from_index( BaseSequentialStream *chp, 
                    int argc, char *argv[], uint8_t cmd_index);

static const char invalid_num_str[] = "%s is not a valid number!\n\r";
static const char invalid_arg_str[] = "%s is an invalid argument!\n\r";

static const char loop_usage[] = "loop usage: loop <command> "
                    "[ -h | -l [<loop_times> | -i ]]\n\r"
                    "\t-h: prints help\n\r"
                    "\t-l <loop_times> : loops the amount of times\n\r"
                    "\t-l -i : loops infinitely\n\r"
                    "looping can be halted by pressing CTRL+C or 'q' \n\r";


static void cmd_led(BaseSequentialStream *chp, int argc, char *argv[]) {
    const char led_usage[] = "usage: led <led_state>\n\r\tled_state:"
                                        " 0=OFF 1=ON t=TOGGLE\n\r";
    const char led_toggle_text[] = "led TOGGLE\n\r";
    if((argc > 0) && (argc < 2)){
        if(argv[0][0] == '1'){
            palSetPad(CMD_LED_GPIO,CMD_LED_NUM);
            chprintf(chp, "led ON\n\r");
        } else if(argv[0][0] == '0'){
            palClearPad(CMD_LED_GPIO,CMD_LED_NUM);
            chprintf(chp, "led OFF\n\r");
        } else if(argv[0][0] == 't'){
            palTogglePad(CMD_LED_GPIO,CMD_LED_NUM);
            chprintf(chp, led_toggle_text);
        } else if(strcmp("-h",argv[0])==0){
            chprintf(chp, led_usage);
        }else {
            chprintf(chp, "%c is not a valid value for led!\n\r",argv[1][0]);
            chprintf(chp, led_usage);
        }
    } else if(argc == 0){
        palTogglePad(CMD_LED_GPIO,CMD_LED_NUM);
        chprintf(chp, led_toggle_text);
    }else {
        if(argc >= 2){chprintf(chp, "too many arguments!\n\r");}
        //chprintf(chp, led_usage);
    }
}
static void cmd_loop(BaseSequentialStream *chp, int argc, char *argv[]) {
    uint32_t loop_times=1;
    uint32_t loop_delay_ms=100;
    uint8_t cmd_match=0;
    uint8_t is_infinite=0;
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
                uint8_t is_valid_num = str_is_valid_num(argv[2]);
                is_infinite = (strcmp("-i",argv[2])==0);
                if(is_valid_num || is_infinite){ 
                    if(is_valid_num){loop_times = atol(argv[2]);}
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
    chprintf(chp,"looping %s command %U times with delay of %U milliseconds:\n\r"
            "looping can be halted by pressing CTRL+C or 'q' during operation\n\r", 
        argv[0],loop_times,loop_delay_ms);
    char c;
    while(loop_times--){
        call_cmd_from_index(chp,pass_argc,pass_argv,cmd_i);        
        chThdSleepMilliseconds(loop_delay_ms);
        if(sdReadTimeout((SerialDriver *)chp,(uint8_t *)&c,1,TIME_IMMEDIATE)!=0){
            //checks if 'q' or CTRL+C (ascii dec 3) have been input
          if((c==3)||(c=='q')){break;} 
        }
        if(is_infinite){loop_times++;}
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

static void call_cmd_from_index( BaseSequentialStream *chp,
                 int argc, char *argv[], uint8_t cmd_index){
    switch (cmd_index){
    case CMD_LED:
        cmd_led(chp, argc, argv);
        break;
    case CMD_LOOP:
        cmd_loop(chp, argc, argv);
        break;
    }
}

static const ShellCommand shCmds[] = {
    {CMD_STR_LED,cmd_led},   
    {CMD_STR_LOOP,cmd_loop},
    {NULL, NULL}
};
static const ShellConfig shCfg = {
(BaseSequentialStream *)&SD1,
shCmds
};

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

/* config for the serial stuff */
static SerialConfig sd1conf = {
38400, /* baud */
0,0,0,
};

int main(void){
    
    halInit();
    chSysInit();

    Thread *sh = NULL;
    sdStart(&SD1, &sd1conf);

    /*
     * Initializes the SPI driver 1.
     */
    spiStart(&SPID1, &spicfg);
    

    chThdCreateStatic(waBlinker1, sizeof(waBlinker1), NORMALPRIO, Blinker1, NULL);
    chThdCreateStatic(waSegThread1, sizeof(waSegThread1), NORMALPRIO+2, SegThread1, NULL);
    //chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    //chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO+1, PWMThread2, NULL);    
    
    init_digit_pins();
    init_SPI1();

    palSetPadMode(CMD_LED_GPIO, CMD_LED_NUM, PAL_MODE_OUTPUT_PUSHPULL);
    palSetPadMode(BLINKER_LED_GPIO, BLINKER_LED_NUM, PAL_MODE_OUTPUT_PUSHPULL);
//TestThread(&SD1);
    shellInit();    

    while (TRUE){
        if (!sh){
            chprintf((BaseSequentialStream *)&SD1,"Starting ChibiOS/RT Shell\n\r");
            sh = shellCreate(&shCfg, SHELL_WA_SIZE, NORMALPRIO+1);
        } else if (chThdTerminated(sh)) {
            chThdRelease(sh);
            sh = NULL;
        }
        //palSetPad(GPIOC, 8);
        //TestThread(&SD1);
        //chThdSleepMilliseconds(500);
        //palClearPad(GPIOC, 8);
        //chThdSleepMilliseconds(500);
    }
}
