/*!
 * @file
 * @brief     Handles analog signal generation using a DAC on the SPI bus
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
#ifndef __GENERATOR_DAC_H
#define __GENERATOR_DAC_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "error_codes.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! \name Types of waveforms, used in \ref gen_dac_cfg_t
 * \{
 */
#define GEN_DAC_CFG_WAVE_SINUS         0
#define GEN_DAC_CFG_WAVE_SQUARE        1
#define GEN_DAC_CFG_WAVE_TRIANGLE      2
#define GEN_DAC_CFG_WAVE_SAWTOOTH      3
#define GEN_DAC_CFG_WAVE_INV_SAWTOOTH  4
#define GEN_DAC_CFG_WAVE_LEVEL         5
/* \} */

/*! @brief Configuration of one analog signal to generate.
 */
typedef struct
{
  /*! @brief Type of waveform to generate.
   *
   * Value | Waveform type
   * :---: | -------------
   *   0   | Sine
   *   1   | Square
   *   2   | Triangle
   *   3   | Sawtooth
   *   4   | Reverse (or inverse) Sawtooth
   *   5   | Level (outputs DC offset, ignores amplitude)
   */
  uint32_t waveform;
  uint32_t frequency; /*!< Frequency in Hz */
  uint32_t amplitude; /*!< Amplitude in mV, 0..5000 */
  int32_t  dcOffset;  /*!< DC offset in mV, -5000..5000 */
} gen_dac_one_ch_cfg_t;

/*! @brief Configuration of the analog signal(s) to generate.
 */
typedef struct
{
  uint32_t              available;  /*!< Bitmask, bit0=ch1, bit1=ch2 */
  gen_dac_one_ch_cfg_t  ch[2];      /*!< Configuration for A_OUT_1 and A_OUT_2 */
} gen_dac_cfg_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

void gen_dac_Init(void);
cmd_status_t gen_dac_Configure(const gen_dac_cfg_t * const cfg);
cmd_status_t gen_dac_Start(void);
void gen_dac_Stop(void);

#endif /* end __GENERATOR_DAC_H */

