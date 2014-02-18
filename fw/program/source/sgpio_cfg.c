/*!
 * @file
 * @brief   SGPIO configuration common to both capturing and generation
 * @ingroup IP_SGPIO
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

#include "lpc43xx.h"

#include "lpc43xx_cgu.h"
#include "log.h"
#include "capture_sgpio.h"
#include "sgpio_cfg.h"
#include "labtool_config.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Calculates the value for the POS register to exchange the content every
 * __k * 32 bits. */
#define CLOCK_POS(__k)  (((0x20 * (__k)) - 1) & 0xFF)

/*! Calculates the value for the PRESET register. Parameter 1 <= __x <= 4096. */
#define CLOCK_PRESET(__x) ((((__x) & 0x1fff) - 1) & 0xfff)

/******************************************************************************
 * Global variables
 *****************************************************************************/

#ifdef USE_INTERNAL_CLOCK
  /* Mask to use for the slice generating the SGPIO8 clock */
  uint32_t g_spgio_clock_mask = 0xAAAAAAAA;
#endif

/******************************************************************************
 * Local variables
 *****************************************************************************/

/*! Lookup table for SGPIO slice to SGPIO pin. Example: SLICE_L = SGPIO_7. */
static const sgpio_pin_t SGPIO_PIN_FROM_SLICE[16] =
{
  SGPIO_0,SGPIO_8,SGPIO_4,SGPIO_12,
  SGPIO_2,SGPIO_6,SGPIO_10,SGPIO_14,
  SGPIO_1,SGPIO_3,SGPIO_5,SGPIO_7,
  SGPIO_9,SGPIO_11,SGPIO_13,SGPIO_15
};

/*! Lookup table for SGPIO slice to DIO number (as used in the client software).
 * Example: SLICE_L = DIO_5. */
static const dio_t DIO_FROM_SLICE[16] =
{
  /* A */ DIO_0,
  /* B */ DIO_CLK,
  /* C */ DIO_0,
  /* D */ DIO_9,
  /* E */ DIO_4,
  /* F */ DIO_4,
  /* G */ DIO_3,
  /* H */ DIO_6,
  /* I */ DIO_2,
  /* J */ DIO_5,
  /* K */ DIO_2,
  /* L */ DIO_5,
  /* M */ DIO_7,
  /* N */ DIO_8,
  /* O */ DIO_1,
  /* P */ DIO_UNAVAIL
};

/*! Lookup table for DIO number (as used in the client software) to SGPIO slice.
 * Example: DIO_4 = SLICE_E. */
static const sgpio_slice_t SLICE_FROM_DIO[MAX_NUM_DIOS] =
{
  /* DIO0 */ SLICE_A, // or SLICE_C
  /* DIO1 */ SLICE_O,
  /* DIO2 */ SLICE_K, // or SLICE_I
  /* DIO3 */ SLICE_G,
  /* DIO4 */ SLICE_E, // or SLICE_F
  /* DIO5 */ SLICE_L, // or SLICE_J
  /* DIO6 */ SLICE_H,
  /* DIO7 */ SLICE_M,
  /* DIO8 */ SLICE_N,
  /* DIO9 */ SLICE_D,
  /* DIOCLK */ SLICE_B
};


/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes the SGPIO channel as input and without interrupts
 *
 * @param [in] slice             The SGPIO slice (A-P)
 * @param [in] shiftClockPreset  Clocking
 * @param [in] pCh               The channel configuration to initialize
 *
 *****************************************************************************/
