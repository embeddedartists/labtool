/*!
 * @file
 * @brief     Controls available LEDs
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
#ifndef __LED_H
#define __LED_H

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! \name LED directly connected to a GPIO
 * \{
 */
#define LED_FAST_ON()      LPC_GPIO_PORT->SET[0]=(1UL<<8)
#define LED_FAST_OFF()     LPC_GPIO_PORT->CLR[0]=(1UL<<8)
#define LED_FAST_TOGGLE()  LPC_GPIO_PORT->NOT[0]=(1UL<<8)
/* \} */

/*! \name LED directly connected to a GPIO, but also to SSEL_E2PROM
 * \{
 */
#define LED_SSEL_ON()      /*do {} while(0)*/ LPC_GPIO_PORT->SET[5]=(1UL<<9)
#define LED_SSEL_OFF()     /*do {} while(0)*/ LPC_GPIO_PORT->CLR[5]=(1UL<<9)
#define LED_SSEL_TOGGLE()  /*do {} while(0)*/ LPC_GPIO_PORT->NOT[5]=(1UL<<9)
/* \} */

/*! \name Aliases for the LEDs
 * \{
 */
#define LED_TRIG_ON()     do {} while(0)
#define LED_TRIG_OFF()    do {} while(0)
#define LED_ARM_ON()      do {} while(0)
#define LED_ARM_OFF()     do {} while(0)
#define LED_SPARE1_ON()   do {} while(0)
#define LED_SPARE1_OFF()  do {} while(0)
#define LED_SPARE2_ON()   do {} while(0)
#define LED_SPARE2_OFF()  do {} while(0)
/* \} */

/*! \name Alias for the USB LED
 * \{
 */
#define LED_USB_CONNECTED_ON()  LED_SPARE2_ON()
#define LED_USB_CONNECTED_OFF() LED_SPARE2_OFF()
/* \} */


/******************************************************************************
 * Global Variables
 *****************************************************************************/

#endif /* end __LED_H */

