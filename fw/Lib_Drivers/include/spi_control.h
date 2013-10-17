/*!
 * @file
 * @brief     Driver for the NXP 74HC595PW Shift Register
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
#ifndef __SPI_CONTROL_H
#define __SPI_CONTROL_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! \name Connections on the 74HC595PW chip
 * \{
 */
#define CTRL_CH1_GN0      (1<<0)
#define CTRL_CH1_GN1      (1<<1)
#define CTRL_CH1_GN2      (1<<2)
#define CTRL_CH1_AC_DC    (1<<3)
#define CTRL_CH2_GN0      (1<<4)
#define CTRL_CH2_GN1      (1<<5)
#define CTRL_CH2_GN2      (1<<6)
#define CTRL_CH2_AC_DC    (1<<7)
/* \} */

/*! Default output values. */
#define DEFAULT_VALUES     (CTRL_CH1_GN0 | CTRL_CH1_GN1 | CTRL_CH1_GN2 | CTRL_CH1_AC_DC \
                            | CTRL_CH2_GN0 | CTRL_CH2_GN1 | CTRL_CH2_GN2 | CTRL_CH2_AC_DC)

/******************************************************************************
 * Functions
 *****************************************************************************/

void spi_control_init(void);
void spi_control_write(uint8_t data, uint8_t mask);

#endif /* end __SPI_CONTROL_H */
