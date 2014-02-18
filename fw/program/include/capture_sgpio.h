/*!
 * @file
 * @brief     Handles capturing of digital signals using SGPIO
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
#ifndef __CAPTURE_SGPIO_H
#define __CAPTURE_SGPIO_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "circbuff.h"
#include "capture.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Configuration for digital signal capture.
 * This is part of the \ref capture_cfg_t structure that the client software
 * must send to configure capture of analog and/or digital signals.
 */
typedef struct
{
  /*! @brief Which digital signals should be sampled.
   *
   * Bit assignment (0 = not sampled, 1 = sampled):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for \a DIO_0
   *    1   | Setting for \a DIO_1
   *    2   | Setting for \a DIO_2
   *    3   | Setting for \a DIO_3
   *    4   | Setting for \a DIO_4
   *    5   | Setting for \a DIO_5
   *    6   | Setting for \a DIO_6
   *    7   | Setting for \a DIO_7
   *    8   | Setting for \a DIO_8
   *    9   | Setting for \a DIO_9
   *   10   | Setting for \a DIO_CLK
   *  11-31 | Reserved
   */
  uint32_t enabledChannels;

  /*! @brief Which digital signals have triggering conditions.
   *
   * Bit assignment (0 = not triggering, 1 = have trigger condition):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for \a DIO_0
   *    1   | Setting for \a DIO_1
   *    2   | Setting for \a DIO_2
   *    3   | Setting for \a DIO_3
   *    4   | Setting for \a DIO_4
   *    5   | Setting for \a DIO_5
   *    6   | Setting for \a DIO_6
   *    7   | Setting for \a DIO_7
   *    8   | Setting for \a DIO_8
   *    9   | Setting for \a DIO_9
   *   10   | Setting for \a DIO_CLK
   *  11-31 | Reserved
   */
  uint32_t enabledTriggers;

  /*! @brief Trigger information.
   *
   * Two bits are used per channel:
   * - 00 = falling edge
   * - 01 = rising edge
   * - 10 = high level
   * - 11 = low level
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-1  | Setting for \a DIO_0
   *   2-3  | Setting for \a DIO_1
   *   4-5  | Setting for \a DIO_2
   *   6-7  | Setting for \a DIO_3
   *   8-9  | Setting for \a DIO_4
   *  10-11 | Setting for \a DIO_5
   *  12-13 | Setting for \a DIO_6
   *  14-15 | Setting for \a DIO_7
   *  16-17 | Setting for \a DIO_8
   *  18-19 | Setting for \a DIO_9
   *  20-21 | Setting for \a DIO_CLK
   *  22-31 | Reserved
   */
  uint32_t triggerSetup;
} cap_sgpio_cfg_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void cap_sgpio_Init(void);
cmd_status_t cap_sgpio_Configure(circbuff_t* buff, cap_sgpio_cfg_t* cfg, uint32_t postFill, Bool forceTrigger, uint32_t shiftClockPreset);
cmd_status_t cap_sgpio_PrepareToArm(void);
void cap_sgpio_Arm(void);
cmd_status_t cap_sgpio_Disarm(void);
void cap_sgpio_Triggered(void);

#endif /* end __CAPTURE_SGPIO_H */

