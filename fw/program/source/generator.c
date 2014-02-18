/*!
 * @file
 * @brief   Handles setup shared by analog and digital signal generation
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
#include "lpc43xx_gpio.h"
#include "lpc43xx_gpdma.h"

#include "log.h"
#include "led.h"
#include "generator.h"
#include "generator_dac.h"
#include "generator_sgpio.h"
#include "usb_handler.h"
#include "statemachine.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! Absolute value of difference between \a __a and \a __b. */
#define ABS_DIFF(__a, __b)  (((__a) > (__b)) ? ((__a) - (__b)) : ((__b) - (__a)))


#define GEN_CFG_SGPIO_AVAILABLE  (1<<0)
#define GEN_CFG_DAC_AVAILABLE    (1<<1)

/*! @brief Configuration for signal generation.
 * This is the structure that the client software must send to configure
 * generation of analog and/or digital signals.
 */
typedef struct
{
  uint32_t         available;  /*!< Bitmask, bit0=SGPIO, bit1=DAC */
  uint32_t         runCounter; /*!< 0=countinuous run, 1=run only once, >1 currently invalid */
  gen_sgpio_cfg_t  sgpio;      /*!< Configuration of digital signals */
  gen_dac_cfg_t    dac;        /*!< Configuration of analog signals */
} generator_cfg_t;

/*! @brief A set of parameters that result in a frequency. */
typedef struct
{
  uint32_t freq;     /*!< The frequency */
  uint32_t idiva;    /*!< Value for the IDIVA divider */
  uint32_t idive;    /*!< Value for the IDIVE divider */
  uint32_t counter;  /*!< Value for the SGPIO counter */
} generator_freq_match_t;

/******************************************************************************
 * Global variables
 *****************************************************************************/


/******************************************************************************
 * Local variables
 *****************************************************************************/

static Bool DAC_GenerationEnabled = FALSE;
static Bool SGPIO_GenerationEnabled = FALSE;

static generator_freq_match_t currentSampleRate = {0,0,0,0};

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
 * @brief  Finds the set of parameters that comes closes to the wanted rate.
 *
 * The rate at which a signal is generated is determined by
 *       Rate = PLL1 / IDIVA / IDIVE / SGPIO counter
 *
 * @param [in]   wantedRate  The wanted rate
 * @param [out]  match       The parameters
 *
 *****************************************************************************/
static void generator_FindClosestRate(uint32_t wantedRate, generator_freq_match_t* match)
{
  uint32_t a, e, c;
  uint32_t tmpA, tmpE, tmpC;
  uint32_t bestDiff;
  if (wantedRate >= SystemCoreClock)
  {
    match->idiva = match->idive = match->counter = 1;
    match->freq = SystemCoreClock;
    bestDiff = ABS_DIFF(match->freq, wantedRate);
  }
  else if (wantedRate <= (SystemCoreClock/(16*256*4096)))
  {
    match->idiva = 16;
    match->idive = 256;
    match->counter = 4096;
    match->freq = SystemCoreClock/(16*256*4096);
    bestDiff = ABS_DIFF(match->freq, wantedRate);
  }
  else
  {
    match->idiva = match->idive = match->counter = 0;
    match->freq = 0;
    bestDiff = wantedRate;
    for (a = 1; a <= 16; a++)
    {
      tmpA = SystemCoreClock/a;
      for (e = 1; e <= 256; e++)
      {
        tmpE = tmpA/e;
        for (c = 1; c<=4096; c++)
        {
          tmpC = tmpE/c;
          if (ABS_DIFF(tmpC, wantedRate) < bestDiff)
          {
            bestDiff = ABS_DIFF(tmpC, wantedRate);
            match->idiva = a;
            match->idive = e;
            match->counter = c;
            match->freq = tmpC;

            if (bestDiff == 0)
            {
              // found exact match, return result
              log_i("Exact match for %u is IDIVA=%u, IDIVE=%u, COUNTER=%u\r\n", wantedRate, match->idiva, match->idive, match->counter);
              return;
            }
          }
          if (tmpC < wantedRate)
          {
            // values will only get further apart with increased 'c', try next IDIVE value instead
            break;
          }
        }
      }
    }
  }

  log_i("Best match for %u is %u (off by %u), with IDIVA=%u, IDIVE=%u, COUNTER=%u\r\n", wantedRate, match->freq, bestDiff, match->idiva, match->idive, match->counter);
}

/**************************************************************************//**
 *
 * @brief  Sets the wanted generation rate
 *
 * @param [in]  wantedRate  The wanted rate
 *
 * @retval CMD_STATUS_OK      If successfully set
 * @retval CMD_STATUS_ERR_*   If the sample rate could not be set
 *
 *****************************************************************************/
