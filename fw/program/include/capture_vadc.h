/*!
 * @file
 * @brief     Handles capturing of analog signals using the 12-bit VADC
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
#ifndef __CAPTURE_VADC_H
#define __CAPTURE_VADC_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "circbuff.h"
#include "capture.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Configuration for analog signal capture.
 * This is part of the \ref capture_cfg_t structure that the client software
 * must send to configure capture of analog and/or digital signals.
 */
typedef struct
{
  /*! @brief Which analog signals should be sampled.
   *
   * Bit assignment (0 = not sampled, 1 = sampled):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t enabledChannels;

  /*! @brief Which analog signals have triggering conditions.
   *
   * Bit assignment (0 = not triggering, 1 = have trigger condition):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t enabledTriggers;

  /*! @brief Trigger information (ignored if the trigger is not enabled).
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-11 | Trigger level for ch0
   *  12-13 | Reserved
   *  14-15 | 00 = rising edge, 01 = falling edge
   *  16-27 | Trigger level for ch1
   *  28-29 | Reserved
   *  30-31 | 00 = rising edge, 01 = falling edge
   */
  uint32_t triggerSetup;

  /*! @brief Volts/div configuration.
   * Values are indices in the \a VDIV_CONFIG table* found in \ref capture_vadc.c.
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-3  | Index for ch0
   *   4-7  | Index for ch1
   *   8-31 | Reserved
   */
  uint32_t voltPerDiv;

  /*! @brief AC/DC coupling information.
   *
   * Bit assignment (0 = DC, 1 = AC):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t couplings;

  /*! @brief Noise suppression.
   * The same filter is applied to both channels.
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-3  | Index for ch0
   *   4-7  | Index for ch1
   *   8-31 | Reserved
   */
  uint32_t noiseReduction;
} cap_vadc_cfg_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void cap_vadc_Init(void);
cmd_status_t cap_vadc_Configure(circbuff_t* buff, cap_vadc_cfg_t* cfg, uint32_t postFill, Bool forceTrigger);
cmd_status_t cap_vadc_PrepareToArm(void);
void cap_vadc_Arm(void);
cmd_status_t cap_vadc_Disarm(void);
void cap_vadc_Triggered(void);

uint32_t cap_vadc_GetMilliVoltsPerDiv(int ch);

#endif /* end __CAPTURE_VADC_H */

