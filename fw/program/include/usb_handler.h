/*!
 * @file
 * @brief     Handles all USB communication
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
#ifndef __USB_STUFF_H
#define __USB_STUFF_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "bsp.h"
#include "USB.h"
#include "usb_descriptors.h"
#include "circbuff.h"
#include "capture.h"
#include "calibrate.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Container holding all information for a completed signal sampling. */
typedef struct
{
  uint32_t trigpoint;           /*!< Possible trigger information */
  uint32_t sgpioTrigSample;     /*!< SGPIO sample when trigger was found */
  uint32_t vadcTrigSample;      /*!< VADC sample when trigger was found */
  uint32_t sgpioActiveChannels; /*!< Which digital signals were enabled */
  uint32_t vadcActiveChannels;  /*!< Which analog signals were enabled */
  circbuff_t* sgpio_samples;    /*!< Collected digital samples or NULL */
  circbuff_t* vadc_samples;     /*!< Collected analog samples or NULL */
} captured_samples_t;


/*! \name Function pointers
 * \{
 */
typedef cmd_status_t (*cmdFunc)(void);
typedef cmd_status_t (*cmdFuncParam)(uint8_t* pData, uint32_t size);
/* \} */

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Function Prototypes
 *****************************************************************************/

void usb_handler_InitUSB(cmdFunc capStop, cmdFuncParam capConfigure, cmdFunc capRun,
                         cmdFunc genStop, cmdFuncParam genConfigure, cmdFunc genRun);
void usb_handler_SendSamples(const captured_samples_t* const cap);
void usb_handler_SignalFailedSampling(cmd_status_t error);
void usb_handler_SendCalibrationResult(const calib_result_t* const parameters);
void usb_handler_SignalFailedCalibration(cmd_status_t error);
void usb_handler_Run(void);

#endif /* end __USB_STUFF_H */

