/*!
 * @file
 * @brief   Handles setup shared by analog and digital signal capturing
 * @ingroup FUNC_CAP
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
#include "lpc43xx_cgu_improved.h"
#include "lpc43xx_rgu.h"
#include "lpc43xx_gpio.h"
#include "lpc43xx_gpdma.h"

#include "log.h"
#include "led.h"
#include "capture.h"
#include "capture_sgpio.h"
#include "capture_vadc.h"
#include "usb_handler.h"
#include "statemachine.h"
#include "sgpio_cfg.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Initial sample rate - 2MHz. Index is in the \ref RATECONFIG table. */
#define INITIAL_SAMPLE_RATE_IDX  14

/*! Offset in the \ref RATECONFIG table to where the SGPIO only value start. */
#define SGPIO_ONLY_OFFSET  25

/*! @brief Configuration for one sample rate. Used in the \ref RATECONFIG table. */
typedef struct
{
  uint32_t sampleRate;       /*!< Wanted sample rate */

  uint8_t  pll0_msel;        /*!< PLL0AUDIO multiplier */
  uint8_t  pll0_nsel;        /*!< PLL0AUDIO pre-divider */
  uint8_t  pll0_psel;        /*!< PLL0AUDIO post-divider */

  uint16_t counter;          /*!< Counter for SGPIO, Match for VADC */

  uint32_t pll0_freq;        /*!< Actual output of PLL0AUDIO */
} sample_rate_cfg_t;

/*! @brief Configuration for signal capture.
 * This is the structure that the client software must send to configure
 * capture of analog and/or digital signals.
 */
typedef struct
{
  uint32_t         numEnabledSGPIO; /*!< Number of enabled digital signals */
  uint32_t         numEnabledVADC;  /*!< Number of enabled analog signals */
  uint32_t         sampleRate;      /*!< Wanted sample rate */

  /*! Post fill configuration. The lower 8 bits specify the percent of the
   * maximum buffer size that will be used for samples taken AFTER the trigger.
   * The upper 24 bits specifies the maximum number of samples to gather after
   * a trigger has been found.*/
  uint32_t         postFill;

  cap_sgpio_cfg_t  sgpio;  /*!< Configuration of digital signals */
  cap_vadc_cfg_t   vadc;   /*!< Configuration of analog signals */
} capture_cfg_t;


#define NUM_BUFFER_CONFIGURATIONS  (sizeof(BUFFERCONFIG)/sizeof(buffer_size_cfg_t))

/*! @brief Configuration for one capture buffer setup. Used in \ref BUFFERCONFIG */
typedef struct
{
  uint8_t  numVADC;         /*!< Number of enabled analog signals */
  uint8_t  numDIO;          /*!< Number of enabled digital signals */
  uint32_t buffEndSGPIO;    /*!< End of address space for digital signal */
  uint32_t buffStartVADC;   /*!< Start of address space for analog signal */
} buffer_size_cfg_t;

/******************************************************************************
 * Global variables
 *****************************************************************************/

volatile uint8_t capture_PrefillComplete__;

/******************************************************************************
 * Local variables
 *****************************************************************************/

/*!
 * Lookup table for configuration of the PLL0AUDIO and SGPIO/VADC counters
 * based on wanted sample rate.
 */