static cmd_status_t generator_SetRate(uint32_t wantedRate)
{
  generator_freq_match_t tmp;
  generator_FindClosestRate(wantedRate, &tmp);

  CGU_SetDIV(CGU_CLKSRC_IDIVA, tmp.idiva);
  CGU_SetDIV(CGU_CLKSRC_IDIVE, tmp.idive);

  CGU_UpdateClock();

  currentSampleRate = tmp;

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Sets the initial generation rate
 *
 *****************************************************************************/
static void generator_SetInitialRate(void)
{
  /*
   * To be able to reach the lowest rates without modifying PLL1
   * integer dividers IDIVA and IDIVE are used.
   *
   * The board always crashed during testing if the sample rate was
   * changed from a low to a high sample rate (so that one or more of
   * the IDIVx were no longer needed). The current workaround is to
   * always use all but set the divider to 1 when it is not needed.
   *
   * SGPIO will use IDIVA and IDIVE so that
   *
   *      CGP_BASE_PERIPH = (CGU_CLKSRC_PLL1 / IDIVA) / IDIVE
   */

  /* connect IDIVA to PLL, enable, set to autoblock and set divider */
  CGU_EntityConnect(CGU_CLKSRC_PLL1, CGU_CLKSRC_IDIVA);
  CGU_EnableEntity(CGU_CLKSRC_IDIVA, ENABLE);
  LPC_CGU->IDIVA_CTRL |= (1<<11);
//  CGU_SetDIV(CGU_CLKSRC_IDIVA, RATECONFIG[INITIAL_SAMPLE_RATE_IDX].only_dio_idiva);

  /* connect IDIVE to IDIVA, enable, set to autoblock and set divider */
  CGU_EntityConnect(CGU_CLKSRC_IDIVA, CGU_CLKSRC_IDIVE);
  CGU_EnableEntity(CGU_CLKSRC_IDIVE, ENABLE);
  LPC_CGU->IDIVE_CTRL |= (1<<11);
//  CGU_SetDIV(CGU_CLKSRC_IDIVE, RATECONFIG[INITIAL_SAMPLE_RATE_IDX].only_dio_idive);

  generator_SetRate(2000000);

  log_d("Set initial sample rate to %d", currentSampleRate.freq);
}


/******************************************************************************
 * External method
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes generation of both analog and digital signals.
 *
 *****************************************************************************/
void generator_Init(void)
{
  generator_SetInitialRate();

  DAC_GenerationEnabled = FALSE;
  SGPIO_GenerationEnabled = FALSE;

  /*! @todo Move the controls for the DIO direction to a central place as it will prevent any signal capture */
  LPC_GPIO_PORT->SET[1] |= (1UL <<  8);
  LPC_GPIO_PORT->CLR[0] |= (1UL << 14);
  LPC_GPIO_PORT->SET[1] |= (1UL << 11);

  gen_sgpio_Init();
  gen_dac_Init();
}

/**************************************************************************//**
 *
 * @brief  Starts the signal generation according to last configuration.
 *
 * @retval CMD_STATUS_OK      If successfully started
 * @retval CMD_STATUS_ERR_*   If the generation could not be started
 *
 *****************************************************************************/
cmd_status_t generator_Start(void)
{
  cmd_status_t result = CMD_STATUS_OK;

  do
  {
    if (!SGPIO_GenerationEnabled && !DAC_GenerationEnabled)
    {
      result = CMD_STATUS_ERR_NOTHING_TO_GENERATE;
      break;
    }

    if (SGPIO_GenerationEnabled)
    {
      result = gen_sgpio_Start();
      if (result != CMD_STATUS_OK)
      {
        break;
      }
    }

    if (DAC_GenerationEnabled)
    {
      result = gen_dac_Start();
      if (result != CMD_STATUS_OK)
      {
        gen_sgpio_Stop();
        break;
      }
    }

  } while (FALSE);

  return result;
}

/**************************************************************************//**
 *
 * @brief  Stops the signal generation.
 *
 * @retval CMD_STATUS_OK      If successfully stopped, or alreay stopped
 * @retval CMD_STATUS_ERR_*   If the generation could not be stopped
 *
 *****************************************************************************/
cmd_status_t generator_Stop(void)
{
  gen_sgpio_Stop();
  gen_dac_Stop();
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * @param [in] cfg   Configuration from client (must be of type generator_cfg_t)
 * @param [in] size  Size of configuration from client
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t generator_Configure(uint8_t* cfg, uint32_t size)
{
  generator_cfg_t* gen_cfg = (generator_cfg_t*)cfg;
  cmd_status_t result;

  do
  {
    SGPIO_GenerationEnabled = FALSE;
    DAC_GenerationEnabled = FALSE;

    result = statemachine_RequestState(STATE_GENERATING);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    // Make sure all generators have stopped. This ensures that if a previously
    // enabled generator is not enabled anymore it will be stopped.
    generator_Stop();

    if (gen_cfg->available & GEN_CFG_SGPIO_AVAILABLE)
    {
      result = generator_SetRate(gen_cfg->sgpio.frequency);
      if (result != CMD_STATUS_OK)
      {
        break;
      }

      result = gen_sgpio_Configure(&gen_cfg->sgpio, currentSampleRate.counter, gen_cfg->runCounter);
      if (result != CMD_STATUS_OK)
      {
        break;
      }
    }

    if (gen_cfg->available & GEN_CFG_DAC_AVAILABLE)
    {
      result = gen_dac_Configure(&gen_cfg->dac);
      if (result != CMD_STATUS_OK)
      {
        break;
      }
    }

    if (gen_cfg->available & GEN_CFG_SGPIO_AVAILABLE)
    {
      SGPIO_GenerationEnabled = TRUE;
    }
    if (gen_cfg->available & GEN_CFG_DAC_AVAILABLE)
    {
      DAC_GenerationEnabled = TRUE;
    }

  } while (FALSE);

  return result;
}

