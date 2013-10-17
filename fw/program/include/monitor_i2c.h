/*!
 * @file
 * @brief     Captures I2C communication
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
#ifndef __MONITOR_I2C_H
#define __MONITOR_I2C_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "error_codes.h"
#include "circbuff.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Configuration data for the I2C Monitor. */
typedef struct
{
  uint32_t clockrate;      /*!< 100000, 400000 or 1000000 */
  uint32_t bytesToCapture; /*!< Each I2C byte is timestamped with 4 bytes */
} monitor_i2c_cfg_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void monitor_i2c_Init(void);
cmd_status_t monitor_i2c_Configure(circbuff_t* buff, monitor_i2c_cfg_t* cfg);
cmd_status_t monitor_i2c_Start(void);
cmd_status_t monitor_i2c_Stop(void);

#if (TEST_I2C_MONITOR == OPT_ENABLED)
void monitor_i2c_Test(void);
#endif

#endif /* end __MONITOR_I2C_H */