static const sample_rate_cfg_t RATECONFIG[] =
{
//              PLL0AUDIO Cfg    SGPIO/VADC   PLL out
//              --------------   ----------   ---------
//    Wanted     M     N     P      Counter       fADC
  {        50,  100,  250,  24,      4000,        200000 },
  {       100,  100,  250,  12,      4000,        400000 },
  {       200,  100,  250,   6,      4000,        800000 },
  {       500,  100,  200,   3,      4000,       2000000 },
  {      1000,  100,  150,   2,      4000,       4000000 },
  {      2000,  100,  150,   1,      4000,       8000000 },
  {      5000,  100,   60,   1,      4000,      20000000 },
  {     10000,  100,   30,   1,      4000,      40000000 },
  {     20000,  100,   15,   1,      4000,      80000000 },
  {     50000,  100,   15,   1,      1600,      80000000 },
  {    100000,  100,   15,   1,       800,      80000000 },
  {    200000,  100,   15,   1,       400,      80000000 },
  {    500000,  100,   15,   1,       160,      80000000 },
  {   1000000,  100,   15,   1,        80,      80000000 },
  {   2000000,  100,   15,   1,        40,      80000000 },  // <-- INITIAL_SAMPLE_RATE_IDX
  {   5000000,  100,   15,   1,        16,      80000000 },
  {  10000000,  100,   15,   1,         8,      80000000 },
  {  20000000,  100,   15,   1,         4,      80000000 },
  {  30000000,  100,   20,   1,         2,      60000000 },
  {  40000000,  100,   15,   1,         2,      80000000 },
  {  50000000,  100,   24,   1,         1,      50000000 },
  {  60000000,  100,   20,   1,         1,      60000000 },
  {  70000000,   70,   12,   1,         1,      70000000 },
  {  80000000,  100,   15,   1,         1,      80000000 },
  {         0,    0,    0,   0,         0,             0 },
  {  10000000,   50,    3,   1,        20,     200000000 },  // <-- SGPIO_ONLY_OFFSET
  {  20000000,   50,    3,   1,        10,     200000000 },
  {  30000000,   15,    1,   1,         6,     180000000 },
  {  40000000,   50,    3,   1,         5,     200000000 },
  {  50000000,   50,    3,   1,         4,     200000000 },
  {  60000000,   15,    1,   1,         3,     180000000 },
  {  70000000,   70,    4,   1,         3,     210000000 },
  {  80000000,   20,    1,   1,         3,     240000000 },
  {  90000000,   15,    1,   1,         2,     180000000 },
  { 100000000,   50,    3,   1,         2,     200000000 },
  {         0,    0,    0,   0,         0,             0 },
};

/*!
 * The table is based on the fact that the SGPIO capturing needs to copy all DIOx values
 * up to and including the highest enabled DIOx. Concatenation of SGPIO data introduces
 * further limitations.
 *
 * For analog signals only the enabled ones are copied.
 *
 * With only digital signals (or only analog signals) the entire buffer is used. This
 * table only deals with the case where a combination of analog and digital signals
 * is selected.
 */
static const buffer_size_cfg_t BUFFERCONFIG[] =
{
// Analog    Digital     End of digital    Start of analog
// Channels  Channels    buffer            buffer
// --------  --------    --------------    ---------------
  {   1,        1,         0x20001C00,       0x20002000 },
  {   1,        2,         0x20001C00,       0x20002000 },
  {   1,        3,         0x20003300,       0x20003400 },
  {   1,        4,         0x20003300,       0x20003400 },
  {   1,        5,         0x20005400,       0x20005800 },
  {   1,        6,         0x20005400,       0x20005800 },
  {   1,        7,         0x20005400,       0x20005800 },
  {   1,        8,         0x20005400,       0x20005800 },
  {   1,        9,         0x20005A00,       0x20006000 },
  {   1,       10,         0x20006180,       0x20006400 },
  {   1,       11,         0x200065C0,       0x20006C00 },
  {   2,        1,         0x20000F00,       0x20001000 },
  {   2,        2,         0x20000F00,       0x20001000 },
  {   2,        3,         0x20001C00,       0x20002000 },
  {   2,        4,         0x20001C00,       0x20002000 },
  {   2,        5,         0x20003200,       0x20003800 },
  {   2,        6,         0x20003200,       0x20003800 },
  {   2,        7,         0x20003200,       0x20003800 },
  {   2,        8,         0x20003200,       0x20003800 },
  {   2,        9,         0x20003600,       0x20004000 },
  {   2,       10,         0x20003C00,       0x20004000 },
  {   2,       11,         0x20003F40,       0x20004800 },
};

static circbuff_t sampleBufferSGPIO;
static circbuff_t sampleBufferVADC;
static int        enabledSgpioChannels = 0;
static int        enabledVadcChannels = 0;

