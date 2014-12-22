#include "ch.h"
#include "hal.h"
//#include "test.h"

#define MY_ABS(a) (((a)>=0) ? (a) : (0L-(a)))  

#define DIG_TO_CHAR(d) ((char)((d) + 48)) 


/* adc callback functions */
static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n);
/* adc defines: */
/* Total number of channels to be sampled by a single ADC operation.*/
#define ADC_GRP1_NUM_CHANNELS   3
/* Depth of the conversion buffer, channels are sampled this many times each.*/
#define ADC_GRP1_BUF_DEPTH      8
/* adc sample buffer */
static adcsample_t samples[ADC_GRP1_NUM_CHANNELS * ADC_GRP1_BUF_DEPTH];
static adcsample_t adc_avg_in0=0;
static adcsample_t adc_avg_sensor=0;
static adcsample_t adc_avg_STLM20=0;
/* adc config stuff: */
static const ADCConversionGroup adcgrpcfg = {
    FALSE, //linear buffer
    ADC_GRP1_NUM_CHANNELS, //num channels
    adccb, //callback
    NULL, //no error callback
    /* HW stuff / regs */
    0, //cr1
    ADC_CR2_TSVREFE, //cr2 enable temp sensor vref
    ADC_SMPR1_SMP_SENSOR(ADC_SAMPLE_239P5),//smpr1 sensor sample 239.5 cycles
    //smpr2 set analog 0 to 41.5 cycles sample, STLM20 temp Vout PB1 analog 9 239.5 cycles
    ADC_SMPR2_SMP_AN0(ADC_SAMPLE_41P5) |
    ADC_SMPR2_SMP_AN9(ADC_SAMPLE_239P5),
    ADC_SQR1_NUM_CH(ADC_GRP1_NUM_CHANNELS), //sqr1 number of channels
    0, //sqr2
    //sqr3 channel in0 is 1st in sequence, sensor channel is 2nd in sequence
    //channel in9 is 3rd in sequence,
    ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0) | ADC_SQR3_SQ2_N(ADC_CHANNEL_SENSOR) |
    ADC_SQR3_SQ3_N(ADC_CHANNEL_IN9),
};
/* gets the average of a channel from a sample array */
static  adcsample_t get_avg_adcgrp_ch(
        uint8_t channel, 
        adcsample_t *in_samples, 
        uint8_t num_channels, 
        uint8_t buf_depth){
    
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

//#define MY_VDDA_MV 3300U //Vdda in millivolts

/* change the reference voltage definition here for the situation */
//#define MY_VDDA_UV 3300000UL //Vdda in microvolts
//#define MY_VDDA_UV 3000000UL //Vdda in microvolts
#define MY_VDDA_UV 2950000UL //Vdda in microvolts

#define MY_UV_PER_ADC12 (MY_VDDA_UV >> 12) //amount of microvolts per adc12 tick
#define MY_ADC12_TO_UV(a) (a * MY_UV_PER_ADC12) //gets microvolts from adc12 val

/* function to get microvolts from an ADC reading */
static uint32_t adc12_to_uV(adcsample_t adc12_in, uint32_t ref_uV){
    return (adc12_in * (ref_uV >> 12));
}
/* function to get millivolts from an ADC reading */
static uint16_t adc12_to_mV(adcsample_t adc12_in, uint32_t ref_uV){
    return (uint16_t)(adc12_to_uV(adc12_in,ref_uV) / 1000U);
}
/* function to calculate temp in C using linear function
found in the datasheet of STLM20 analog temp sensor */
/* for range -10 to 65 degrees Celsius
-11.71mV/C * T + 1.8641V = Vout
Vout = 1.8641V - (0.01171V * T)
T = (1.8641V - Vout)/(0.01171V)
T = (1864.1mV - mVout)/(11.71mV)
T = (1864100uV - uVout)/(11710uV) 
T (in 100ths of a degree) = ((int32_t
*/
static int32_t uV_to_centiDegC(uint32_t in_uV){
    int32_t uVout = (int32_t)in_uV;
    int32_t temp_calc = (int32_t)(((1864100L - uVout)*100L)/11710L);
    return temp_calc;
}
/* degF = degC * 9/5 + 32
cF = ((cC * 9) + (3200*5))/5
cF = ((cC * 9) + 16000)/5
*/
static int32_t centiDegC_to_centiDegF(int32_t cdegC){
    return (int32_t)(((cdegC * 9L)+16000L)/5L);
}

static char * centiDeg_to_str(int32_t cdeg){
    static char out[7]="ABCD.EF";
    static uint8_t out_digs[5];
    uint32_t val=(uint32_t)cdeg;
    out[0] = (cdeg<0) ? '-' : '+';
    if(( val > 9999 )){
        out_digs[0] = val / 10000UL ;
        val -= out_digs[0] * 10000UL;
        out[1]= DIG_TO_CHAR(out_digs[0]);
    }else{out[1]=' ';}
    out_digs[1] = val / 1000UL;
    val -= out_digs[1] * 1000UL;
    out[2] = DIG_TO_CHAR(out_digs[1]);
    out_digs[2] = val / 100UL;
    val -= out_digs[2] * 100UL;
    out[3] = DIG_TO_CHAR(out_digs[2]);
    out_digs[3] = val / 10UL;
    val -= out_digs[3] * 10UL;
    out[5] = DIG_TO_CHAR(out_digs[3]);
    out_digs[4] = val;
    out[6] = DIG_TO_CHAR(out_digs[4]);
    return (char *)out;
}
static char * fourDig_to_str(uint16_t fourDig){
    static char out_str[4]="    ";
    static uint8_t digs[4];
    uint16_t val = fourDig;
    digs[0] = val / 1000U;
    val -= digs[0] * 1000U;
    digs[1] = val / 100U;
    val -= digs[1] * 100U;
    digs[2] = val / 10U;
    val -= digs[2] * 10U;
    digs[3] = val;
    uint8_t i=4;
    while(i--){
        out_str[i] = DIG_TO_CHAR(digs[i]);
    } 
    return (char *)out_str;
}

/* adc callback complete function */
static void adccb(ADCDriver *adcp, adcsample_t *buffer, size_t n){
    (void)buffer;(void)n;
    /* check for ADC_COMPLETE before continuing */
    if (adcp->state == ADC_COMPLETE) {
        adcsample_t avg_in0, avg_sensor, avg_STLM20;

        /* calculate avg of samples */
        avg_in0 = get_avg_adcgrp_ch( 0, samples, ADC_GRP1_NUM_CHANNELS, ADC_GRP1_BUF_DEPTH);
        adc_avg_in0 = avg_in0;
        avg_sensor = get_avg_adcgrp_ch( 1, samples, ADC_GRP1_NUM_CHANNELS, ADC_GRP1_BUF_DEPTH);
        adc_avg_sensor = avg_sensor;
        avg_STLM20 = get_avg_adcgrp_ch( 2, samples,  ADC_GRP1_NUM_CHANNELS, ADC_GRP1_BUF_DEPTH);
        adc_avg_STLM20 = avg_STLM20;
    }
}


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
9600, /* baud */
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
        //adcsample_t val= adc_avg_in0;
        uint16_t val = adc12_to_mV(adc_avg_in0,MY_VDDA_UV);
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
        char * cDegStr = centiDeg_to_str(uV_to_centiDegC(adc12_to_uV(adc_avg_STLM20,MY_VDDA_UV)));
        sdWrite(&SD1,(uint8_t *)"PA0 input: ",11);
        sdWrite(&SD1,(uint8_t *) readout_str, sizeof(readout_str));
        sdWrite(&SD1,(uint8_t *)"mV ",3);
        sdWrite(&SD1,(uint8_t *)"STLM20 Vout: ",13);
        sdWrite(&SD1,(uint8_t *)fourDig_to_str(adc12_to_mV(adc_avg_STLM20,MY_VDDA_UV)),4);
        sdWrite(&SD1,(uint8_t *)"mV ",3);
        sdWrite(&SD1,(uint8_t *) cDegStr, 7);
        sdWrite(&SD1,(uint8_t *)" DegC",5);
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

    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    //chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO+2, Thread2, NULL);
    chThdCreateStatic(waPWMThread2, sizeof(waPWMThread2), NORMALPRIO+1, PWMThread2, NULL);    
    chThdCreateStatic(waADCreadout, sizeof(waADCreadout), NORMALPRIO+2, ADCreadout, NULL);    

    palSetPadMode(GPIOA, GPIOA_PA0, PAL_MODE_INPUT_ANALOG);
    palSetPadMode(GPIOB, GPIOB_PB1, PAL_MODE_INPUT_ANALOG);
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
