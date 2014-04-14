/*!
 * @file
 * @brief   A circular buffer for continuous capturing of samples
 * @ingroup FUNC_CAP
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


/******************************************************************************
 * Includes
 *****************************************************************************/

#include "log.h"
#include "circbuff.h"
#include <string.h>

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/


/******************************************************************************
 * Global variables
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/


/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/


/******************************************************************************
 * Global Functions
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * External method
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes the circular buffer
 *
 * @param [in,out] pBuff  The buffer to initialize
 * @param [in]     addr   The address to start the buffer at
 * @param [in]     size   The size of the buffer in bytes
 *
 *****************************************************************************/
void circbuff_Init(circbuff_t* pBuff, uint32_t addr, uint32_t size)
{
  pBuff->data    = (uint8_t*) addr;
  pBuff->size    = size;
  pBuff->maxSize = size;
  pBuff->last    = 0;
  pBuff->empty   = TRUE;

  /* To help troubleshooting the entire buffer is filled to see what is
     actually copied and what is leftovers. */
  //memset(pBuff->data, 0x00, 0x10000);
//  memset((uint8_t*)0x20000000, 0x00, 0x10000);
  memset(pBuff->data, 0xea, size);
}

/**************************************************************************//**
 *
 * @brief  Changes the size of the buffer
 *
 * @param [in,out] pBuff       The buffer to resize
 * @param [in]     actualSize  The new size
 *
 *****************************************************************************/
void circbuff_Resize(circbuff_t* pBuff, uint32_t actualSize)
{
  pBuff->size = MIN(pBuff->maxSize, actualSize);
}

/**************************************************************************//**
 *
 * @brief  Empties the buffer
 *
 * @param [in,out] pBuff       The buffer to reset
 *
 *****************************************************************************/
void circbuff_Reset(circbuff_t* pBuff)
{
  pBuff->last  = 0;
  pBuff->empty = TRUE;

  /* To help troubleshooting the entire buffer is filled to see what is
     actually copied and what is leftovers. */
  //memset(pBuff->data, 0x00, 0x10000);
//  memset((uint8_t*)0x20000000, 0x00, 0x10000);
  memset(pBuff->data, 0xea, pBuff->size);
}

/**************************************************************************//**
 *
 * @brief  Returns true if the buffer is valid and is filled
 *
 * @param [in] pBuff       The buffer to reset
 *
 * @return TRUE if valid and filled
 *
 *****************************************************************************/
Bool circbuff_Full(const circbuff_t * const pBuff)
{
  if (pBuff == NULL || pBuff->empty)
  {
    return FALSE;
  }
  return TRUE;
}

/**************************************************************************//**
 *
 * @brief  Returns the next position in the buffer that can hold \a incrInBytes bytes
 *
 * This operation moves the buffer's cursor effectively reserving the specified
 * number of bytes.
 *
 * @param [in,out] pBuff        The buffer
 * @param [in]     incrInBytes  Number of bytes that will be reserved
 *
 * @return The address
 *
 *****************************************************************************/
uint32_t circbuff_NextPos(circbuff_t* pBuff, uint32_t incrInBytes)
{
  uint32_t addr;

  if ((pBuff->last + incrInBytes) > pBuff->size)
  {
    pBuff->empty = FALSE;
    pBuff->last = incrInBytes;
    addr = ((uint32_t)pBuff->data);
  }
  else
  {
    addr = ((uint32_t)pBuff->data) + pBuff->last;
    pBuff->last += incrInBytes;
  }
  return addr;
}

/**************************************************************************//**
 *
 * @brief  Returns address to the first (oldest) byte in the circular buffer
 *
 * @param [in] pBuff  The buffer
 *
 * @return The address to the first byte
 *
 *****************************************************************************/
uint32_t circbuff_GetFirstAddr(const circbuff_t * const pBuff)
{
  if (pBuff->empty)
  {
    return (uint32_t)pBuff->data;
  }
  else
  {
    return ((uint32_t)pBuff->data) + (pBuff->last % pBuff->size);
  }
}

/**************************************************************************//**
 *
 * @brief  Returns the number of bytes stored in the circular buffer
 *
 * @param [in] pBuff  The buffer
 *
 * @return The number of bytes in the buffer
 *
 *****************************************************************************/
uint32_t circbuff_GetUsedSize(const circbuff_t * const pBuff)
{
  if (pBuff == NULL)
  {
    return 0;
  }
  if (pBuff->empty)
  {
    return pBuff->last;
  }
  else
  {
    return pBuff->size;
  }
}

/**************************************************************************//**
 *
 * @brief  Finds which address \a addrInBuff will get when the circular buffer is straightened
 *
 * @param [in] pBuff       The buffer
 * @param [in] addrInBuff  The address in the buffer
 *
 * @return The corresponding address after the buffer has been straightened out
 *
 *****************************************************************************/
uint32_t circbuff_ConvertAddress(const circbuff_t * const pBuff, uint32_t addrInBuff)
{
  if (pBuff == NULL)
  {
    return 0;
  }
  else
  {
    uint32_t first = circbuff_GetFirstAddr(pBuff);
    if (addrInBuff >= first)
    {
      // addrInBuff is either in an non-filled buffer or in the first part of a wrapped buffer
      return addrInBuff - first;
    }
    else
    {
      // addrInBuff is in the second half of a wrapped buffer, add size of first half
      return addrInBuff - ((uint32_t)pBuff->data) + pBuff->size - pBuff->last;
    }
  }
}