static int currentSampleRateIdx = -1;

static captured_samples_t capturedSamples;

static capture_cfg_t calibrationSetup;

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
 * @brief  Set the default sample rate
 *
 *****************************************************************************/
static void capture_SetInitialSampleRate(void)
{
  /*
   * Both SGPIO and VADC use the PLL0AUDIO without additional integer
   * dividers.
   */

  CGU_Improved_SetPLL0audio(RATECONFIG[INITIAL_SAMPLE_RATE_IDX].pll0_msel,
                            RATECONFIG[INITIAL_SAMPLE_RATE_IDX].pll0_nsel,
                            RATECONFIG[INITIAL_SAMPLE_RATE_IDX].pll0_psel);
  currentSampleRateIdx = INITIAL_SAMPLE_RATE_IDX;

  CGU_UpdateClock();

  log_d("Set initial sample rate to %d", RATECONFIG[currentSampleRateIdx].sampleRate);
}

/**************************************************************************//**
 *
 * @brief  Returns the index for the wanted rate in the \ref RATECONFIG table
 *
 * @param [in] wantedRate  The wanted sample rate
 * @param [in] numVADC     Number of enabled analog channels
 *
 * @return The index in the \ref RATECONFIG table or -1 if \a wantedRate is invalid
 *
 *****************************************************************************/
static int capture_FindSampleRateIndex(uint32_t wantedRate, int numVADC)
{
  int i;
  if (numVADC == 0)
  {
    for (i = SGPIO_ONLY_OFFSET; RATECONFIG[i].sampleRate > 0; i++)
    {
      if (RATECONFIG[i].sampleRate == wantedRate)
      {
        return i;
      }
    }
  }
  for (i = 0; RATECONFIG[i].sampleRate > 0; i++)
  {
    if (RATECONFIG[i].sampleRate == wantedRate)
    {
      return i;
    }
  }

  return -1;
}

/**************************************************************************//**
 *
 * @brief  Attempt to set the wanted sample rate
 *
 * @param [in] wantedRate  The wanted sample rate
 * @param [in] numVADC     Number of enabled analog channels
 *
 * @retval CMD_STATUS_OK                            If successful
 * @retval CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE   When the sample rate could not be changed
 *
 *****************************************************************************/
