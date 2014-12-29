
/**
 * @file    m25.h
 * @brief   M25 flash Driver macros and structures.
 *
 * @author Ethan Durrant 2014
 * 
 */

#ifndef _M25_H_
#define _M25_H_

//#if HAL_USE_M25 || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  M25_UNINIT = 0,                   /**< Not initialized.                   */
  M25_STOP = 1,                     /**< Stopped.                           */
  M25_READY = 2,                    /**< Ready.                             */
} m25state_t;

/**
 * @brief   Type of a structure representing a M25 driver.
 */
typedef struct M25Driver M25Driver;

#include "m25_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void m25Init(void);
  void m25ObjectInit(M25Driver *m25p);
  void m25Start(M25Driver *m25p, const M25Config *config);
  void m25Stop(M25Driver *m25p);
#ifdef __cplusplus
}
#endif

//#endif /* HAL_USE_M25 */

#endif /* _M25_H_ */

/** @} */
