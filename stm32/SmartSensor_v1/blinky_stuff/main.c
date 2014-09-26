#include "ch.h"
#include "hal.h"

int main(void){
    
    halInit();
    chSysInit();

    palSetPadMode(GPIOA, 3, PAL_MODE_OUTPUT_PUSHPULL);

    while (TRUE){
        palSetPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
        palClearPad(GPIOA, 3);
        chThdSleepMilliseconds(500);
    }
}
