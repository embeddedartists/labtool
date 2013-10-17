/*!
 * @file
 * @brief     Handles digital signal generation using SGPIO
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
#ifndef __GENERATOR_SGPIO_H
#define __GENERATOR_SGPIO_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "generator.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Configuration of the digital signal(s) to generate.
 * The \a enabledChannels bit mask represents DIO0..DIO9 and DIO_CLK.
 */
typedef struct
{
  uint32_t enabledChannels; /*!< using bits 0-10, a 1 means enabled */
  uint32_t frequency;       /*!< Frequency of generated signal */
  uint32_t numStates;       /*!< Bits per channel 1..256 */
  uint32_t patterns[8][11];  /*!< Up to 8*32 states for up to 11 channels */
} gen_sgpio_cfg_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void gen_sgpio_Init(void);
cmd_status_t gen_sgpio_Configure(gen_sgpio_cfg_t* cfg, uint32_t shiftClockPreset, uint32_t runCounter);
cmd_status_t gen_sgpio_Start(void);
void gen_sgpio_Stop(void);

#endif /* end __GENERATOR_SGPIO_H */

