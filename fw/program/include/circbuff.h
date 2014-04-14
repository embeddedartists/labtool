/*!
 * @file
 * @brief     A circular buffer for continuous capturing of samples
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
#ifndef __CIRCBUFF_H
#define __CIRCBUFF_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief A circular buffer.
 *
 * The \a data and \a maxSize determines the memory area available to
 * the circular buffer. The user of the buffer can further reduce the
 * size (e.g. to get an even multiple of samples to fit) and that only
 * affects the \a size member.
 *
 * The \a last member is the offset from \a data where the next byte
 * should be written.
 */
typedef struct
{
  uint8_t* data;     /*!< Start of the data. Never changed. */
  uint32_t size;     /*!< Current size */
  uint32_t maxSize;  /*!< Maximum size of data. Never changed. */
  uint32_t last;     /*!< Offset to next byte to write. */
  Bool empty;        /*!< True if the buffer has not been filled yet. */
} circbuff_t;


/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

void circbuff_Init(circbuff_t* pBuff, uint32_t addr, uint32_t maxSize);
void circbuff_Resize(circbuff_t* pBuff, uint32_t actualSize);
void circbuff_Reset(circbuff_t* pBuff);
Bool circbuff_Full(const circbuff_t * const pBuff);
uint32_t circbuff_NextPos(circbuff_t* pBuff, uint32_t incrInBytes);
uint32_t circbuff_GetFirstAddr(const circbuff_t * const pBuff);

uint32_t circbuff_GetUsedSize(const circbuff_t * const pBuff);

uint32_t circbuff_ConvertAddress(const circbuff_t * const pBuff, uint32_t addrInBuff);

#endif /* end __CIRCBUFF_H */

