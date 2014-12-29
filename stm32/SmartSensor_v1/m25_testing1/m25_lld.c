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

/**
 * @file    templates/m25_lld.c
 * @brief   M25 Driver subsystem low level driver source template.
 *
 * @addtogroup M25
 * @{
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

/**
 * @brief   M251 driver identifier.
 */
#if PLATFORM_M25_USE_M251 || defined(__DOXYGEN__)
M25Driver M25D1;
#endif

/*===========================================================================*/
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level M25 driver initialization.
 *
 * @notapi
 */
void m25_lld_init(void) {

#if PLATFORM_M25_USE_M251
  /* Driver initialization.*/
  m25ObjectInit(&M25D1);
#endif /* PLATFORM_M25_USE_M251 */
}

/**
 * @brief   Configures and activates the M25 peripheral.
 *
 * @param[in] m25p      pointer to the @p M25Driver object
 *
 * @notapi
 */
void m25_lld_start(M25Driver *m25p) {

  if (m25p->state == M25_STOP) {
    /* Enables the peripheral.*/
#if PLATFORM_M25_USE_M251
    if (&M25D1 == m25p) {

    }
#endif /* PLATFORM_M25_USE_M251 */
  }
  /* Configures the peripheral.*/

}

/**
 * @brief   Deactivates the M25 peripheral.
 *
 * @param[in] m25p      pointer to the @p M25Driver object
 *
 * @notapi
 */
void m25_lld_stop(M25Driver *m25p) {

  if (m25p->state == M25_READY) {
    /* Resets the peripheral.*/

    /* Disables the peripheral.*/
#if PLATFORM_M25_USE_M251
    if (&M25D1 == m25p) {

    }
#endif /* PLATFORM_M25_USE_M251 */
  }
}

//#endif /* HAL_USE_M25 */

/** @} */
