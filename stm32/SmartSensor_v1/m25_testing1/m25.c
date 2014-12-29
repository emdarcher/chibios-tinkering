
/**
 * @file    m25.c
 * @brief   M25 flash Driver code.
 *
 * @author Ethan Durrant 2014
 * 
 */

#include "ch.h"
#include "hal.h"

//#if HAL_USE_M25 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   M25 Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void M25Init(void) {

  m25_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p M25Driver structure.
 
 * @param[out] m25p     pointer to the @p M25Driver object
 *
 * @init
 */
void m25ObjectInit(M25Driver *m25p) {

  m25p->state  = M25_STOP;
  m25p->config = NULL;
}

/**
 * @brief   Configures and activates the M25 peripheral.
 *
 * @param[in] m25p      pointer to the @p M25Driver object
 * @param[in] config    pointer to the @p M25Config object
 *
 * @api
 */
void m25Start(M25Driver *m25p, const M25Config *config) {

  chDbgCheck((m25p != NULL) && (config != NULL), "m25Start");

  chSysLock();
  chDbgAssert((m25p->state == M25_STOP) || (m25p->state == M25_READY),
              "m25Start(), #1", "invalid state");
  m25p->config = config;
  m25_lld_start(m25p);
  m25p->state = M25_READY;
  chSysUnlock();
}

/**
 * @brief   Deactivates the M25 peripheral.
 *
 * @param[in] m25p      pointer to the @p M25Driver object
 *
 * @api
 */
void m25Stop(M25Driver *m25p) {

  chDbgCheck(m25p != NULL, "m25Stop");

  chSysLock();
  chDbgAssert((m25p->state == M25_STOP) || (m25p->state == M25_READY),
              "m25Stop(), #1", "invalid state");
  m25_lld_stop(m25p);
  m25p->state = M25_STOP;
  chSysUnlock();
}

//#endif /* HAL_USE_M25 */

/** @} */
