/*
    ChibiOS/RT - Copyright (C) 2006-2013 Giovanni Di Sirio

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/
/*
 * NOTE: this is standalone app for miniecu board.
 */

#include "ch.h"
#include "hal.h"
//#include "flash25.h"
#include "sst25.h"
#include "chprintf.h"

static const SPIConfig spi1_cfg = {
	NULL,
	GPIOA,
	4,//GPIOA_SPI1NSS,
	0, /* hi speed, mode0 */
	0
};

static SST25Driver FLASH25;
static const SST25Config flash_cfg = {
	.spip = &SPID1,
	.spicfg = &spi1_cfg
};

static uint8_t flash_buff[256]; /* note: for sst25 */

static void print_buff16(uint8_t buf[16])
{
	chprintf(&SD1, "buff[16]:");
	int i;
    for ( i = 0; i < 16; i++)
		chprintf(&SD1, " %02x", buf[i]);
	sdPut(&SD1, '\n\r');
}

static WORKING_AREA(wa_test, 1024);
static msg_t th_test(void *arg __attribute__((unused)))
{
	sst25ObjectInit(&FLASH25);
	sst25Start(&FLASH25, &flash_cfg);

	while (true) {
		chThdSleepMilliseconds(5000);
		chprintf(&SD1, "Connecting... ");

		if (blkConnect(&FLASH25) == CH_SUCCESS) {
			chprintf(&SD1, "OK, JDEC ID: 0x%06X: %s\n\r", sst25GetJdecID(&FLASH25), mtdGetName(&FLASH25));
			chprintf(&SD1, "Page sz: %d, erase sz: %d, pages: %d, total %d kB\n\r",
					mtdGetPageSize(&FLASH25),
					mtdGetEraseSize(&FLASH25),
					FLASH25.nr_pages,
					mtdGetSize(&FLASH25));
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}

		chThdSleepMilliseconds(500);

		chprintf(&SD1, "Reading... ");
		if (blkRead(&FLASH25, 0, flash_buff, 1) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
			print_buff16(flash_buff);
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}

		chThdSleepMilliseconds(500);

		chprintf(&SD1, "Fill pattern 0xa5\n\r");
		memset(flash_buff, 0xa5, sizeof(flash_buff));
		print_buff16(flash_buff);
		chprintf(&SD1, "Writing... ");
		if (blkWrite(&FLASH25, 0, flash_buff, 1) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}


		chThdSleepMilliseconds(500);

		memset(flash_buff, 0, sizeof(flash_buff));
		chprintf(&SD1, "Reading... ");
		if (blkRead(&FLASH25, 0, flash_buff, 1) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
			print_buff16(flash_buff);
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}

		chThdSleepMilliseconds(500);

		chprintf(&SD1, "Erasing block... ");
		/* NOTE: one erase block == 16 pages */
		if (mtdErase(&FLASH25, 0, 16) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}

		chThdSleepMilliseconds(500);

		chprintf(&SD1, "Reading... ");
		if (blkRead(&FLASH25, 0, flash_buff, 1) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
			print_buff16(flash_buff);
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}

		chThdSleepMilliseconds(500);

		chprintf(&SD1, "Erasing chip... ");
		if (mtdErase(&FLASH25, 0, UINT32_MAX) == CH_SUCCESS) {
			chprintf(&SD1, "OK\n\r");
		}
		else {
			chprintf(&SD1, "FAILED\n\r");
			continue;
		}
	};

	return 0;
}

/* config for the serial stuff */
static SerialConfig sd1conf = {
38400, /* baud */
0,0,0,
};


/*
 * Application entry point.
 */
int main(void) {

	/*
	 * System initializations.
	 * - HAL initialization, this also initializes the configured device drivers
	 *   and performs the board-specific initializations.
	 * - Kernel initialization, the main() function becomes a thread and the
	 *   RTOS is active.
	 */
	halInit();
	chSysInit();

	sdStart(&SD1, &sd1conf);
    
     palSetPadMode(GPIOA, 5, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* SCK. */
  palSetPadMode(GPIOA, 6, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* MISO.*/
  palSetPadMode(GPIOA, 7, PAL_MODE_STM32_ALTERNATE_PUSHPULL);     /* MOSI.*/
  palSetPadMode(GPIOA, 4, PAL_MODE_OUTPUT_PUSHPULL);

    
	chThdCreateStatic(wa_test, sizeof(wa_test), NORMALPRIO, th_test, NULL);

	/* we use main thread as idle */
	chThdSetPriority(IDLEPRIO);

	/* This is now the idle thread loop, you may perform here a low priority
	   task but you must never try to sleep or wait in this loop. Note that
	   this tasks runs at the lowest priority level so any instruction added
	   here will be executed after all other tasks have been started.*/
	while (true) {
	}
}
