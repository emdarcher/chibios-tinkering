#include "ch.h"
#include "hal.h"
#include "test.h"
#include "seven_segs.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 

#define DIG_SWITCH_DELAY_MS 10

void init_SPI1(void);
//void init_digit_pins(void);
void write_SPI1(uint8_t out_byte);
void msg_error(void);

void write_number(int16_t number);
void write_digit(int8_t num, uint8_t dig);


static inline void init_digit_pins(void){
    palSetGroupMode(GPIOC, ALL_DIGS, 0,PAL_MODE_OUTPUT_PUSHPULL );
}

/* spi callback proto */
static void spicb(SPIDriver *spip);

/*
 * SPI configuration structure.
 * Maximum speed (12MHz), CPHA=0, CPOL=0, 8bits frames, MSb transmitted first.
 * The slave select line is the pin GPIOA_SPI1NSS on the port GPIOA.
 */
static const SPIConfig spicfg = {
  spicb,
  /* HW dependent part.*/
  GPIOA,
  GPIOA_SPI1NSS,
  0,//cr1
};

/*
 * SPI end transfer callback.
 */
static void spicb(SPIDriver *spip) {
 
  /* On transfer end just releases the slave select line.*/
  chSysLockFromIsr();
  spiUnselectI(spip);
  chSysUnlockFromIsr();
}

void write_digit(int8_t num, uint8_t dig){
     
    uint8_t out_bytes[1] = {
        (((num<10)&&(num>=0)) ? number_seg_bytes[num] : number_seg_bytes[10]),
    };

    chSysLockFromIsr();
    /* SPI slave selection and transmission start.*/
    spiSelectI(&SPID1);
    spiStartSendI(&SPID1, 1, out_bytes);

    chSysUnlockFromIsr();
    uint8_t nd = NUM_DIGS;
    while(nd--){
        if(nd == dig){
            palSetPad(GPIOC,nd); 
        } else {
            palClearPad(GPIOC,nd);
        }
    }

    chThdSleepMilliseconds(DIG_SWITCH_DELAY_MS); 
}


static WORKING_AREA(waSegThread1,32);
static __attribute__((noreturn)) msg_t SegThread1(void *arg){
    (void)arg;
    chRegSetThreadName("seg thread");
    while(TRUE){
        write_digit(1,0);
        write_digit(2,1);
        write_digit(3,2);
        write_digit(4,3);
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
    chThdCreateStatic(waSegThread1, sizeof(waSegThread1), NORMALPRIO, SegThread1, NULL);
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
