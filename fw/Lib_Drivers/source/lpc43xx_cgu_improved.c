/*!
 * @file
 * @brief     Fixes to the broken or limited lpc43xx_cgu driver
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

#include "lpc43xx_cgu_improved.h"
#include "labtool_config.h"

/******************************************************************************
 * Defines and typedefs
 *****************************************************************************/

/*! Maximum allowed value for the PLL0AUDIO M-divider. */
#define PLL0_MSEL_MAX (1<<15)

/*! Maximum allowed value for the PLL0AUDIO N-divider. */
#define PLL0_NSEL_MAX (1<<8)

/*! Maximum allowed value for the PLL0AUDIO P-divider. */
#define PLL0_PSEL_MAX (1<<5)

/*! Frequency of external xtal */
#define XTAL_FREQ  (12000000UL)

/******************************************************************************
 * External global variables
 *****************************************************************************/

extern uint32_t CGU_ClockSourceFrequency[CGU_CLKSRC_NUM];

/******************************************************************************
 * Local variables
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief Computes the MDEC value from the msel (M) value.
 *
 * Function is described in section "PLL0AUDIO M-divider register" in the
 * LPC43xx User's Manual.
 *
 * @param [in] msel  The M divider
 *
 * @return The MDEC value to use in the PLL0AUDIO_MDIV register
 *
 *****************************************************************************/
static uint32_t FindMDEC(uint32_t msel)
{
  /* multiplier: compute mdec from msel */
  uint32_t x = 0x4000;
  uint32_t im;

  switch (msel)
  {
    case 0:
      return 0xffffffff;
    case 1:
      return 0x18003;
    case 2:
      return 0x10003;
    default:
      for (im = msel; im <= PLL0_MSEL_MAX; im++)
      {
        x = ((x ^ x>>1) & 1) << 14 | x>>1 & 0xFFFF;
      }
      return x;
  }
}

/**************************************************************************//**
 *
 * @brief Computes the NDEC value from the nsel (N) value.
 *
 * Function is described in section "PLL0AUDIO NP-divider register" in the
 * LPC43xx User's Manual.
 *
 * @param [in] nsel  The N divider
 *
 * @return The NDEC value to use in the PLL0AUDIO_NP_DIV register
 *
 *****************************************************************************/
static uint32_t FindNDEC(uint32_t nsel)
{
  /* pre-divider: compute ndec from nsel */
  uint32_t x = 0x80;
  uint32_t in;

  switch (nsel)
  {
    case 0:
      return 0xffffffff;
    case 1:
      return 0x302;
    case 2:
      return 0x202;
    default:
      for (in = nsel; in <= PLL0_NSEL_MAX; in++)
      {
        x = ((x ^ x>>2 ^ x>>3 ^ x>>4) & 1) << 7 | x>>1 & 0xFF;
      }
      return x;
  }
}

/**************************************************************************//**
 *
 * @brief Computes the PDEC value from the psel (P) value.
 *
 * Function is described in section "PLL0AUDIO NP-divider register" in the
 * LPC43xx User's Manual.
 *
 * @param [in] psel  The P divider
 *
 * @return The PDEC value to use in the PLL0AUDIO_NP_DIV register
 *
 *****************************************************************************/
static uint32_t FindPDEC(uint32_t psel)
{
  /* post-divider: compute pdec from psel */
  uint32_t x = 0x10;
  uint32_t ip;

  switch (psel)
  {
    case 0:
      return 0xffffffff;
    case 1:
      return 0x62;
    case 2:
      return 0x42;
    default:
      for (ip = psel; ip <= PLL0_PSEL_MAX; ip++)
      {
        x = ((x ^ x>>2) & 1) << 4 | x>>1 & 0x3F;
      }
      return x;
  }
}

/**************************************************************************//**
 *
 * @brief Rough approximation of a delay function with microsecond resolution.
 *
 * Used during initiale clock setup as the Timers are not configured yet.
 *
 * @param [in] us  The number of microseconds to wait
 *
 *****************************************************************************/
