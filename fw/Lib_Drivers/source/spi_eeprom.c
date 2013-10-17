/*!
 * @file
 * @brief     Driver for the SPI EEPROM
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
#include "spi_eeprom.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/*! Chip Select On. GPIO is used instead of the SSP_SSEL. */
#define CS_ON    (LPC_GPIO_PORT->CLR[5] |= (1UL << 9)) // p3.2 (gpio5[9]) -> low

/*! Chip Select Off. GPIO is used instead of the SSP_SSEL. */
#define CS_OFF   (LPC_GPIO_PORT->SET[5] |= (1UL << 9)) // p3.2 (gpio5[9]) -> high

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

/******************************************************************************
 * Local Functions
 *****************************************************************************/


/**************************************************************************//**
 *
 * @brief Write a maximum of 16 bytes to the EEPROM
 *
 * @param [in] data    Data to write
 * @param [in] addr    Address in EEPROM to start to write to
 * @param [in] len     Number of bytes to write (max 16)
 *
 * @return Number of written bytes or -1 in case of an error
 *
 *****************************************************************************/
static int16_t spi_eeprom_writeMax16(uint8_t* data, uint16_t addr, uint16_t len)
{
  uint8_t buf[1 + 1 + 16]; //CMD + ADDR + DATA
  int i;
  SSP_DATA_SETUP_Type sspCfg;
  int delay;

  CS_ON;

  buf[0] = 0x6; // 0000 0110 for Write enable latch

  sspCfg.tx_data = buf;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 1;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;

  delay = 10;
  while ((delay-- > 0) && ((spi_eeprom_read_status() & 0x3) != 0x2)) // Want no WIP and enabled WEL
  {
    TIM_Waitms(2);
  }

  CS_ON;

  buf[0] = 0x2; // 0000 0010 for Write sequence
  buf[1] = (addr & 0xff);
  for (i = 0; i < len; i++)
  {
    buf[i+2] = data[i];
  }

  sspCfg.tx_data = buf;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 2+len;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;

  delay = 10;
  while ((delay-- > 0) && (spi_eeprom_read_status() & 1)) // Want no WIP
  {
    TIM_Waitms(2);
  }

  return len;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief Initialize the EEPROM Driver
 *
 *****************************************************************************/
void spi_eeprom_init(void)
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
 * @brief  Read from the EEPROM's Status Register
 *
 * @return Number the content of the status register
 *
 *****************************************************************************/
uint8_t spi_eeprom_read_status(void)
{
  uint8_t buf[1]; //CMD
  SSP_DATA_SETUP_Type sspCfg;

  CS_ON;

  buf[0] = 0x3; // 0000 0101 for Read Status Register instruction

  sspCfg.tx_data = buf;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 1;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  sspCfg.tx_data = NULL;
  sspCfg.rx_data = buf;
  sspCfg.length  = 1;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;

  TIM_Waitms(2);

  return buf[0];
}

/**************************************************************************//**
 *
 * @brief  Read from the EEPROM
 *
 * @param [in] data    Read buffer
 * @param [in] offset  Offset to start to read from
 * @param [in] len     Number of bytes to read (max 256)
 *
 * @return Number of read bytes or -1 in case of an error
 *
 *****************************************************************************/
int16_t spi_eeprom_read(uint8_t* data, uint16_t offset, uint16_t len)
{
  uint8_t buf[1 + 1]; //CMD + ADDR
  SSP_DATA_SETUP_Type sspCfg;

  CS_ON;

  buf[0] = 0x3; // 0000 0011 for Read sequence
  buf[1] = (offset & 0xff);

  sspCfg.tx_data = buf;
  sspCfg.rx_data = NULL;
  sspCfg.length  = 2;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  sspCfg.tx_data = NULL;
  sspCfg.rx_data = data;
  sspCfg.length  = len;

  SSP_ReadWrite (SSP_PORT, &sspCfg, SSP_TRANSFER_POLLING);

  CS_OFF;

  TIM_Waitms(2);

  return len;
}

/**************************************************************************//**
 *
 * @brief Write to the EEPROM
 *
 * @param [in] data    Data to write
 * @param [in] offset  Offset in EEPROM to start to write to
 * @param [in] len     Number of bytes to write (max 256)
 *
 * @return Number of written bytes or -1 in case of an error
 *
 *****************************************************************************/
int16_t spi_eeprom_write(uint8_t* data, uint16_t offset, uint16_t len)
{
  uint16_t off = 0;
  uint16_t left = len;
  uint16_t chunk;
  int16_t tmp;

  if ((offset + len) > 256)
  {
    return -1;
  }

  while (left > 0)
  {
    chunk = (left <= 16 ? left : 16);
    tmp = spi_eeprom_writeMax16(data+off, offset+off, chunk);
    if (tmp == -1) {
      return -1;
    }
    left -= tmp;
    off += tmp;

  }

  return len - left;
}