static cmd_status_t capture_SetSampleRate(uint32_t wantedRate, int numVADC)
{
  static int lastNumVADC = -1;
  uint32_t oldSampleRate = RATECONFIG[currentSampleRateIdx].sampleRate;

  if ((wantedRate != oldSampleRate) || (lastNumVADC != numVADC))
  {
    int i = capture_FindSampleRateIndex(wantedRate, numVADC);
    if (i != -1)
    {
      if ((numVADC == 2) && (RATECONFIG[i].counter == 1))
      {
        // with 2 analog channels the sample rate must be doubled which
        // is not possible when the counter value is supposed to be 1
        // (that is we cannot have a counter value of 0.5)
        return CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE;
      }

      // Found the wanted rate, now disable the clocks that uses PLL0AUDIO
      CGU_EnableEntity(CGU_BASE_PERIPH, DISABLE);
      CGU_EnableEntity(CGU_BASE_VADC, DISABLE);

      // Change PLL0AUDIO
      CGU_Improved_SetPLL0audio(RATECONFIG[i].pll0_msel,
                                RATECONFIG[i].pll0_nsel,
                                RATECONFIG[i].pll0_psel);
      currentSampleRateIdx = i;

      CGU_UpdateClock();

      // Re-enable the clocks that uses PLL0AUDIO
      CGU_EnableEntity(CGU_BASE_PERIPH, ENABLE);
      CGU_EnableEntity(CGU_BASE_VADC, ENABLE);

      log_d("Changed from %u to %u sample rate", oldSampleRate, wantedRate);

      lastNumVADC = numVADC;

      return CMD_STATUS_OK;
    }
    log_i("Failed to change sample rate to %u. Keeping it at %u\r\n", wantedRate, oldSampleRate);
    return CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE;
  }

  /* no change */
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Configures the capture buffers to be optimally used.
 *
 * When only analog or only digital signals are enabled then the entire
 * available address space (0x20000000 - 0x20010000) is used as one buffer.
 *
 * When a combination of analog and digital signals is selected then two separate
 * buffers will be created and the size of those buffers are adjusted so that
 * the same number of samples will fit in each buffer. The BUFFERCONFIG
 * table is used for this.
 *
 * @param [in] cap_cfg  Configuration from client
 *
 * @retval CMD_STATUS_OK                                  If successful
 * @retval CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION  When the configuration could not be applied
 *
 *****************************************************************************/
static cmd_status_t capture_ConfigureCaptureBuffers(const capture_cfg_t * const cap_cfg)
{
  if (cap_cfg->numEnabledVADC == 0)
  {
    // Only digital capture
    circbuff_Init(&sampleBufferSGPIO, 0x20000000, 0x10000);
  }
  else if (cap_cfg->numEnabledSGPIO == 0)
  {
    // Only analog capture
    circbuff_Init(&sampleBufferVADC, 0x20000000, 0x10000);
  }
  else
  {
    int i;
    int numDIO = -1;

    // BUFFERCONFIG table is based on how many digital signals are copied and that
    // is determined by the highest enabled DIOx. E.g. with DIO0 and DIO5 enabled
    // 6 (DIO0..DIO5) signals will be copied even if only two of them are actually
    // sampled
    for (i = MAX_NUM_DIOS; i >= 0; i--)
    {
      if (cap_cfg->sgpio.enabledChannels & (1<<i))
      {
        // highest enabled is DIOi
        numDIO = i + 1;
        break;
      }
    }
    for (i = 0; i < NUM_BUFFER_CONFIGURATIONS; i++)
    {
      if ((BUFFERCONFIG[i].numVADC == cap_cfg->numEnabledVADC) &&
          (BUFFERCONFIG[i].numDIO == numDIO))
      {
        // When sampling both SGPIO and VADC at the same rate, VADC will
        // need sixteen times the memory.
        // It is important that VADC buffer ends at memory boundary 0x20010000
        // so a unused zone is added between the buffers.
        circbuff_Init(&sampleBufferSGPIO, 0x20000000, BUFFERCONFIG[i].buffEndSGPIO - 0x20000000);
        circbuff_Init(&sampleBufferVADC,  BUFFERCONFIG[i].buffStartVADC, 0x20010000 - BUFFERCONFIG[i].buffStartVADC);
        return CMD_STATUS_OK;
      }
    }
    return CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Checks for combinations of captured signals that may cause problems
 *
 * @param [in] cap_cfg  Configuration from client
 *
 * @retval CMD_STATUS_OK                                  If successful
 * @retval CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION  When the configuration could not be applied
 *
 *****************************************************************************/
static cmd_status_t capture_WeightedConfigCheck(const capture_cfg_t * const cap_cfg)
{
#if (DO_WEIGHTED_CONFIG_CHECK == OPT_ENABLED)
  cmd_status_t result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;

  do
  {
    if (cap_cfg->sampleRate < 20000)
    {
      // Sample rates below 20KHz are not correctly setup in the PLL0AUDIO
      // (it can take >10s to get it to lock). Better to restrict this.
      result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
      break;
    }

    if (cap_cfg->numEnabledVADC == 0)
    {
      // Only digital capture

      uint16_t tmp = cap_cfg->sgpio.enabledChannels & 0x7ff;
      if (tmp > 0x0ff)
      {
        if (cap_cfg->sampleRate > 20000000)
        {
          // limit the sample rate to 20MHz when sampling all digital signals
          result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
          break;
        }
      }
      else if (tmp > 0x00f)
      {
        if (cap_cfg->sampleRate > 50000000)
        {
          // limit the sample rate to 50MHz when sampling DIO0..DIO7
          result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
          break;
        }
        if ((cap_cfg->sampleRate > 40000000) && (cap_cfg->sgpio.enabledTriggers & 0x7ff))
        {
          // limit the sample rate to 40MHz when sampling DIO0..DIO7 with triggers
          result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
          break;
        }
      }
      else if (tmp > 0x003)
      {
        if ((cap_cfg->sampleRate > 80000000) && (cap_cfg->sgpio.enabledTriggers & 0x7ff))
        {
          // limit the sample rate to 80MHz when sampling DIO0..DIO3 with triggers
          result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
          break;
        }
      }

      result = CMD_STATUS_OK;
      break;
    }
    if (cap_cfg->numEnabledSGPIO == 0)
    {
      // Only analog capture
      if (cap_cfg->sampleRate > 60000000)
      {
        // limit the sample rate to 60MHz when sampling both analog signals
        result = CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE;
        break;
      }
      if ((cap_cfg->sampleRate > 30000000) && (cap_cfg->numEnabledVADC == 2))
      {
        // limit the sample rate to 30MHz when sampling both analog signals
        result = CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE;
        break;
      }
      result = CMD_STATUS_OK;
      break;
    }

    // At this point we have at least one analog and one digital enabled

    if (cap_cfg->sampleRate > 20000000)
    {
      // limit the sample rate to 20MHz when sampling both analog and digital signals
      result = CMD_STATUS_ERR_CFG_INVALID_SIGNAL_COMBINATION;
      break;
    }

    result = CMD_STATUS_OK;
  }
  while (FALSE);

  return result;
#else  
  // Validation is disabled - accept everything!
  return CMD_STATUS_OK;
#endif  
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes capture of both analog and digital signals.
 *
 *****************************************************************************/
void capture_Init(void)
{
  LED_ARM_OFF();
  LED_TRIG_OFF();

  circbuff_Init(&sampleBufferSGPIO, 0x20000000, 0x10000);
  circbuff_Init(&sampleBufferVADC, 0x20000000, 0x10000);

  capture_SetInitialSampleRate();

  memset(&capturedSamples, 0, sizeof(captured_samples_t));

  /*! @todo Move the controls for the DIO direction to a central place as it will prevent any signal generation */
  LPC_GPIO_PORT->CLR[1] |= (1UL <<  8);
  LPC_GPIO_PORT->SET[0] |= (1UL << 14);
  LPC_GPIO_PORT->CLR[1] |= (1UL << 11);

  cap_sgpio_Init();
  cap_vadc_Init();
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * @param [in] cfg   Configuration from client (must be of type capture_cfg_t)
 * @param [in] size  Size of configuration from client
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t capture_Configure(uint8_t* cfg, uint32_t size)
{
  capture_cfg_t* cap_cfg = (capture_cfg_t*)cfg;
  cmd_status_t result;
  Bool forcedTrigger = TRUE;

  result = statemachine_RequestState(STATE_CAPTURING);
  if (result != CMD_STATUS_OK)
  {
    return result;
  }

  LED_ARM_OFF();
  LED_TRIG_OFF();

  // disable all channels until configuration is done
  enabledSgpioChannels = 0;
  enabledVadcChannels = 0;

  // if neither a digital nor a analog signal has been selected as trigger then
  // enter forced trigger mode (i.e. capture as much as the buffer can hold)
  if (((cap_cfg->numEnabledSGPIO > 0) && (cap_cfg->sgpio.enabledTriggers > 0)) ||
      ((cap_cfg->numEnabledVADC > 0) && (cap_cfg->vadc.enabledTriggers > 0)))
  {
    forcedTrigger = FALSE;
  }

  do
  {
    if ((cap_cfg->numEnabledSGPIO == 0) && (cap_cfg->numEnabledVADC == 0))
    {
      // Must have at least one SGPIO or one VADC enabled
      result = CMD_STATUS_ERR_CFG_NO_CHANNELS_ENABLED;
      break;
    }

    result = capture_WeightedConfigCheck(cap_cfg);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    result = capture_SetSampleRate(cap_cfg->sampleRate, cap_cfg->numEnabledVADC);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    result = capture_ConfigureCaptureBuffers(cap_cfg);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    if (cap_cfg->numEnabledSGPIO > 0)
    {
      result = cap_sgpio_Configure(&sampleBufferSGPIO, &cap_cfg->sgpio, cap_cfg->postFill, forcedTrigger, RATECONFIG[currentSampleRateIdx].counter);
      if (result != CMD_STATUS_OK)
      {
        break;
      }
    }

    if (cap_cfg->numEnabledVADC > 0)
    {
      result = cap_vadc_Configure(&sampleBufferVADC, &cap_cfg->vadc, cap_cfg->postFill, forcedTrigger);
      if (result != CMD_STATUS_OK)
      {
        break;
      }
    }

    enabledSgpioChannels = cap_cfg->numEnabledSGPIO;
    enabledVadcChannels = cap_cfg->numEnabledVADC;

  } while (FALSE);

  return result;
}

/**************************************************************************//**
 *
 * @brief  Arms (starts) the signal capturing according to last configuration.
 *
 * @retval CMD_STATUS_OK      If successfully armed
 * @retval CMD_STATUS_ERR_*   If the capture could not be armed
 *
 *****************************************************************************/
cmd_status_t capture_Arm(void)
{
  cmd_status_t result;

  result = statemachine_RequestState(STATE_CAPTURING);
  if (result != CMD_STATUS_OK)
  {
    return result;
  }

  LED_ARM_ON();
  LED_TRIG_OFF();

  memset(&capturedSamples, 0, sizeof(captured_samples_t));

  CAP_PREFILL_SET_AS_NEEDED();

  // Do 99% of preparations for SGPIO
  if (enabledSgpioChannels > 0)
  {
    result = cap_sgpio_PrepareToArm();
    if (result != CMD_STATUS_OK)
    {
      return result;
    }
  }
  else
  {
    CAP_PREFILL_MARK_SGPIO_DONE();
  }

  // Do 99% of preparations for VADC
  if (enabledVadcChannels > 0)
  {
    result = cap_vadc_PrepareToArm();
    if (result != CMD_STATUS_OK)
    {
      return result;
    }
  }
  else
  {
    CAP_PREFILL_MARK_VADC_DONE();
  }

  if (enabledSgpioChannels > 0)
  {
    cap_sgpio_Arm();
  }
  if (enabledVadcChannels > 0)
  {
    cap_vadc_Arm();
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Disarms (stops) the signal capturing.
 *
 * @retval CMD_STATUS_OK      If successfully stopped, or alreay stopped
 * @retval CMD_STATUS_ERR_*   If the capture could not be stopped
 *
 *****************************************************************************/
cmd_status_t capture_Disarm(void)
{
  LED_ARM_OFF();
  LED_TRIG_OFF();

  if (enabledSgpioChannels > 0)
  {
    cap_sgpio_Disarm();
  }
  if (enabledVadcChannels > 0)
  {
    cap_vadc_Disarm();
  }
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Returns the VADC Match Value for the current sample rate.
 *
 * @return The match value
 *
 *****************************************************************************/
uint16_t capture_GetVadcMatchValue(void)
{
  return RATECONFIG[currentSampleRateIdx].counter;
}

/**************************************************************************//**
 *
 * @brief  Returns the frequency (fADC) that the VADC will be executed in.
 *
 * The fADC that this function returns is not the same as the sample rate.
 *
 * The fADC is used when calculating the VADC's CRS and DGECi settings.
 *
 * @return The VADC frequency
 *
 *****************************************************************************/
uint32_t capture_GetFadc(void)
{
  return RATECONFIG[currentSampleRateIdx].pll0_freq;
}

/**************************************************************************//**
 *
 * @brief  Returns the current sample rate.
 *
 * @return The sample rate
 *
 *****************************************************************************/
uint32_t capture_GetSampleRate(void)
{
  return RATECONFIG[currentSampleRateIdx].sampleRate;
}

/**************************************************************************//**
 *
 * @brief  Reports that capturing of digital signal(s) is completed.
 *
 * The result of the capturing is saved and if only digital signals are being
 * captured (or if the analog signal(s) are also done) sends the result to
 * the client.
 *
 *****************************************************************************/
void capture_ReportSGPIODone(circbuff_t* buff, uint32_t trigpoint, uint32_t triggerSample, uint32_t activeChannels)
{
  capturedSamples.trigpoint |= trigpoint;
  capturedSamples.sgpioTrigSample = triggerSample;
  capturedSamples.sgpioActiveChannels = activeChannels;
  capturedSamples.sgpio_samples = buff;

  if (enabledVadcChannels == 0 || capturedSamples.vadc_samples != NULL)
  {
    usb_handler_SendSamples(&capturedSamples);
  }
}

/**************************************************************************//**
 *
 * @brief  Reports that capturing of digital signal(s) failed.
 *
 * The error code is saved and if only digital signals are being captured
 * (or if the analog signal(s) are also done) sends the result to the client.
 *
 * @param [in] error  The error code
 *
 *****************************************************************************/
void capture_ReportSGPIOSamplingFailed(cmd_status_t error)
{
  if (enabledVadcChannels == 0)
  {
    usb_handler_SignalFailedSampling(error);
  }
}

/**************************************************************************//**
 *
 * @brief  Reports that capturing of analog signal(s) is completed.
 *
 * The result of the capturing is saved and if only analog signals are being
 * captured (or if the digital signal(s) are also done) sends the result to
 * the client.
 *
 *****************************************************************************/
void capture_ReportVADCDone(circbuff_t* buff, uint32_t trigpoint, uint32_t triggerSample, uint32_t activeChannels)
{
  capturedSamples.trigpoint |= trigpoint<<16;
  capturedSamples.vadcTrigSample = triggerSample;
  capturedSamples.vadcActiveChannels = activeChannels;
  capturedSamples.vadc_samples = buff;

  if (enabledSgpioChannels == 0 || capturedSamples.sgpio_samples != NULL)
  {
    usb_handler_SendSamples(&capturedSamples);
  }
}

/**************************************************************************//**
 *
 * @brief  Reports that capturing of analog signal(s) failed.
 *
 * The error code is saved and if only analog signals are being captured
 * (or if the digital signal(s) are also done) sends the result to the client.
 *
 * @param [in] error  The error code
 *
 *****************************************************************************/
void capture_ReportVADCSamplingFailed(cmd_status_t error)
{
  if (enabledSgpioChannels == 0)
  {
    usb_handler_SignalFailedSampling(error);
  }
}

/**************************************************************************//**
 *
 * @brief  Configures and then starts capturing of analog inputs for calibration
 *
 * Called during calibration to request sampling on both analog channels at
 * a predetermined rate.
 *
 * @param [in] voltsPerDiv  The wanted Volts/div setting
 *
 * @retval CMD_STATUS_OK      If successfully armed
 * @retval CMD_STATUS_ERR_*   If the capture could not be armed
 *
 *****************************************************************************/
cmd_status_t capture_ConfigureForCalibration(int voltsPerDiv)
{
  cmd_status_t result;
  uint32_t val = voltsPerDiv & 0x7;
  calibrationSetup.numEnabledSGPIO = 0; // no digital signals enabled
  calibrationSetup.numEnabledVADC = 2; // both analog channels enabled
  calibrationSetup.sampleRate = 1000000;  // 1 MHz
  calibrationSetup.postFill = 0x0fffff00 | 50; // 50% post fill, will be ignored
  calibrationSetup.sgpio.enabledChannels = 0; // no digital signals enabled
  calibrationSetup.vadc.enabledChannels = 3; // both analog channels enabled
  calibrationSetup.vadc.enabledTriggers = 0; // want forced trigger
  calibrationSetup.vadc.voltPerDiv =  val | (val<<4);
  calibrationSetup.vadc.couplings = 0; // want DC coupling
  calibrationSetup.vadc.noiseReduction = 0; // no noise reduction

  result = capture_Configure((uint8_t*)&calibrationSetup, sizeof(capture_cfg_t));
  if (result == CMD_STATUS_OK)
  {
    result = capture_Arm();
  }
  return result;
}