static void sgpio_cfg_DefaultConfig(sgpio_slice_t slice, uint32_t shiftClockPreset, sgpio_channel_config_t* pCh)
{
  pCh->enabled  = TRUE;
  pCh->internal = FALSE;
  pCh->slice    = slice;
  pCh->pin      = SGPIO_PIN_FROM_SLICE[slice];
  pCh->dio      = DIO_FROM_SLICE[slice];

  pCh->slice_mux_cfg = /* Slice multiplexer configuration register */
    (0<<0) |       /* MATCH_MODE:        0=do not match, 1=match */
    (0<<1) |       /* CLK_CAPTURE_MODE:  0=rising, 1=falling clock edge */
    (0<<2) |       /* CLKGEN_MODE:       0=internal, 1=external */
    (0<<3) |       /* INV_OUT_CLK:       0=normal, 1=inverted clock */
    (0<<4) |       /* DATA_CAPTURE_MODE: 0=rising edge, 1=falling edge, 2=low level, 3=high level */
    (0<<6) |       /* PARALLEL_MODE:     0=shift 1 bit, 1=shift 2 bits, 2=shift 4 bits, 3=shift 8 bits per clock */
    (0<<8);        /* INV_QUALIFIER:     0=normal, 1=inverted qualifier */

  pCh->sgpio_mux_cfg = /* SGPIO multiplexer configuration register */
    (0<<0) |       /* EXT_CLK_ENABLE:        0=internal clock (slice), 1=external clock (pin) */
    (0<<1) |       /* CLK_SOURCE_PIN_MODE:   0=SGPIO8, 1=SGPIO9, 2=SGPIO10, 3=SGPIO11 */
    (0<<3) |       /* CLK_SOURCE_SLICE_MODE: 0=slice D, 1=slice H, 2=slice O, 3=slice P */
    (0<<5) |       /* QUALIFIER_MODE:        0=enable, 1=disable, 2=slice, 3=external sgpio */
    (0<<7) |       /* QUALIFIER_PIN_MODE:    0=SGPIO8, 1=SGPIO9, 2=SGPIO10, 3=SGPIO11 */
    (0<<9) |       /* QUALIFIER_SLICE_MODE:  0=A/D, 1=H/O, 2=I/D, 3=P/O */
    (0<<11) |      /* CONCAT_ENABLE:         0=external data pin, 1=concatenate data */
    (0<<12);       /* CONCAT_ORDER:          0=self loop, 1=2 slices, 2=4 slices, 3=8 slices */

  pCh->out_mux_cfg = /* Pin multiplexer configuration register */
    (0<<0) |       /* P_OUT_CFG:     Output control of output SGPIOn. See table 212 */
    (0<<4);        /* P_OE_CFG:      Output enable source. See table 212 */

  pCh->gpio_oenreg = 0;

  pCh->pos =         /* Position register */
    (CLOCK_POS(1)<<0) |   /* POS:         Each time COUNT reaches 0x0 POS counts down */
    (CLOCK_POS(1)<<8);    /* POS_PRESET:  Reload value for POS after POS reaches 0x0 */

  pCh->preset =      /* Reload register - Controls the internally generated slice shift clock frequency */
    (CLOCK_PRESET(shiftClockPreset)<<0);    /* PRESET:      frequency(shift_cloc k) = frequency(SGPIO_CLK) / (PRESET+1) */

  // unused
  pCh->reg      = 0;
  pCh->reg_ss   = 0;
  pCh->mask     = 0;
  pCh->set_en_0 = 0;
  pCh->set_en_1 = 0;
  pCh->set_en_2 = 0;
  pCh->set_en_3 = 0;
}

#ifdef USE_INTERNAL_CLOCK
/**************************************************************************//**
 *
 * @brief  Configures the SGPIO channel for continuous output of a clock signal
 *
 * @param [in] slice             The SGPIO slice (A-P)
 * @param [in] shiftClockPreset  Clocking
 * @param [in] pCh               The channel configuration to initialize
 *
 *****************************************************************************/
static void sgpio_cfg_ClockSlice(sgpio_slice_t slice, uint32_t shiftClockPreset, sgpio_channel_config_t* pCh)
{
  sgpio_cfg_DefaultConfig(slice, shiftClockPreset, pCh);

  pCh->slice_mux_cfg = /* Slice multiplexer configuration register */
    (1<<0);       /* MATCH_MODE:        0=do not match, 1=match */

  pCh->sgpio_mux_cfg = /* SGPIO multiplexer configuration register */
    (1<<11);      /* CONCAT_ENABLE:     0=external data pin, 1=concatenate data */

  pCh->gpio_oenreg = (1 << pCh->pin);

  // data is looping back forever
  pCh->reg = g_spgio_clock_mask;
}
#endif

/**************************************************************************//**
 *
 * @brief  Configures the SGPIO channel for input sampling
 *
 * @param [in] slice             The SGPIO slice (A-P)
 * @param [in] shiftClockPreset  Clocking
 * @param [in] pCh               The channel configuration to initialize
 *
 *****************************************************************************/
