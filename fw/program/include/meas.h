/*!
 * @file
 * @brief     Control of GPIOs for measuring
 *
 * @copyright Copyright 2013 Embedded Artists AB
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef _MEAS_H_
#define _MEAS_H_

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc43xx_scu.h"
#include "labtool_config.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! \name Internal macros, don't use directly
 * \{
 */
#if (ENABLE_MEASSURING == OPT_ENABLED)
  #define _INTERNAL_TOGGLE_MEAS_PIN(__port, __pin) (LPC_GPIO_PORT->NOT[(__port)] = (1UL << (__pin)))
  #define _INTERNAL_SET_MEAS_PIN(__port, __pin)    (LPC_GPIO_PORT->SET[(__port)] |= (1UL << (__pin)))
  #define _INTERNAL_CLR_MEAS_PIN(__port, __pin)    (LPC_GPIO_PORT->CLR[(__port)] = (1UL << (__pin)))
#else
  #define _INTERNAL_TOGGLE_MEAS_PIN(__port, __pin) do {} while(0)
  #define _INTERNAL_SET_MEAS_PIN(__port, __pin)    do {} while(0)
  #define _INTERNAL_CLR_MEAS_PIN(__port, __pin)    do {} while(0)
#endif
/* \} */

/*! \name Manipulates measurement pin 1 which is GPIO0[15] (SPI_SSEL_DAC), available on J7-6
 * \{
 */
#define TOGGLE_MEAS_PIN_1()  _INTERNAL_TOGGLE_MEAS_PIN(0, 15)
#define SET_MEAS_PIN_1()     _INTERNAL_SET_MEAS_PIN(0, 15)
#define CLR_MEAS_PIN_1()     _INTERNAL_CLR_MEAS_PIN(0, 15)
/* \} */

/*! \name Manipulates measurement pin 2 which is GPIO5[9] (SPI_SSEL_E2PROM), available on J7-12
 * \{
 */
#define TOGGLE_MEAS_PIN_2()  _INTERNAL_TOGGLE_MEAS_PIN(5, 9)
#define SET_MEAS_PIN_2()     _INTERNAL_SET_MEAS_PIN(5, 9)
#define CLR_MEAS_PIN_2()     _INTERNAL_CLR_MEAS_PIN(5, 9)
/* \} */

/*! \name Manipulates measurement pin 3 which is GPIO3[7] (SPI_SSEL_GPO), available on J7-14
 * \{
 */
#define TOGGLE_MEAS_PIN_3()  _INTERNAL_TOGGLE_MEAS_PIN(3, 7)
#define SET_MEAS_PIN_3()     _INTERNAL_SET_MEAS_PIN(3, 7)
#define CLR_MEAS_PIN_3()     _INTERNAL_CLR_MEAS_PIN(3, 7)
/* \} */

/******************************************************************************
 * Global Variables
 *****************************************************************************/

#endif /* end _MEAS_H_ */
