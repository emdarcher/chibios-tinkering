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
 * @file    templates/m25_lld.h
 * @brief   M25 Driver subsystem low level driver header template.
 *
 * @addtogroup M25
 * @{
 */

#ifndef _M25_LLD_H_
#define _M25_LLD_H_

#if HAL_USE_M25 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   M25 driver enable switch.
 * @details If set to @p TRUE the support for M251 is included.
 */
#if !defined(PLATFORM_M25_USE_M251) || defined(__DOXYGEN__)
#define PLATFORM_M25_USE_M251             FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an M25 driver.
 */
typedef struct M25Driver M25Driver;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {

} M25Config;

/**
 * @brief   Structure representing an M25 driver.
 */
struct M25Driver {
  /**
   * @brief Driver state.
   */
  m25state_t                state;
  /**
   * @brief Current configuration data.
   */
  const M25Config           *config;
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if PLATFORM_M25_USE_M251 && !defined(__DOXYGEN__)
extern M25Driver M25D1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void m25_lld_init(void);
  void m25_lld_start(M25Driver *m25p);
  void m25_lld_stop(M25Driver *m25p);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_M25 */

#endif /* _M25_LLD_H_ */

/** @} */
