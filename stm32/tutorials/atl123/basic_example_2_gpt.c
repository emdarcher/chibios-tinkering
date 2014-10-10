/*
 *
 * Chibios Example program #2 GPT
 *
 */

// standard 'C' include files
#include <stdio.h>   
#include <string.h>
#include <stdlib.h>

// chibios specific include files
#include "ch.h"
#include "hal.h"

/*
 * GREEN blinker thread, times are in milliseconds.
 *
 * This function will become a thread that will execute independent
 *  of the other threads.  We will call this thread 2
 */
 
// This macro creates the working space and stack space for thread 2 
static WORKING_AREA(waThread2, 128); 

// This function will become the code for thread 2
static msg_t Thread2(void *arg)
{
    (void)arg;
    chRegSetThreadName("Green Blinker");
    while (TRUE)
    {
        palClearPad(GPIOD, GPIOD_LED4);     // PD12 - Green LED
        chThdSleepMilliseconds( 200 );             // delay 200mS
        palSetPad(GPIOD, GPIOD_LED4);       // PD12 - Green LED
        chThdSleepMilliseconds( 20 );             // delay 20mS
    }
    return 0;
}

/*
 * RED blinker thread, times are in milliseconds.
 *
 * This function will become a thread that will execute independent
 *  of the other threads.  We will call this thread 3
 *
 */

// This macro creates the working space and stack space for thread 3
static WORKING_AREA(waThread3, 128);

// This function will become the code for thread 3
static msg_t Thread3(void *arg)
{
    (void)arg;
    chRegSetThreadName("Red Blinker");
    while (TRUE)
    {
        palSetPad(GPIOD, GPIOD_LED5);         // PD14 - Red LED
        chThdSleepMilliseconds( 100 );            // delay 100mS
        palClearPad(GPIOD, GPIOD_LED5);      // PD14 - Red LED
        chThdSleepMilliseconds( 100 );            // delay 100mS
    }
    return 0;
}

/*
 * GPT2 callback.
 *      Every time the timer fires send a new sample to the output port
 */
static void sample_output_callback(GPTDriver *gptp)
{
    /* counter to waste some time...  */
    uint32_t delay_count;

    /* define the pattern index, dataset and datasize size */
    static uint16_t pattern_index = 0;
    static uint8_t pattern_data[] =
    {
         0x00,0x00,0x00,0x00,0x00,0x3a,0x00,0x00,
         0x30,0x00,0x30,0x00,0x3e,0x14,0x3e,0x14,
         0x1a,0x3b,0x16,0x00,0x24,0x08,0x12,0x00,
         0x14,0x2a,0x14,0x02,0x00,0x30,0x00,0x00,
         0x00,0x1e,0x21,0x00,0x21,0x1e,0x00,0x00,
         0x2a,0x1c,0x2a,0x00,0x08,0x3e,0x08,0x00,
         0x01,0x02,0x00,0x00,0x08,0x08,0x08,0x00,
         0x00,0x02,0x00,0x00,0x06,0x08,0x30,0x00,
         0x1c,0x2a,0x1c,0x00,0x12,0x3e,0x02,0x00,
    };
#define PATTERN_SIZE sizeof( pattern_data )

    /* define the output port -- yea, yea...  this is just an example.... */
    volatile uint8_t port;
    volatile uint8_t port2;


    /* turn on the LED to give us an indication of when this code is running */
    /*   you will need a logic sniffer or scope to see the pulses */
    palSetPad(GPIOD, GPIOD_LED6);   // Turn off the Blue LED

    port = pattern_data[ pattern_index ];

    pattern_index++;
    if( pattern_index > PATTERN_SIZE )
    {
        pattern_index = 0;
    }

    /* waste some time, otherwise this callback will run so quick */
    /*   you won't be able to see it.  We have to do something    */
    /*   or our loop could be optimized out */
    for( delay_count = 0; delay_count < 100000; delay_count++ )
    {
        port2 = (uint8_t)delay_count % 256;
    }

    /* turn off the port to indicate how long this code took to execute */
    /*   yep, still need that logic sniffer to see it */
    palClearPad(GPIOD, GPIOD_LED6);  // Turn off the Blue LED
  }

/*
 * GPT2 configuration.
 *   This configuration block defines a time with a 200kHz counting clock,
 *   and our callback function named sample_output_callback.  When we start
 *   the timer we will specify the numbers of clock ticks we want to elapse
 *   between callback execution.
 *
 *   NOTE:  Be sure the execution of the callback is done before calling it
 *            again.  Otherwise, strange things may happen....
 *          Generally, you will want to define the callback function above
 *            this or the compiler will complain about it being undefined.
 */
static GPTConfig gpt2cfg =
{
    200000,                    /* timer clock.*/
    sample_output_callback     /* Timer callback.*/
};

/*
 * Application entry point.
 */
// main() function will become thread 1

// we do not need to allocate working space and stack space for this function,
//  as this is handled by the startup code for us.
int main(void) 
{
    /*
     * System initializations.
     * - HAL initialization, this also initializes the configured device drivers
     *   and performs the board-specific initializations.
     * - Kernel initialization, the main() function becomes a thread and the
     *   RTOS is active.
     */
    halInit();
    chSysInit();

    /*
     * Creates the blinker threads.
     */
    chThdCreateStatic(waThread2, sizeof(waThread2), NORMALPRIO, Thread2, NULL);
    chThdCreateStatic(waThread3, sizeof(waThread3), NORMALPRIO, Thread3, NULL);


    // configure the GPT timer
    gptStart(&GPTD2, &gpt2cfg);

    // start the GPT in continuous mode.  dT is the time between triggers
    //   Here, we have set the timer clock to 200,000Hz, and we want
    //   to call the callback function every 25 GPT clock cycles.  This
    //   means we call the callback function every 125uS or 8,000 time
    //   per second
    gptStartContinuous(&GPTD2, 25); // dT = 200,000 / 25 = 8,000Hz

    /*
     * Main loop, flash Orange LED
     */

    while (TRUE)
    {
        palSetPad(GPIOD, GPIOD_LED3);       // PD13 - Orange LED
        chThdSleepMilliseconds( 750 );      // delay 750mS
        palClearPad(GPIOD, GPIOD_LED3);     // PD13 - Orange LED
        chThdSleepMilliseconds( 750 );      // delay 750mS
    }
  }

