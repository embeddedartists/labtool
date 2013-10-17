/*!
 * @file
 * @brief     Error codes shared with client application
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
#ifndef __ERROR_CODES_H
#define __ERROR_CODES_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "circbuff.h"
#include "labtool_config.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

typedef enum
{
  CMD_STATUS_OK,
  CMD_STATUS_ERR,

  /* Related to Signal Capture */
  CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE,
  CMD_STATUS_ERR_INVALID_POSTFILLPERCENT,
  CMD_STATUS_ERR_INVALID_VDIV,
  CMD_STATUS_ERR_FAILED_TO_SET_VDIV,
  CMD_STATUS_ERR_FAILED_TO_SET_ACDC_COUPLING,
  CMD_STATUS_ERR_NO_DIGITAL_SIGNALS_ENABLED,
  CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_LOW,
  CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_HIGH,
  CMD_STATUS_ERR_NOISE_REDUCTION_LEVEL_TOO_HIGH,
  CMD_STATUS_ERR_CFG_NO_CHANNELS_ENABLED,
  CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION,

  /* Related to Signal Generation */
  CMD_STATUS_ERR_NOTHING_TO_GENERATE = 25,
  CMD_STATUS_ERR_GEN_INVALID_WAVEFORM,
  CMD_STATUS_ERR_GEN_INVALID_FREQUENCY,
  CMD_STATUS_ERR_GEN_INVALID_RUN_COUNTER,
  CMD_STATUS_ERR_GEN_INVALID_NUMBER_OF_STATES,
  CMD_STATUS_ERR_GEN_INVALID_AMPLITUDE,

  /* Related to I2C monitoring */
  CMD_STATUS_ERR_MON_I2C_PCA95555_FAILED = 40,
  CMD_STATUS_ERR_MON_I2C_INVALID_RATE,
  CMD_STATUS_ERR_MON_I2C_NOT_CONFIGURED,

  /* Related to calibration of analog signals */
  CMD_STATUS_ERR_CAL_AOUT_INVALID_PARAMS = 50,
  CMD_STATUS_ERR_CAL_AIN_INVALID_PARAMS,
  CMD_STATUS_ERR_CAL_FAILED_TO_STORE_DATA,

  /* Internal state machine errors */
  CMD_STATUS_ERR_NO_SUCH_STATE = 99,
} cmd_status_t;


/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/


#endif /* end __ERROR_CODES_H */