static void emc_WaitUS(volatile uint32_t us)
{
  us *= (SystemCoreClock / 1000000) / 3;
  while(us--);
}


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief Setup PLL0Audio as XTAL*msel/(nsel*psel)
 *
 * @param [in] msel  Multiple value
 * @param [in] nsel  Pre-divider value
 * @param [in] psel  Post-divider value
 *
 * @return  Status of operation
 *
 *****************************************************************************/
CGU_ERROR CGU_Improved_SetPLL0audio(uint32_t msel, uint32_t nsel, uint32_t psel)
{
  uint32_t ClkSrc;

  /* disable clock, disable skew enable, power down pll,
  * (dis/en)able post divider, (dis/en)able pre-divider,
  * disable free running mode, disable bandsel,
  * enable up limmiter, disable bypass
  */
  LPC_CGU->PLL0AUDIO_CTRL = (6 << 24)   /* source = XTAL OSC 12 MHz */
                            | _BIT(0);  /* power down */

  /* set NDEC, PDEC and MDEC register */
  LPC_CGU->PLL0AUDIO_NP_DIV = (FindNDEC(nsel)<<12) | (FindPDEC(psel) << 0);
  LPC_CGU->PLL0AUDIO_MDIV = FindMDEC(msel);

  LPC_CGU->PLL0AUDIO_CTRL = (6 << 24)   /* source = XTAL OSC 12 MHz */
                            | (6<< 12);     // fractional divider off and bypassed

  /* wait for lock */
  while (!(LPC_CGU->PLL0AUDIO_STAT & 1));

  /* enable clock output */
  LPC_CGU->PLL0AUDIO_CTRL |= (1<<4); /* CLKEN */

  ClkSrc = (LPC_CGU->PLL0AUDIO_CTRL & CGU_CTRL_SRC_MASK)>>24;
  CGU_ClockSourceFrequency[CGU_CLKSRC_PLL0_AUDIO] =
    (msel * CGU_ClockSourceFrequency[ClkSrc] ) / (psel * nsel);

  return CGU_ERROR_SUCCESS;
}

/**************************************************************************//**
 *
 * @brief   Setting PLL1, result is XTAL*mult/div
 *
 * @param[in] mult  Multiple value
 * @param[in] div   Divider value
 *
 * @retval CGU_ERROR_SUCCESS        On successful
 * @retval CGU_ERROR_INVALID_PARAM  Invalid parameter error
 *
 *****************************************************************************/
CGU_ERROR CGU_Improved_SetPLL1(uint32_t mult, uint32_t div)
{
  uint32_t msel=0, nsel=0, psel=0, pval=1;
  uint32_t freq;
  uint32_t ClkSrc = (LPC_CGU->PLL1_CTRL & CGU_CTRL_SRC_MASK)>>24;
  freq = CGU_ClockSourceFrequency[ClkSrc];
  freq *= mult;
  freq /= div;
  msel = mult-1;
  nsel = div-1;

  LPC_CGU->PLL1_CTRL &= ~(CGU_PLL1_FBSEL_MASK |
                          CGU_PLL1_BYPASS_MASK |
                          CGU_PLL1_DIRECT_MASK |
                          (0x03<<8) | (0xFF<<16) | (0x03<<12));

  if(freq<156000000)
  {
    //psel is encoded such that 0=1, 1=2, 2=4, 3=8
    while(2*(pval)*freq < 156000000)
    {
      psel++;
      pval*=2;
    }
//    if(2*(pval)*freq > 320000000) {
//      //THIS IS OUT OF RANGE!!!
//      //HOW DO WE ASSERT IN SAMPLE CODE?
//      //__breakpoint(0);
//      return CGU_ERROR_INVALID_PARAM;
//    }
    LPC_CGU->PLL1_CTRL |= (msel<<16) | (nsel<<12) | (psel<<8) | CGU_PLL1_FBSEL_MASK;
  }
  else if(freq<320000000)
  {
    LPC_CGU->PLL1_CTRL |= (msel<<16) | (nsel<<12) | (psel<<8) | CGU_PLL1_DIRECT_MASK | CGU_PLL1_FBSEL_MASK;
  }
  else
  {
    return CGU_ERROR_INVALID_PARAM;
  }

  return CGU_ERROR_SUCCESS;
}

