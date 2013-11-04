/*!
 * @file
 * @brief   Handles analog signal generation using a DAC on the SPI bus
 * @ingroup IP_DAC
 * @ingroup FUNC_GEN
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

#include "lpc43xx_scu.h"
#include "lpc43xx_cgu.h"
#include "lpc43xx_rgu.h"
#include "lpc43xx_timer.h"

#include "led.h"
#include "log.h"
#include "generator_dac.h"
#include "meas.h"

#include <string.h>
#include "math.h"

#include "spi_dac.h"
#include "calibrate.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Size of lookup table for waveform data */
#define MAX_LUT_SIZE  2000

/*! Smallest allowed LUT size */
#define MIN_LUT_SIZE (MAX_DAC_FREQ / MAX_FREQ)

/*! Lowest supported frequency (in Hz) on generated signal */
#define MIN_FREQ  (1)

/*! Highest supported frequency (in Hz) on generated signal */
#define MAX_FREQ  (50000)

/*! Highest supported DAC update frequency (in Hz).
 * This value is controlled by:
 *
 *    SSP Bus Speed / Bits to send
 *
 * The SSP Bus is initialized to 20MHz in \ref spi_dac.c and the
 * number of bits to send is always 16. With 20/16 it should be
 * possible to update the DAC at 1.25MHz but there are other
 * delays causing the \ref spi_dac_write() call to take ca 2us
 * to complete, reducing the DAC update rate to a maximum of
 * 500KHz.
 *
 * The value can be further tweeked but for now it's set to
 * 300KHz to give some time for other tasks (e.g. USB stack).
 *
 * When using two channels the update frequency will be divided
 * between the channels - i.e. half each.
 */
#define MAX_DAC_FREQ  (300000)

/*! Lowest supported amplitude (in mV) on generated signal */
#define MIN_AMPLITUDE  (-5000)

/*! Highest supported amplitude (in mV) on generated signal */
#define MAX_AMPLITUDE  (5000)

/*! Absolute value of difference between \a __a and \a __b. */
#define ABS_DIFF(__a, __b)  (((__a) > (__b)) ? ((__a) - (__b)) : ((__b) - (__a)))

/*! Number of supported channels */
#define MAX_SUPPORTED_CHANNELS  (2)

/*! @brief Configuration of one analog output.
 */
typedef struct
{
  /*! TRUE if this channel is in use */
  Bool enabled;

  /*! Timer that the channel uses for DAC updates */
  LPC_TIMERn_Type* timer;

  /*! Timer interrupt that the channel uses for DAC updates */
  IRQn_Type timerIRQ;

  /*! Lookup table for waveform data */
  uint16_t LUT_BUFFER[MAX_LUT_SIZE];

  /*! Current number of entries in the lookup table */
  uint16_t numLUTEntries;

  /*! Current index in the lookup table */
  uint16_t idxLUT;

  /*! Calibration parameter A */
  double calib_a;

  /*! Calibration parameter B */
  double calib_b;

} dac_setup_t;

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

/*! Configurations for the supported channels */
static dac_setup_t channels[MAX_SUPPORTED_CHANNELS] = {
  {
    .enabled = FALSE,
    .timer = LPC_TIMER1,
    .timerIRQ = TIMER1_IRQn,
  },
  {
    .enabled = FALSE,
    .timer = LPC_TIMER3,
    .timerIRQ = TIMER3_IRQn,
  },
};

/*! TRUE if the generator has been configured and is ready to start */
static Bool validConfiguration = FALSE;

/*! String representation of the GEN_DAC_CFG_WAVE_* defines in generator_dac.h */
static const char* const WAVEFORMS[6] = { "Sinus", "Square", "Triangular", "Sawtooth", "Inv Sawtooth", "Level" };

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  TIMER1 IRQ Handler.
 * @ingroup RES_IRQ
 *
 * Sends the next value from the lookup table to the DAC.
 *
 *****************************************************************************/
