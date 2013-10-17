/*!
 * @file
 * @brief     Driver for TI DAC102S085CIMM Dual DAC (10-bit)
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
#ifndef __SPI_DAC_H
#define __SPI_DAC_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief DAC output channel A. */
#define SPI_DAC_OUT_A  (0)

/*! @brief DAC output channel B. */
#define SPI_DAC_OUT_B  (1)

/*! @brief Keep the DAC value within the upper 10-bits of a 12 bit value */
#define SPI_DAC_LIMIT(__val) ( ((__val) >= 4095) ? 4095 : (((__val) < 4) ? 0 : (__val)) )

/*! @brief Create the 16-bit value to send to the DAC.
 *
 * Bits   | Description
 * :----: | -----------
 *  0-1   | Reserved
 *  2-11  | D0-D9
 *   12   | OP0, see below
 *   13   | OP1, see below
 *   14   | 0=DAC A, 1=DAC B
 *   15   | Always 0
 *
 *  OP0   |  OP1   | Description
 * :----: | :----: | -----------
 *    0   |    0   | Write to specified register but do not update outputs.
 *    1   |    0   | Write to specified register and update outputs.
 *    0   |    1   | Write to all registers and update outputs.
 *    1   |    1   | Power-down outputs.
 *
 * @param __val  A 12-bit value with the data in the upper 10-bits
 * @param __out  SPI_DAC_OUT_A or SPI_DAC_OUT_B
 */
#define SPI_DAC_VALUE(__out, __val) ((0<<15) | ((__out)<<14) | (0<<13) | (1<<12) | (SPI_DAC_LIMIT(__val) & 0xffc))


/******************************************************************************
 * Functions
 *****************************************************************************/

void spi_dac_init(void);
void spi_dac_write(uint16_t data);
void spi_dac_stop(void);


#endif /* end __SPI_DAC_H */
