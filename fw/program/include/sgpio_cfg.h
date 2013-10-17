/*!
 * @file
 * @brief     SGPIO configuration common to both capturing and generation
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
#ifndef __SGPIO_CFG_H
#define __SGPIO_CFG_H

/******************************************************************************
 * Includes
 *****************************************************************************/

#include "lpc_types.h"
#include "labtool_config.h"
#include "capture_sgpio.h"
#include "capture.h"
#include "generator_sgpio.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Maximum number of SGPIO slices */
#define MAX_NUM_SLICES  16

/*! SGPIO slice names */
typedef enum
{
  SLICE_A,  // SGPIO_0
  SLICE_B,  // SGPIO_8
  SLICE_C,  // SGPIO_4
  SLICE_D,  // SGPIO_12
  SLICE_E,  // SGPIO_2
  SLICE_F,  // SGPIO_6
  SLICE_G,  // SGPIO_10
  SLICE_H,  // SGPIO_14
  SLICE_I,  // SGPIO_1
  SLICE_J,  // SGPIO_3
  SLICE_K,  // SGPIO_5
  SLICE_L,  // SGPIO_7
  SLICE_M,  // SGPIO_9
  SLICE_N,  // SGPIO_11
  SLICE_O,  // SGPIO_13
  SLICE_P,  // SGPIO_15
} sgpio_slice_t;

/*! SGPIO pin numbers */
typedef enum
{
  SGPIO_0,
  SGPIO_1,
  SGPIO_2,
  SGPIO_3,
  SGPIO_4,
  SGPIO_5,
  SGPIO_6,
  SGPIO_7,
  SGPIO_8,
  SGPIO_9,
  SGPIO_10,
  SGPIO_11,
  SGPIO_12,
  SGPIO_13,
  SGPIO_14,
  SGPIO_15,
} sgpio_pin_t;

/*! Maximum number of DIOs */
#define MAX_NUM_DIOS  11

/*! The client software's numbering of the digital inputs/outputs */
typedef enum
{
  DIO_0,
  DIO_1,
  DIO_2,
  DIO_3,
  DIO_4,
  DIO_5,
  DIO_6,
  DIO_7,
  DIO_8,
  DIO_9,
  DIO_CLK,
  DIO_UNAVAIL = 15,
} dio_t;

/*! Trigger types for digital signal sampling */
typedef enum
{
  CAP_RISING_EDGE,
  CAP_FALLING_EDGE,
  CAP_LOW_LEVEL,
  CAP_HIGH_LEVEL,
} sgpio_capture_t;

/*! @brief Configuration for one SGPIO channel */
typedef struct
{
  Bool           enabled;  /*!< TRUE if channel is used */
  Bool           internal; /*!< TRUE if slice is only used for internal concatenation of data */
  sgpio_slice_t  slice;    /*!< Slice name (A-P) */
  sgpio_pin_t    pin;      /*!< Slice pin (0-15) */
  dio_t          dio;      /*!< Digital I/O that the channel is connected to */

  uint32_t slice_mux_cfg; /*!< Slice multiplexer configuration register */
  uint32_t sgpio_mux_cfg; /*!< SGPIO multiplexer configuration register */
  uint32_t out_mux_cfg;   /*!< Pin multiplexer configuration register */
  uint32_t gpio_oenreg;   /*!< GPIO output enable register */
  uint32_t pos;           /*!< Position register */
  uint32_t preset;        /*!< Reload register - Controls the internally generated slice shift clock frequency */
  uint32_t reg;           /*!< Slice data register */
  uint32_t reg_ss;        /*!< Slice data shadow register */
  uint32_t mask;          /*!< MASK_A, MASK_H, MASK_I or MASK_P depending on slice */
  uint32_t set_en_0;      /*!< Enable mask for the shift clock interrupt */
  uint32_t set_en_1;      /*!< Enable mask for the exchange clock interrupt */
  uint32_t set_en_2;      /*!< Enable mask for the pattern match interrupt */
  uint32_t set_en_3;      /*!< Enable mask for the input bit match interrupt */

} sgpio_channel_config_t;

/*! Number of steps of SGPIO concatenation */
typedef enum
{
  SGPIO_CONCAT_NONE  = 1,
  SGPIO_CONCAT_TWO   = 2,
  SGPIO_CONCAT_FOUR  = 4,
  SGPIO_CONCAT_EIGHT = 8,
} sgpio_concat_t;

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

cmd_status_t sgpio_cfg_SetupInputChannels(sgpio_channel_config_t config[16], sgpio_concat_t* concat, cap_sgpio_cfg_t* cfg, uint32_t shiftClockPreset);
cmd_status_t sgpio_cfg_SetupOutputChannels(sgpio_channel_config_t config[16], gen_sgpio_cfg_t* cfg, uint32_t shiftClockPreset);

dio_t sgpio_cfg_GetDioForSliceInterrupt(uint32_t interruptMask);

#endif /* end __SGPIO_CFG_H */