void TIMER1_IRQHandler(void)
{
  SET_MEAS_PIN_3();

  // Clear the pending interrupt (MR1 is bit 1)
  LPC_TIMER1->IR = (1<<1);

  // Send the new value to the DAC
  spi_dac_write(channels[0].LUT_BUFFER[channels[0].idxLUT]);

  // Find next value to send
  channels[0].idxLUT++;
  if (channels[0].idxLUT >= channels[0].numLUTEntries)
  {
    channels[0].idxLUT = 0;
  }

  CLR_MEAS_PIN_3();
}

/**************************************************************************//**
 *
 * @brief  TIMER3 IRQ Handler.
 * @ingroup RES_IRQ
 *
 * Sends the next value from the lookup table to the DAC.
 *
 *****************************************************************************/
void TIMER3_IRQHandler(void)
{
  SET_MEAS_PIN_3();

  // Clear the pending interrupt (MR1 is bit 1)
  LPC_TIMER3->IR = (1<<1);

  // Send the new value to the DAC
  spi_dac_write(channels[1].LUT_BUFFER[channels[1].idxLUT]);

  // Find next value to send
  channels[1].idxLUT++;
  if (channels[1].idxLUT >= channels[1].numLUTEntries)
  {
    channels[1].idxLUT = 0;
  }

  CLR_MEAS_PIN_3();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Finds a set of parameters to make up the wanted frequency.
 *
 * The frequency of the generated analog signal can be controlled by two
 * parameters:
 * - Timer prescale value. This controlls the rate that samples are sent to
 *   the DAC and it is limited to \ref MAX_DAC_FREQ
 * - Size of lookup table (LUT). A larger table means a finer granularity of
 *   the generated signal but it increases the number of values to send to the
 *   DAC each period.
 *
 * Example 1: Requested frequency is 50kHz, Timer clock 200MHz
 *
 * LUT size of 20 and a prescale value of 200 results in a DAC update rate
 * of 200MHz/200 = 1MHz and with 20 entries it will be a 1MHz/20 = 50kHz
 * generated signal. Only 20 values is quite low, but at 50kHz which is the
 * highest supported it is not possible to have a higher LUT size as that would
 * require a higher DAC update rate which in turn is not possible.
 *
 * Example 2: Requested frequency is 500Hz, Timer clock 200MHz
 *
 * LUT size of 20 and a prescale value of 20000 results in a DAC update rate
 * of 200MHz/20000 = 10kHz and with 20 entries it will be a 10kHz/20 = 500Hz
 * generated signal. The signal will only consist of 20 values which will give
 * it a coarse look.
 *
 * LUT size of 400 and a prescale value of 1000 results in a DAC update rate
 * of 200MHz/1000 = 200kHz and with 400 entries it will be a 200kHz/400 = 500Hz
 * generated signal. The signal will have the same frequency as the 20-LUT one
 * above, but with 20 times more values it will be a much smoother signal.
 *
 * @param [in] frequency        The wanted frequency in Hz
 * @param [in] numChannels      The number of enabled channels
 * @param [out] pLUTSize        The number of entries in the lookup table
 * @param [out] pPrescaleValue  The timer's prescale value
 *
 * @retval CMD_STATUS_OK                         If the values were found
 * @retval CMD_STATUS_ERR_GEN_INVALID_FREQUENCY  If the frequency is impossible
 *
 *****************************************************************************/
static cmd_status_t gen_dac_FindFrequency(uint32_t frequency,
                                          uint32_t numChannels,
                                          uint32_t* pLUTSize,
                                          uint32_t* pPrescaleValue)
{
  uint32_t lut;
  uint32_t pre;
  uint32_t err;
  uint32_t min_err = 0xffffffff;
  uint32_t pclk = CGU_GetPCLKFrequency(CGU_PERIPHERAL_TIMER1);
  uint32_t min_lut_size = MIN_LUT_SIZE / numChannels;
  uint32_t max_dac_freq = MAX_DAC_FREQ / numChannels;

  for (lut = MAX_LUT_SIZE; (lut >= min_lut_size) && (min_err > 0); lut--)
  {
    pre = pclk/(lut * frequency); // 200MHz/(20 * 50KHz)
    if ((pclk/pre) > max_dac_freq)
    {
      pre++; // rounding error
      if ((pclk/pre) > max_dac_freq)
      {
        continue; // too high output frequency to the DAC
      }
    }

    err = (pclk/pre)/lut;
    err = ABS_DIFF(err, frequency);
    if (err < min_err)
    {
      min_err = err;
      *pLUTSize = lut;
      *pPrescaleValue = pre;
    }
  }
  if (min_err == 0xffffffff)
  {
    return CMD_STATUS_ERR_GEN_INVALID_FREQUENCY;
  }
  log_i("Configured for %dHz as LUT size %d, prescale %d, PCLK %dMHz\r\n", frequency, *pLUTSize, *pPrescaleValue, pclk/1000000);
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Fills the lookup table with data for the requested waveform.
 *
 * Generates the data for the lookup table based on waveform and amplitude.
 *
 * @param [in]  cfg      Configuration for the waveform (from client)
 * @param [in]  lutSize  Number of entries to use in the lookup table
 * @param [in]  ch       The configuration to update (local)
 *
 * @retval CMD_STATUS_OK                        If the lookup table was filled
 * @retval CMD_STATUS_ERR_GEN_INVALID_WAVEFORM  If the waveform is unknown
 *
 *****************************************************************************/
static cmd_status_t gen_dac_SetupLUT(const gen_dac_one_ch_cfg_t * const cfg, uint32_t lutSize, int ch)
{
  int i;
  int val;
  float dcOffset = cfg->dcOffset / 1000.0;
  float amplitude = cfg->amplitude / 1000.0;
  dac_setup_t* dacSetup = &(channels[ch]);

  if (cfg->waveform == GEN_DAC_CFG_WAVE_SINUS)
  {
    for (i = 0; i < lutSize; i++)
    {
      float deg = (i * 360.0)/lutSize;
      float rad = (deg * 3.141519) / 180.0;
      float sin = sinf(rad);

      // change amplitude
      sin = sin * amplitude;//cfg->amplitude / MAX_AMPLITUDE;

      // apply calibration
      val = (sin + dcOffset - channels[ch].calib_a) / channels[ch].calib_b;

      // move the 10 value bits into the upper 10-bits of a 12-bit value
      val = val << 2;

      dacSetup->LUT_BUFFER[i] = SPI_DAC_VALUE(ch, val);
    }
  }
  else if (cfg->waveform == GEN_DAC_CFG_WAVE_TRIANGLE)
  {
    /*
     * Based on http://en.wikipedia.org/wiki/Triangle_wave
     * and the fact that the triangle wave can be the absolute
     * value of the sawtooth wave.
     *
     * x(t) = ABS( 2 * (t/a - FLOOR( t/a - 1/2 )) )
     *
     * with
     *   t = 0..1,
     *   a = num periods (always 1)
     *   x = 0..1..0
     */
    for (i = 0; i < lutSize; i++)
    {
      float t = i/((float)lutSize);
      float a = 1;
      float x = fabs(2 * (t/a - floorf(t/a + 0.5)));

      // move from 0..1 to -1..1 and change amplitude
      x = (x - 0.5) * 2 * amplitude;

      // apply calibration
      val = (x + dcOffset - channels[ch].calib_a) / channels[ch].calib_b;

      // move the 10 value bits into the upper 10-bits of a 12-bit value
      val = val << 2;

      dacSetup->LUT_BUFFER[i] = SPI_DAC_VALUE(ch, val);
    }
  }
  else if (cfg->waveform == GEN_DAC_CFG_WAVE_SQUARE)
  {
    // Calculate the square wave's high point
    float tmp = (dcOffset + amplitude);

    // apply calibration
    val = (tmp - channels[ch].calib_a) / channels[ch].calib_b;

    // move the 10 value bits into the upper 10-bits of a 12-bit value
    val = val << 2;

    for (i = 0; i < lutSize/2; i++)
    {
      dacSetup->LUT_BUFFER[i] = SPI_DAC_VALUE(ch, val);
    }

    // Calculate the square wave's low point
    tmp = (dcOffset - amplitude);

    // apply calibration
    val = (tmp - channels[ch].calib_a) / channels[ch].calib_b;

    // move the 10 value bits into the upper 10-bits of a 12-bit value
    val = val << 2;

    for (; i < lutSize; i++)
    {
      dacSetup->LUT_BUFFER[i] = SPI_DAC_VALUE(ch, val);
    }
  }
  else if ((cfg->waveform == GEN_DAC_CFG_WAVE_SAWTOOTH) ||
           (cfg->waveform == GEN_DAC_CFG_WAVE_INV_SAWTOOTH))
  {
    int mul = 1;
    if (cfg->waveform == GEN_DAC_CFG_WAVE_INV_SAWTOOTH)
    {
      mul = -1;
    }

    /*
     * Based on http://en.wikipedia.org/wiki/Sawtooth_wave
     *
     * x(t) = 2 * (t/a - FLOOR( t/a + 1/2 ))
     *
     * with
     *   t = 0..1,
     *   a = num periods (always 1)
     *   x = 0..1..-1..0
     */
    for (i = 0; i < lutSize; i++)
    {
      float t = i/((float)lutSize);
      float a = 1;
      float x = 2 * (t/a - floorf(t/a + 0.5));

      // change amplitude, and correct if it is an inverse sawtooth
      x = x * mul * amplitude;

      // apply calibration
      val = (x - channels[ch].calib_a) / channels[ch].calib_b;

      // move the 10 value bits into the upper 10-bits of a 12-bit value
      val = val << 2;

      dacSetup->LUT_BUFFER[i] = SPI_DAC_VALUE(ch, val);
    }
  }
  else if (cfg->waveform == GEN_DAC_CFG_WAVE_LEVEL)
  {
    // Find actual amplitude, which for a level type is based on offset

    // apply calibration
    val = (dcOffset - channels[ch].calib_a) / channels[ch].calib_b;

    // move the 10 value bits into the upper 10-bits of a 12-bit value
    val = val << 2;

    dacSetup->LUT_BUFFER[0] = SPI_DAC_VALUE(ch, val);
  }
  else
  {
    return CMD_STATUS_ERR_GEN_INVALID_WAVEFORM;
  }
  dacSetup->numLUTEntries = lutSize;

//   for (i = 0; i < lutSize; i++)
//   {
//     log_i("%d\t%#x\t%d\r\n", i, dacSetup->LUT_BUFFER[i], ((dacSetup->LUT_BUFFER[i]>>2)&0x3ff));
//     TIM_Waitms(3);
//   }
  log_i("LUT with %u entries for %dmV + %umV amplitude %s waveform\r\n", dacSetup->numLUTEntries, cfg->dcOffset, cfg->amplitude, WAVEFORMS[cfg->waveform]);

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Configures the timer with the parameters from \ref gen_dac_FindFrequency
 *
 * @param [in] prescale  Configuration for the waveform
 * @param [in] tim       One of LPC_TIMER0 .. LPC_TIMER3
 *
 *****************************************************************************/
static void gen_dac_SetupTimer(uint32_t prescale, LPC_TIMERn_Type* tim)
{
  TIM_TIMERCFG_Type timerCfg;
  TIM_MATCHCFG_Type matchCfg;

  // Initialize timer 1, prescale count in ticks
  timerCfg.PrescaleOption = TIM_PRESCALE_TICKVAL;
  timerCfg.PrescaleValue  = 1;

  // Use channel 1, MR1
  matchCfg.MatchChannel = 1;
  // Enable interrupt when MR1 matches the value in TC register
  matchCfg.IntOnMatch   = TRUE;
  // Enable reset on MR1: TIMER will reset if MR1 matches it
  matchCfg.ResetOnMatch = TRUE;
  // Stop on MR1 if MR1 matches it
  matchCfg.StopOnMatch  = FALSE;
  // Don't toggle MR1.1 pin if MR1 matches it
  matchCfg.ExtMatchOutputType = TIM_EXTMATCH_NOTHING;
  // Set Match value, should be N-1
  matchCfg.MatchValue   = prescale - 1;

  TIM_Init(tim, TIM_TIMER_MODE, &timerCfg);
  TIM_ConfigMatch(tim, &matchCfg);
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes the DAC
 *
 *****************************************************************************/
void gen_dac_Init(void)
{
  spi_dac_init();

  // Get current calibration data
  calibrate_GetFactorsForDAC(0, &channels[0].calib_a, &channels[0].calib_b);
  calibrate_GetFactorsForDAC(1, &channels[1].calib_a, &channels[1].calib_b);
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * The "force trigger mode" means that no trigger is used and instead the entire
 * capture buffer should be filled and then returned to the client.
 *
 * @param [in] cfg               Configuration to apply
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t gen_dac_Configure(const gen_dac_cfg_t * const cfg)
{
  cmd_status_t result;
  uint32_t lutSize = 0;
  uint32_t prescaler = 0;
  int i;
  int numChannels;
  validConfiguration = FALSE;

  gen_dac_Stop();
  channels[0].enabled = FALSE;
  channels[1].enabled = FALSE;

  do
  {
    i = cfg->available & ((1<<MAX_SUPPORTED_CHANNELS) - 1);
    if (i == 0 || i > 3)
    {
      result = CMD_STATUS_ERR_NOTHING_TO_GENERATE;
      break;
    }
    else if (i == 3)
    {
      numChannels = 2;
    }
    else
    {
      numChannels = 1;
    }

    for (i = 0; i < MAX_SUPPORTED_CHANNELS; i++)
    {
      if (cfg->available & (1<<i))
      {
        if ((cfg->ch[i].frequency < MIN_FREQ) || (cfg->ch[i].frequency > MAX_FREQ))
        {
          result = CMD_STATUS_ERR_GEN_INVALID_FREQUENCY;
          break;
        }

        if (((cfg->ch[i].dcOffset - ((int)cfg->ch[i].amplitude)) < MIN_AMPLITUDE) ||
            ((cfg->ch[i].dcOffset + ((int)cfg->ch[i].amplitude)) > MAX_AMPLITUDE))
        {
          result = CMD_STATUS_ERR_GEN_INVALID_AMPLITUDE;
          break;
        }

        // The level waveform does not change (by definition) so there is
        // no need for timers and the lookup table only have to have one value
        if (cfg->ch[i].waveform == GEN_DAC_CFG_WAVE_LEVEL)
        {
          result = gen_dac_SetupLUT(&(cfg->ch[i]), 1, i);
          if (result != CMD_STATUS_OK)
          {
            break;
          }
        }
        else
        {
          result = gen_dac_FindFrequency(cfg->ch[i].frequency, numChannels, &lutSize, &prescaler);
          if (result != CMD_STATUS_OK)
          {
            break;
          }

          result = gen_dac_SetupLUT(&(cfg->ch[i]), lutSize, i);
          if (result != CMD_STATUS_OK)
          {
            break;
          }

          gen_dac_SetupTimer(prescaler, channels[i].timer);
        }

        channels[i].enabled = TRUE;
      }
    }

    validConfiguration = TRUE;

    result = CMD_STATUS_OK;
  } while (FALSE);

  return result;
}

/**************************************************************************//**
 *
 * @brief  Starts the signal generation.
 *
 * @retval CMD_STATUS_OK      If successfully started
 * @retval CMD_STATUS_ERR_*   If the generation could not be started
 *
 *****************************************************************************/
cmd_status_t gen_dac_Start(void)
{
  int i;
  if (!validConfiguration)
  {
    return CMD_STATUS_ERR_NOTHING_TO_GENERATE;
  }

  CLR_MEAS_PIN_3();

  for (i = 0; i < MAX_SUPPORTED_CHANNELS; i++)
  {
    if (channels[i].enabled)
    {
      // The level waveform does not change (by definition) so there is
      // no need for timers or lookup tables
      if (channels[i].numLUTEntries == 1)
      {
        spi_dac_write(SPI_DAC_VALUE(i, channels[i].LUT_BUFFER[0]));
      }
      else
      {
        channels[i].idxLUT = 0;
        NVIC_EnableIRQ(channels[i].timerIRQ);
        TIM_Cmd(channels[i].timer, ENABLE);
      }
    }
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Disarms (stops) the signal generation.
 *
 *****************************************************************************/
void gen_dac_Stop(void)
{
  int i;
  for (i = 0; i < MAX_SUPPORTED_CHANNELS; i++)
  {
    NVIC_DisableIRQ(channels[i].timerIRQ);
    TIM_Cmd(channels[i].timer, DISABLE);
    TIM_ClearIntPending(channels[i].timer, TIM_MR1_INT);
  }

  spi_dac_stop();
}
