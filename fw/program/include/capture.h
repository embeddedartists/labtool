/*!
 * @file
 * @brief     Handles setup shared by analog and digital signal capturing
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
#ifndef __CAPTURE_H
#define __CAPTURE_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "circbuff.h"
#include "error_codes.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Sets the global prefill flag \ref capture_PrefillComplete__ to indicate
 * that both SGPIO and VADC needs prefill before allowing triggering 
 */
#define CAP_PREFILL_SET_AS_NEEDED()    (capture_PrefillComplete__  = 0)

/*! Sets the global prefill flag \ref capture_PrefillComplete__ to indicate
 * that SGPIO has reached the needed level of prefill
 */
#define CAP_PREFILL_MARK_SGPIO_DONE()  (capture_PrefillComplete__ |= 1)

/*! Sets the global prefill flag \ref capture_PrefillComplete__ to indicate
 * that VADC has reached the needed level of prefill
 */
#define CAP_PREFILL_MARK_VADC_DONE()   (capture_PrefillComplete__ |= 2)

/*! Tests if all needed parts (SGPIO and/or VADC depending on what is being
 * captured) have reached their prefill levels.
 */
#define CAP_PREFILL_IS_PREFILL_DONE()  (capture_PrefillComplete__ == 3)

/*! Tests if SGPIO has reached the prefill level. */
#define CAP_PREFILL_IS_SGPIO_DONE()    (capture_PrefillComplete__  & 1)

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/*! Should never be used directly. Access should be through one of the
 * CAP_PREFILL_* macros. */
extern volatile uint8_t capture_PrefillComplete__;

/******************************************************************************
 * Functions
 *****************************************************************************/

void capture_Init(void);

cmd_status_t capture_Configure(uint8_t* cfg, uint32_t size);
cmd_status_t capture_Arm(void);
cmd_status_t capture_Disarm(void);
cmd_status_t capture_ConfigureForCalibration(int voltsPerDiv);

uint16_t capture_GetVadcMatchValue(void);
uint32_t capture_GetFadc(void);
uint32_t capture_GetSampleRate(void);

void capture_ReportSGPIODone(circbuff_t* buff, uint32_t trigpoint, uint32_t triggerSample, uint32_t activeChannels);
void capture_ReportSGPIOSamplingFailed(cmd_status_t error);

void capture_ReportVADCDone(circbuff_t* buff, uint32_t trigpoint, uint32_t triggerSample, uint32_t activeChannels);
void capture_ReportVADCSamplingFailed(cmd_status_t error);

#endif /* end __CAPTURE_H */

