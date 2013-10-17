/*!
 * @file
 * @brief     Driver for the SPI EEPROM
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
#ifndef __SPI_EEPROM_H
#define __SPI_EEPROM_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"

/******************************************************************************
 * Functions
 *****************************************************************************/

void spi_eeprom_init(void);
uint8_t spi_eeprom_read_status(void);
int16_t spi_eeprom_read(uint8_t* data, uint16_t offset, uint16_t len);
int16_t spi_eeprom_write(uint8_t* data, uint16_t offset, uint16_t len);


#endif /* end __SPI_EEPROM_H */
