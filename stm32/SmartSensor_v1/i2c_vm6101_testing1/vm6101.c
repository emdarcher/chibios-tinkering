

#include <stdlib.h>

#include "ch.h"
#include "hal.h"
#include "vm6101.h"

static uint8_t color_tx_data[COLOR_TX_DEPTH];
static uint8_t color_rx_data[COLOR_RX_DEPTH];

static i2cflags_t errors = 0;