static void sgpio_cfg_InputSlice(sgpio_slice_t slice, uint32_t shiftClockPreset, sgpio_channel_config_t* pCh)
{
  sgpio_cfg_DefaultConfig(slice, shiftClockPreset, pCh);

  pCh->gpio_oenreg = 0;
}

/**************************************************************************//**
 *
 * @brief  Configures the SGPIO channel for input sampling with triggering
 *
 * @param [in] slice             The SGPIO slice (A-P)
 * @param [in] cap               Information about type of trigger
 * @param [in] shiftClockPreset  Clocking
 * @param [in] pCh               The channel configuration to initialize
 *
 *****************************************************************************/
static void sgpio_cfg_CaptureSlice(sgpio_slice_t slice, sgpio_capture_t cap, uint32_t shiftClockPreset, sgpio_channel_config_t* pCh)
{
  sgpio_cfg_InputSlice(slice, shiftClockPreset, pCh);

  // DATA_CAPTURE_MODE: 0=rising edge, 1=falling edge, 2=low level, 3=high level
  pCh->slice_mux_cfg |= (cap << 4);

  // Enable the input bit match interrupt for the slice
  pCh->set_en_3 = (1 << slice);
}

/**************************************************************************//**
 *
 * @brief  Configures the SGPIO channel to generate a signal
 *
 * @param [in] slice             The SGPIO slice (A-P)
 * @param [in] pattern           Pattern to output
 * @param [in] shiftClockPreset  Clocking
 * @param [in] pCh               The channel configuration to initialize
 *
 *****************************************************************************/
static void sgpio_cfg_OutputSlice(sgpio_slice_t slice, uint32_t pattern, uint32_t shiftClockPreset, sgpio_channel_config_t* pCh)
{
  sgpio_cfg_DefaultConfig(slice, shiftClockPreset, pCh);

  pCh->sgpio_mux_cfg = /* SGPIO multiplexer configuration register */
    (0<<0) |       /* EXT_CLK_ENABLE:        0=internal clock (slice), 1=external clock (pin) */
    (0<<1) |       /* CLK_SOURCE_PIN_MODE:   0=SGPIO8, 1=SGPIO9, 2=SGPIO10, 3=SGPIO11 */
    (0<<3) |       /* CLK_SOURCE_SLICE_MODE: 0=slice D, 1=slice H, 2=slice O, 3=slice P */
    (0<<5) |       /* QUALIFIER_MODE:        0=enable, 1=disable, 2=slice, 3=external sgpio */
    (0<<7) |       /* QUALIFIER_PIN_MODE:    0=SGPIO8, 1=SGPIO9, 2=SGPIO10, 3=SGPIO11 */
    (0<<9) |       /* QUALIFIER_SLICE_MODE:  0=A/D, 1=H/O, 2=I/D, 3=P/O */
    (1<<11) |      /* CONCAT_ENABLE:         0=external data pin, 1=concatenate data */
    (0<<12);       /* CONCAT_ORDER:          0=self loop, 1=2 slices, 2=4 slices, 3=8 slices */

  pCh->gpio_oenreg = 1 << pCh->pin;

  pCh->reg = pattern;
  pCh->reg_ss = pattern;
}

/**************************************************************************//**
 *
 * @brief  Configures the concatenation of two SGPIO channels
 *
 * Configures \a secondSlice to take it's input from \a firstSlice.
 *
 * Without concatenation each digital input is sampled by one SGPIO channel
 * and after 32 sampled values those are copied to the circular buffer.
 *
 * By concatenating two slices it is possible to reduce the number of copy
 * operations by a factor two. Instead of copying 32bits for each 32
 * sampled values 64 bits are copied for each 64 sampled values. The same
 * amount of data, but with less overhead.
 *
 * This function can be called repeatedly to create chains of 2, 4 or 8 slices.
 *
 * Example:
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_A, SLICE_I, 2);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_I, SLICE_E, 2);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_E, SLICE_J, 2);
 *
 *    Creates the input chain:
 *
 *    External Pin -> A -> I -> E -> J
 *
 * Example:
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_O, SLICE_H, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_H, SLICE_P, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_P, SLICE_B, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_B, SLICE_M, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_M, SLICE_G, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_G, SLICE_N, 3);
 *     sgpio_cfg_SetupConcatSlice(..., SLICE_N, SLICE_D, 3);
 *
 *    Creates the input chain:
 *
 *    External Pin -> O -> H -> P -> B -> M -> G -> N -> D
 *
 * @param [in] config            Configuration for all slices
 * @param [in] shiftClockPreset  Clocking
 * @param [in] firstSlice        The first SGPIO slice (A-P)
 * @param [in] secondSlice       The second SGPIO slice (A-P)
 * @param [in] order             0=self loop, 1=2 slices, 2=4 slices, 3=8 slices
 *
 * @retval CMD_STATUS_OK    If successfully concatenated
 * @retval CMD_STATUS_ERR   If the \a secondSlice is already in use
 *
 *****************************************************************************/