/**************************************************************************//**
 *
 * @brief   Sets up the PLL1 and connects it to CGU_BASE_M4
 *
 * As the PLL1 will be configured for a high operating frequency (200MHz) it
 * is first increased to the mid range frequency (108MHz) and then target
 * again up to 200MHz. The reason for this is explained in section
 * "Configureing the BASE_M4_CLK for high operating frequencies" in the LPC43xx
 * User's Manual.
 *
 *****************************************************************************/
void CGU_Improved_Init(void)
{
#if (HYPERSPEED == OPT_ENABLED)
  CGU_EnableEntity(CGU_CLKSRC_IRC, ENABLE);

  /* enable the crystal oscillator */
  CGU_SetXTALOSC(XTAL_FREQ);
  CGU_EnableEntity(CGU_CLKSRC_XTAL_OSC, ENABLE);

  /* connect the cpu to the xtal */
  CGU_EntityConnect(CGU_CLKSRC_XTAL_OSC, CGU_BASE_M4);

  /* connect the PLL to the xtal */
  CGU_EntityConnect(CGU_CLKSRC_XTAL_OSC, CGU_CLKSRC_PLL1);

  /* configure the PLL to 120 MHz */
  CGU_SetPLL1(10);
  while((LPC_CGU->PLL1_STAT&1) == 0x0);

  /* enable the PLL */
  CGU_EnableEntity(CGU_CLKSRC_PLL1, ENABLE);

  /* connect to the CPU core */
  CGU_EntityConnect(CGU_CLKSRC_PLL1, CGU_BASE_M4);

  SystemCoreClock = 120000000;

  /* wait one msec */
  emc_WaitUS(1000);

  /* Change the clock to 240 MHz without disconnecting the CPU */
  /* uses direct mode */
  CGU_SetPLL1(20);
  while((LPC_CGU->PLL1_STAT&1) == 0x0);

  SystemCoreClock = 240000000;

  CGU_UpdateClock();
#else
  /* after boot CPU runs at 96 MHz */
  /* cpu runs from: IRC (12MHz) >> PLL M = 24, FCCO @ 288 MHz direct mode >> IDIVC = 4 >> 96 MHz */

  /* enable the crystal oscillator */
  CGU_SetXTALOSC(XTAL_FREQ);
  CGU_EnableEntity(CGU_CLKSRC_XTAL_OSC, ENABLE);

  /* connect the cpu to the xtal */
  CGU_EntityConnect(CGU_CLKSRC_XTAL_OSC, CGU_BASE_M4);

  /* connect the PLL to the xtal */
  CGU_EntityConnect(CGU_CLKSRC_XTAL_OSC, CGU_CLKSRC_PLL1);

  /* configure the PLL to 108 MHz */
  CGU_SetPLL1(9);

  /* enable the PLL */
  CGU_EnableEntity(CGU_CLKSRC_PLL1, ENABLE);

  /* connect to the CPU core */
  CGU_EntityConnect(CGU_CLKSRC_PLL1, CGU_BASE_M4);

  SystemCoreClock = 108000000;

  /* wait one msec */
  emc_WaitUS(1000);

  /* Change the clock to 200 MHz without disconnecting the CPU */
  /* uses direct mode */
  CGU_Improved_SetPLL1(50, 3);

  SystemCoreClock = 200000000;

  /* Wait for PLL1 to lock before CGU_UpdateClock, otherwise it will set the
     clock source frequency for PLL1 to 0 which will cause problems for all
     other clocks. */
  while((LPC_CGU->PLL1_STAT & 1) == 0)
  {
  }

  CGU_UpdateClock();
#endif
}
