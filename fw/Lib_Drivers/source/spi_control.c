/*!
 * @file
 * @brief     Driver for the NXP 74HC595PW Shift Register
 * @ingroup   IP_SPI
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

#include <string.h>
#include <stdio.h>
#include "lpc_types.h"
#include "lpc43xx_ssp.h"
#include "lpc43xx_timer.h"
#include "spi_control.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/*! Chip Select On. GPIO is used instead of the SSP_SSEL. */
#define CS_ON    (LPC_GPIO_PORT->CLR[3] |= (1UL << 7)) // p6.11 (gpio3[7]) -> low

/*! Chip Select Off. GPIO is used instead of the SSP_SSEL. */
#define CS_OFF   (LPC_GPIO_PORT->SET[3] |= (1UL << 7)) // p6.11 (gpio3[7]) -> high


/*! SPI bus to use */
#define SSP_PORT  (LPC_SSP1)

/*! Clock rate to use */
#define SSP_CLOCK 3000000

/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static SSP_CFG_Type SSP_ConfigStruct;

static uint8_t currentValue = DEFAULT_VALUES;

/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief Initialize the Shift Registry Driver
 *
 *****************************************************************************/
void spi_control_init(void)
{
  // Initialize SSP configuration structure to default
  SSP_ConfigStructInit(&SSP_ConfigStruct);

  // Set clock rate and number of address bits
  SSP_ConfigStruct.ClockRate = SSP_CLOCK;
  SSP_ConfigStruct.Databit = SSP_DATABIT_8;

  // Initialize SSP peripheral with parameter given in structure above
  SSP_Init(SSP_PORT, &SSP_ConfigStruct);

  // Enable SSP peripheral
  SSP_Cmd(SSP_PORT, ENABLE);

  spi_control_write(DEFAULT_VALUES, 0xff);
}

/**************************************************************************//**
 *
 * @brief Write to the Shift Register
 *
 * Blocks until data has been transferred.
 *
 * @param [in] data    Data to write
 * @param [in] mask    A bit value of 1 sets the value, 0 is ignored
 *
 *****************************************************************************/
void spi_control_write(uint8_t data, uint8_t mask)
{
  uint8_t newVal;
  SSP_DATA_SETUP_Type sspCfg;

  newVal = currentValue & ~mask;
  newVal |= (data & mask);
  if (newVal == currentValue) {
    // nothing to do
    return;
  }
  currentValue = newVal;

  CS_ON;

  sspCfg.tx_data = &newVal;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 1;

  SSP_ReadWrite(SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;
}
