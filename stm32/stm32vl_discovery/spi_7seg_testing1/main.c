#include "ch.h"
#include "hal.h"
#include "test.h"
#include "seven_segs.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 

#define DIG_SWITCH_DELAY_MS 4
#define DIG_SWITCH_DELAY_US 250

static inline void init_SPI1(void);
//void init_digit_pins(void);
void write_SPI1(uint8_t out_byte);
void msg_error(void);

void write_number(int16_t number);
void write_digit(int8_t num, uint8_t dig);
volatile  bool_t SPI_done = FALSE;

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
static void spicb(SPIDriver *spip);

/*
 * SPI configuration structure.
 * speed clk/div64, CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOA_SPI1NSS on the port GPIOA.
 */
static const SPIConfig spicfg = {
  spicb,
  /* HW dependent part.*/
  GPIOA,
  GPIOA_SPI1NSS,
  SPI_CR1_BR_0 | SPI_CR1_BR_2 | SPI_CR1_MSTR,//cr1 set baud to clk/div64
};

/*
 * SPI end transfer callback.
 */
static void spicb(SPIDriver *spip) {
 
    if(spip->state == SPI_COMPLETE){
    /* On transfer end just releases the slave select line.*/
    chSysLockFromIsr();
    SPI_done = TRUE;
    spiUnselectI(spip);
//    palSetPad(GPIOC,4);
    chSysUnlockFromIsr();
    }
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
        write_number(1234);
    }
}

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
 
    /*
     * Initializes the SPI driver 1.
     */
    spiStart(&SPID1, &spicfg);
    

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    chThdCreateStatic(waSegThread1, sizeof(waSegThread1), NORMALPRIO+2, SegThread1, NULL);
    //chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    //chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO+1, PWMThread2, NULL);    
    
    init_digit_pins();
    init_SPI1();

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
