#include "ch.h"
#include "hal.h"
//#include "test.h"

/* adc callback functions */
static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n);
/* adc defines: */
/* Total number of channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   1
/* Depth of the conversion buffer, channels are sampled four times each.*/
#define ADC_GRP1_BUF_DEPTH      4
/* adc sample buffer */
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t adc_avg_in0=0;
/* adc config stuff: */
static const ADCConversionGroup adcgrpcfg = {
    FALSE, //linear buffer
    ADC_GRP1_NUM_CHANNELS, //num channels
    adccb, //callback
    NULL, //no error callback
    /* HW stuff / regs */
    0, 0, //cr1 and cr2
    0, //smpr1
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_41P5), //smpr2 set analog 0 to 41.5 cycle sample
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS), //sqr1 number of channels
    0, //sqr2
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0),//sqr3 channel in0 is 1st in sequence
};
static  adcsample_t get_avg_adcgrp_ch(uint8_t channel, adcsample_t *in_samples, uint8_t num_channels, uint8_t buf_depth){
    adcsample_t out_avg=0;
    uint16_t index=0;
    int i;
    for(i=0;i<buf_depth;i++){
        out_avg += in_samples[(channel + index)];
        index += num_channels;
    }
    out_avg /= buf_depth;
    return out_avg;
}
/* adc callback complete function */
static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n){
    (void)buffer;(void)n;
    /* check for ADC_COMPLETE before continuing */
    if (adcp->state == ADC_COMPLETE) {
        adcsample_t avg_in0;

        /* calculate avg of samples */
        avg_in0 = get_avg_adcgrp_ch( 0, samples, ADC_GRP1_NUM_CHANNELS, ADC_GRP1_BUF_DEPTH);
        adc_avg_in0 = avg_in0;
    }
}

#define DIG_TO_CHAR(d) ((char)(d + 48)) 

/* pwm callback funtions: */
static void pwm2pcb(PWMDriver *pwmp){
    (void)pwmp;
    palSetPad(GPIOA, GPIOA_PA1);
}
static void pwm2c2cb(PWMDriver *pwmp){
    (void)pwmp;
    palClearPad(GPIOA, GPIOA_PA1);
}

/* config for pwm stuff */
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

/* config for the serial stuff */
static SerialConfig sd1conf = {
9600,
0,
0,
0,
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
#if 0
static WORKING_AREA(waThread2, 32);
static __attribute__((noreturn))  msg_t Thread2(void *arg) {

    (void)arg;
    chRegSetThreadName("messenger");
    static char hello_data[] = "hello\n\r";
    while(TRUE){
        sdWrite(&SD1,(uint8_t *) hello_data, sizeof(hello_data));
        chThdSleepMilliseconds(1000);
    }

}
#endif

static const char newline[2] = "\n\r";

static WORKING_AREA(waADCreadout, 128);
static __attribute__((noreturn)) msg_t ADCreadout(void *arg){
    (void)arg;
    chRegSetThreadName("ADCreadout serial");
    static char readout_str[4]; //only goes up to 4096
    static uint8_t readout_digs[4];
    while(TRUE){
        
        chSysLockFromIsr();
        adcStartConversionI(&ADCD1, &adcgrpcfg, samples, ADC_GRP1_BUF_DEPTH);
        chSysUnlockFromIsr();
        //convert to string the adc val
        adcsample_t val= adc_avg_in0;  
        readout_digs[0] = val / 1000U;
        val -= readout_digs[0] * 1000U;
        readout_digs[1] = val / 100U;
        val -= readout_digs[1] * 100U;
        readout_digs[2] = val / 10U;
        val -= readout_digs[2] * 10U;
        readout_digs[3] = val;
        uint8_t i=4;
        while(i--){
            readout_str[i] = DIG_TO_CHAR(readout_digs[i]);
        } 
        sdWrite(&SD1,(uint8_t *) readout_str, sizeof(readout_str));
        sdWrite(&SD1,(uint8_t *) newline,2);
        chThdSleepMilliseconds(100);
    }
}


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


int main(void){
    
    halInit();
    chSysInit();
    
    sdStart(&SD1,&sd1conf);
    adcStart(&ADCD1, NULL);

    /* start PWM on TIM2 with our config */

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);
    //chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO, PWMThread2, NULL);    
    chThdCreateStatic(waADCreadout, sizeof(waADCreadout), NORMALPRIO-1, ADCreadout, NULL);    

    palSetPadMode(GPIOA, GPIOA_PA0, PAL_MODE_INPUT_ANALOG);
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