static cmd_status_t sgpio_cfg_SetupConcatSlice(sgpio_channel_config_t config[16], uint32_t shiftClockPreset, sgpio_slice_t firstSlice, sgpio_slice_t secondSlice, uint32_t order)
{
  if (config[secondSlice].enabled)
  {
    log_i("Concatenation slice already enabled\r\n");
    return CMD_STATUS_ERR;
  }

  sgpio_cfg_InputSlice(secondSlice, shiftClockPreset, &config[secondSlice]);

  config[secondSlice].sgpio_mux_cfg |= /* SGPIO multiplexer configuration register */
    (1<<11) |      /* CONCAT_ENABLE:  0=external data pin, 1=concatenate data */
    (order<<12);   /* CONCAT_ORDER:   0=self loop, 1=2 slices, 2=4 slices, 3=8 slices */

  config[secondSlice].internal = TRUE;

  config[secondSlice].pos =    /* Position register */
    (CLOCK_POS((1<<order))<<0) |   /* POS:         Each time COUNT reaches 0x0 POS counts down */
    (CLOCK_POS((1<<order))<<8);    /* POS_PRESET:  Reload value for POS after POS reaches 0x0 */

  config[firstSlice].pos =    /* Position register */
    (CLOCK_POS((1<<order))<<0) |   /* POS:         Each time COUNT reaches 0x0 POS counts down */
    (CLOCK_POS((1<<order))<<8);    /* POS_PRESET:  Reload value for POS after POS reaches 0x0 */

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Configures all concatenation of SGPIO channels
 *
 * The slices will be concatenated like this for two-step concatenation:
 *
 *   External Pin | First Slice | Second Slice
 *   :----------: | :---------: | :----------:
 *      DIO_0     |   SLICE_A   |   SLICE_I
 *      DIO_1     |   SLICE_O   |   SLICE_D
 *      DIO_2     |   SLICE_K   |   SLICE_C
 *      DIO_3     |   SLICE_G   |   SLICE_N
 *      DIO_4     |   SLICE_E   |   SLICE_J
 *      DIO_5     |   SLICE_L   |   SLICE_F
 *      DIO_6     |   SLICE_H   |   SLICE_P
 *      DIO_7     |   SLICE_M   |   SLICE_B
 *      DIO_8     |     N/A     |     N/A
 *      DIO_9     |     N/A     |     N/A
 *      DIO_CLK   |     N/A     |     N/A
 *
 * The slices will be concatenated like this for four-step concatenation:
 *
 *   External Pin | First Slice | Second Slice | Third Slice | Fourth Slice
 *   :----------: | :---------: | :----------: | :---------: | :----------:
 *      DIO_0     |   SLICE_A   |   SLICE_I    |   SLICE_E   |   SLICE_J
 *      DIO_1     |   SLICE_O   |   SLICE_H    |   SLICE_P   |   SLICE_D
 *      DIO_2     |   SLICE_K   |   SLICE_F    |   SLICE_L   |   SLICE_C
 *      DIO_3     |   SLICE_G   |   SLICE_N    |   SLICE_B   |   SLICE_M
 *      DIO_4     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_5     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_6     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_7     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_8     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_9     |     N/A     |     N/A      |     N/A     |     N/A
 *      DIO_CLK   |     N/A     |     N/A      |     N/A     |     N/A
 *
 * The slices will be concatenated like this for eight-step concatenation:
 *
 *   External Pin | First Slice | Second Slice | Third Slice | Fourth Slice | Fifth Slice | Sixth Slice | Seventh Slice | Eighth Slice
 *   :----------: | :---------: | :----------: | :---------: | :----------: | :---------: | :---------: | :-----------: | :----------:
 *      DIO_0     |   SLICE_A   |   SLICE_I    |   SLICE_E   |   SLICE_J    |   SLICE_C   |   SLICE_K   |    SLICE_F    |   SLICE_L
 *      DIO_1     |   SLICE_O   |   SLICE_H    |   SLICE_P   |   SLICE_B    |   SLICE_M   |   SLICE_G   |    SLICE_N    |   SLICE_D
 *      DIO_2     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_3     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_4     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_5     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_6     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_7     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_8     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_9     |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *      DIO_CLK   |     N/A     |     N/A      |     N/A     |     N/A      |     N/A     |     N/A     |      N/A      |     N/A
 *
 * @param [in]  config            Configuration for all slices
 * @param [in]  shiftClockPreset  Clocking
 * @param [in]  enabledChannels   Bitmask of enabled channels
 * @param [out] concat            Number of steps of concatenation (if any)
 *
 * @retval CMD_STATUS_OK    If successfully concatenated
 * @retval CMD_STATUS_ERR   If the \a secondSlice is already in use
 *
 *****************************************************************************/
static cmd_status_t sgpio_cfg_SetupConcatSlices(sgpio_channel_config_t config[16],
                                                uint32_t shiftClockPreset,
                                                uint32_t enabledChannels,
                                                sgpio_concat_t* concat)
{
  cmd_status_t result = CMD_STATUS_OK;

  if (enabledChannels <= 0x003)
  {
    *concat = SGPIO_CONCAT_EIGHT;

    log_i("Using 8-step concatenation\r\n");

    if (config[SLICE_FROM_DIO[DIO_0]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_A, SLICE_I, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_I, SLICE_E, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_E, SLICE_J, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_J, SLICE_C, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_C, SLICE_K, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_K, SLICE_F, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_F, SLICE_L, 3);
    }
    if (config[SLICE_FROM_DIO[DIO_1]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_O, SLICE_H, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_H, SLICE_P, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_P, SLICE_B, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_B, SLICE_M, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_M, SLICE_G, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_G, SLICE_N, 3);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_N, SLICE_D, 3);
    }
  }
  else if (enabledChannels <= 0x00f)
  {
    *concat = SGPIO_CONCAT_FOUR;

    log_i("Using 4-step concatenation\r\n");

    if (config[SLICE_FROM_DIO[DIO_0]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_A, SLICE_I, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_I, SLICE_E, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_E, SLICE_J, 2);
    }
    if (config[SLICE_FROM_DIO[DIO_1]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_O, SLICE_H, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_H, SLICE_P, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_P, SLICE_D, 2);
    }
    if (config[SLICE_FROM_DIO[DIO_2]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_K, SLICE_F, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_F, SLICE_L, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_L, SLICE_C, 2);
    }
    if (config[SLICE_FROM_DIO[DIO_3]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_G, SLICE_N, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_N, SLICE_B, 2);
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_B, SLICE_M, 2);
    }
  }
  else if (enabledChannels <= 0x0ff)
  {
    *concat = SGPIO_CONCAT_TWO;

    log_i("Using 2-step concatenation\r\n");

    if (config[SLICE_FROM_DIO[DIO_0]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_A, SLICE_I, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_1]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_O, SLICE_D, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_2]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_K, SLICE_C, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_3]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_G, SLICE_N, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_4]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_E, SLICE_J, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_5]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_L, SLICE_F, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_6]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_H, SLICE_P, 1);
    }
    if (config[SLICE_FROM_DIO[DIO_7]].enabled)
    {
      result = sgpio_cfg_SetupConcatSlice(config, shiftClockPreset, SLICE_M, SLICE_B, 1);
    }
  }
  else
  {
    *concat = SGPIO_CONCAT_NONE;

    log_i("Not using concatenation\r\n");

    return CMD_STATUS_OK;
  }

  return result;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Applies the client's configuration to all SGPIO channels for sampling
 *
 * @param [out] config            Configuration for all slices
 * @param [out] concat            Number of steps of concatenation
 * @param [in]  cfg               The configuration from the client software
 * @param [in]  shiftClockPreset  Clocking
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   If the configuration was invalid
 *
 *****************************************************************************/
