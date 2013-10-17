/*!
 * @file
 * @brief     Driver for TI DAC102S085CIMM Dual DAC (10-bit)
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
#include "spi_dac.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/*! Chip Select On. GPIO is used instead of the SSP_SSEL. */
#define CS_ON    (LPC_GPIO_PORT->CLR[0] |= (1UL << 15)) // p1.20 (gpio0[15]) -> low

/*! Chip Select Off. GPIO is used instead of the SSP_SSEL. */
#define CS_OFF   (LPC_GPIO_PORT->SET[0] |= (1UL << 15)) // p1.20 (gpio0[15]) -> high

/*! SPI bus to use */
#define SSP_PORT  (LPC_SSP1)

/*! Clock rate to use */
#define SSP_CLOCK 20000000

/******************************************************************************
 * External global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static SSP_CFG_Type SSP_ConfigStruct;

/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief Initialize the SPI DAC driver
 *
 *****************************************************************************/
void spi_dac_init(void)
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
}


/**************************************************************************//**
 *
 * @brief Write to the SPI DAC
 *
 * Blocks until data has been transferred.
 *
 * @param [in] data    Data to write
 *
 *****************************************************************************/
void spi_dac_write(uint16_t data)
{
  SSP_DATA_SETUP_Type sspCfg;
  uint8_t tmp[2];

  tmp[0] = (data >> 8) & 0xff;
  tmp[1] = data & 0xff;

  CS_ON;

  sspCfg.tx_data = tmp;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 2;

  SSP_ReadWrite(SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;
}

/**************************************************************************//**
 *
 * @brief Shuts down the DAC
 *
 *****************************************************************************/
void spi_dac_stop(void)
{
  SSP_DATA_SETUP_Type sspCfg;
  uint16_t data = SPI_DAC_VALUE(SPI_DAC_OUT_A, 0) | (1<<14) | (1<<13); // Sets bit 13 and 14 to power down outputs

  CS_ON;

  sspCfg.tx_data = &data;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 2;

  SSP_ReadWrite(SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;
}

