/*
 *
 * Chibios Example program #1 - Threads
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
 *
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
        chThdSleepMilliseconds( 20 );      // delay 200mS
        palSetPad(GPIOD, GPIOD_LED4);       // PD12 - Green LED
        chThdSleepMilliseconds( 20 );      // delay 200mS
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
        palSetPad(GPIOD, GPIOD_LED5);       // PD14 - Red LED
        chThdSleepMilliseconds( 40 );      // delay 400mS
        palClearPad(GPIOD, GPIOD_LED5);     // PD14 - Red LED
        chThdSleepMilliseconds( 40 );      // delay 400mS
    }
    return 0;
}

/*
 * Orange blinker thread, times are in milliseconds.
 *
 * This function will become a thread that will execute independent
 *  of the other threads.  We will call this thread 4
 *
 *  This function produces a 1mS pulse every Second
 */

// This macro creates the working space and stack space for thread 4
static WORKING_AREA(waThread4, 128);

// This function will become the code for thread 3
static msg_t Thread4(void *arg)
{
    (void)arg;
    chRegSetThreadName("Blue Blinker");
    while (TRUE)
    {
        palSetPad(GPIOD, GPIOD_LED6);       // PD15 - Blue LED
        chThdSleepMilliseconds( 1 );      // delay 1mS
        palClearPad(GPIOD, GPIOD_LED6);     // PD15 - Blue LED
        chThdSleepMilliseconds( 10 );      // delay 999mS
    }
    return 0;
}

/*
 * Application entry point.
 */
// main() function will become thread 1

// we do not need to allocate workiing space and stack page for this function,
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
    chThdCreateStatic(waThread4, sizeof(waThread4), NORMALPRIO, Thread4, NULL);


    /*
     * Main loop, flash Orange LED
     */

    while (TRUE)
    {
        palSetPad(GPIOD, GPIOD_LED3);       // PD13 - Orange LED
        chThdSleepMilliseconds( 75 );      // delay 750mS
        palClearPad(GPIOD, GPIOD_LED3);     // PD13 - Orange LED
        chThdSleepMilliseconds( 75 );      // delay 750mS
    }
  }