cmd_status_t sgpio_cfg_SetupInputChannels(sgpio_channel_config_t config[16],
                                          sgpio_concat_t* concat,
                                          cap_sgpio_cfg_t* cfg,
                                          uint32_t shiftClockPreset)
{
  int i;
  int enabled = 0;
  Bool first = TRUE;

#ifdef USE_INTERNAL_CLOCK
  sgpio_cfg_ClockSlice(SLICE_B, &config[SLICE_B]);
  config[SLICE_B].gpio_oenreg = (1 << config[SLICE_B].pin); // not allowed to be output
#endif

  for (i = 0; i < MAX_NUM_DIOS; i++)
  {
    sgpio_slice_t slice = SLICE_FROM_DIO[i];
    if (cfg->enabledChannels & (1<<i))
    {
      enabled++;
      if (cfg->enabledTriggers & (1<<i))
      {
        sgpio_capture_t trig = (sgpio_capture_t)((cfg->triggerSetup >> (2*i)) & 0x3);
        sgpio_cfg_CaptureSlice(slice, trig, shiftClockPreset, &config[slice]);
        if (first)
        {
          first = FALSE;

          // to get an interrupt when REG has been copied to REG_SS
          config[slice].set_en_1 |= (1 << slice);
        }
      }
      else
      {
        sgpio_cfg_InputSlice(slice, shiftClockPreset, &config[slice]);
      }
    }
  }

  sgpio_cfg_SetupConcatSlices(config, shiftClockPreset, cfg->enabledChannels, concat);

  if (enabled==0)
  {
    // nothing to do, no enabled channels
    return CMD_STATUS_ERR_NO_DIGITAL_SIGNALS_ENABLED;
  }
  if (first)
  {
    // no triggers selected, use forced triggering

    // must still have one slice configured to generate interrupts when REG
    // has been copied to REG_SS
    for (i = 0; i < MAX_NUM_DIOS; i++)
    {
      sgpio_slice_t slice = SLICE_FROM_DIO[i];
      if (config[slice].enabled)
      {
        // to get an interrupt when REG has been copied to REG_SS
        config[slice].set_en_1 |= (1 << slice);
        break;
      }
    }
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Applies the client's configuration to all SGPIO channels for signal generation
 *
 * @param [out] config            Configuration for all slices
 * @param [in]  cfg               The configuration from the client software
 * @param [in]  shiftClockPreset  Clocking
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   If the configuration was invalid
 *
 *****************************************************************************/
cmd_status_t sgpio_cfg_SetupOutputChannels(sgpio_channel_config_t config[16], gen_sgpio_cfg_t* cfg, uint32_t shiftClockPreset)
{
  int i;
  int enabled = 0;
  Bool first = TRUE;

  for (i = 0; i < MAX_NUM_DIOS; i++)
  {
    sgpio_slice_t slice = SLICE_FROM_DIO[i];
    if (cfg->enabledChannels & (1<<i))
    {
      enabled++;
      sgpio_cfg_OutputSlice(slice, 0/*cfg->patterns[i]*/, shiftClockPreset, &config[slice]);
      if (first)
      {
        first = FALSE;

        // to get an interrupt when REG has been copied to REG_SS
        config[slice].set_en_1 |= (1 << slice);
      }
    }
  }

  if (enabled==0)
  {
    // nothing to do, no enabled channels
    return CMD_STATUS_ERR_NO_DIGITAL_SIGNALS_ENABLED;
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Extracts SGPIO slice (A-P) from interrupt mask and translates to digital input number
 *
 * The SGPIO Input bit match interrupt's status register has a bit set for the
 * slice (A-P) that caused the interrupt. This function does the translation
 * like this:
 *
 *     Interrupt Mask 0x00000008 => bit 3 is set => slice D => DIO_9
 *
 * @param [in] interruptMask  Value to translate
 *
 * @return The digital input's number or DIO_UNAVAIL if invalid
 *
 *****************************************************************************/
dio_t sgpio_cfg_GetDioForSliceInterrupt(uint32_t interruptMask)
{
  int i = 0;
  if (interruptMask == 0)
  {
    return DIO_UNAVAIL;
  }
  for (i = 0; (interruptMask & 1)==0; i++)
  {
    interruptMask = interruptMask>>1;
  }
  return DIO_FROM_SLICE[i];
}
