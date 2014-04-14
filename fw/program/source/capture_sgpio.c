/*!
 * @file
 * @brief   Handles capturing of digital signals using SGPIO
 * @ingroup IP_SGPIO
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
#include "lpc43xx_cgu.h"
#include "lpc43xx_rgu.h"
#include "lpc43xx_gpio.h"
#include "lpc43xx_gpdma.h"

#include "led.h"
#include "log.h"
#include "capture_sgpio.h"
#include "capture_vadc.h"
#include "sgpio_cfg.h"
#include "meas.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

volatile uint32_t triggered = 0;
volatile uint32_t* circbuff_addr;
volatile uint32_t  circbuff_sample_limit;
volatile uint32_t  circbuff_num_samples;
volatile uint32_t  circbuff_last_sample;
volatile uint32_t  circbuff_last_addr;
volatile uint32_t  circbuff_post_fill;
volatile uint32_t  triggered_pos;

volatile uint32_t CaptureInterruptMask;
volatile uint32_t InputBitInterruptMask;
volatile uint32_t PatternInterruptMask;

static uint32_t activeChannels = 0;

static circbuff_t* pSampleBuffer = NULL;
static uint32_t actualChannelsToCopy = 0;
static uint32_t virtualChannelsToCopy = 0;

static sgpio_channel_config_t config[MAX_NUM_SLICES];

static Bool validConfiguration = FALSE;

static uint32_t slicesToEnable = 0;

static Bool forcedTrigger = FALSE;

static sgpio_concat_t concatenation = SGPIO_CONCAT_NONE;

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Interrupt handler for the SGPIO block
 *
 * The interrupt handler processes two different interrupts:
 *  -# Exchange clock interrupt (STATUS_1)
 *  -# Input bit match interrupt (STATUS_3)
 *
 * The exchange clock interrupt is fired each time the SGPIO's shadow and
 * data registers have been exchanged and at that time this handler copies from
 * the shadow registers into the circular capture buffer.
 * The data is always copied in the correct order so that the data is DIO0, DIO1
 * DIO2,...,DIO0,DIO1,DIO2,... regardless of SGPIO concatenation.
 *
 * The input bit match interrupt is fired if a triggering condition has been
 * met. At that time the position in the circular buffer is saved and VADC is
 * notified (in case analog sampling is done in parallel). An end point is
 * calculated and then the sampling continues.
 *
 * After having copied the data into the circular buffer a test is made to see
 * if the end condition has been met and if so then the SGPIO is stopped and
 * the result is reported through a call to \ref capture_ReportSGPIODone.
 *
 *****************************************************************************/
void SGPIO_IRQHandler(void)
{
  SET_MEAS_PIN_1();

  // Capture Interrupt - Triggered when a slice swap occurs
  if((LPC_SGPIO->STATUS_1) & CaptureInterruptMask)
  {
    LPC_SGPIO->CTR_STATUS_1 = CaptureInterruptMask;

    // The shadow register now contains data that can be read
//  SET_MEAS_PIN_2();

    if (concatenation == SGPIO_CONCAT_NONE)
    {
      switch (actualChannelsToCopy)
      {
//         case 1:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           break;
//         case 2:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           break;
//         case 3:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           break;
//         case 4:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
//           break;
//         case 5:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
//           break;
//         case 6:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
//           break;
//         case 7:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
//           break;
//         case 8:
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
//           *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO7
//           break;
        case 9:
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO7
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO8
          break;
        case 10:
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO7
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO8
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_D]; //DIO9
          break;
        case 11:
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO7
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO8
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_D]; //DIO9
          *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_B]; //CLK
          break;
      }
    }
    else if (concatenation == SGPIO_CONCAT_TWO)
    {
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_I]; //DIO0, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_D]; //DIO1, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_C]; //DIO2, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO3, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_J]; //DIO4, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_F]; //DIO5, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_P]; //DIO6, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_B]; //DIO7, concat
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3    H]; //DIO3
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO4
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO5
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO6    G]; //DIO6
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO7
    }
    else if (concatenation == SGPIO_CONCAT_FOUR)
    {
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_J]; //DIO0, concat 3rd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_D]; //DIO1, concat 3rd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_C]; //DIO2, concat 3rd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO3, concat 3rd

      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO0, concat 2nd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_P]; //DIO1, concat 2nd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO2, concat 2nd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_B]; //DIO3, concat 2nd

      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_I]; //DIO0, concat 1st
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO1, concat 1st
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_F]; //DIO2, concat 1st
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO3, concat 1st

      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO2
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO3
    }
    else if (concatenation == SGPIO_CONCAT_EIGHT)
    {
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_L]; //DIO0, concat 7th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_D]; //DIO1, concat 7th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_F]; //DIO0, concat 6th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_N]; //DIO1, concat 6th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_K]; //DIO0, concat 5th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_G]; //DIO1, concat 5th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_C]; //DIO0, concat 4th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_M]; //DIO1, concat 4th
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_J]; //DIO0, concat 3rd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_B]; //DIO1, concat 3rd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_E]; //DIO0, concat 2nd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_P]; //DIO1, concat 2nd
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_I]; //DIO0, concat 1st
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_H]; //DIO1, concat 1st
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_A]; //DIO0
      *circbuff_addr++ = LPC_SGPIO->REG_SS[SLICE_O]; //DIO1
    }

    //  CLR_MEAS_PIN_2();

    if ((uint32_t)circbuff_addr >= circbuff_last_addr)
    {
      circbuff_addr = (uint32_t*)pSampleBuffer->data;
      if (!CAP_PREFILL_IS_SGPIO_DONE())
      {
        CAP_PREFILL_MARK_SGPIO_DONE();
      }
    }

    // If no triggers are selected then use forced triggering, i.e. fill the
    // capture buffer once and return that to the UI
    if (CAP_PREFILL_IS_PREFILL_DONE() && forcedTrigger && !triggered)
    {
      circbuff_last_sample = circbuff_num_samples + circbuff_sample_limit - 1;
      triggered_pos = circbuff_num_samples + 1;
      triggered = 1; // to prevent ending up here repeatedly
    }

    if (++circbuff_num_samples == circbuff_last_sample)
    {
      // disable SGPIO
      NVIC_DisableIRQ(SGPIO_IINT_IRQn);
      LPC_SGPIO->CTRL_ENABLED &= ~0xffff;

      // update sample buffer with correct positions
      pSampleBuffer->empty = (circbuff_num_samples < circbuff_sample_limit)?TRUE:FALSE;
      pSampleBuffer->last = (circbuff_num_samples % circbuff_sample_limit) * (virtualChannelsToCopy * 4);// * concatenation);

      // Convert the sample position into an address in the circular buffer
      triggered_pos = (triggered_pos % circbuff_sample_limit) * (virtualChannelsToCopy * 4);

      // Convert the address into a relative address that it will have after straightening out the
      // circular buffer
      triggered_pos = circbuff_ConvertAddress(pSampleBuffer, ((uint32_t)pSampleBuffer->data) + triggered_pos);

      // Convert the relative address into a sample number that the client can use after converting all
      // samples into arrays, one per channel.
      triggered_pos = (triggered_pos * 32) / (actualChannelsToCopy * 4);

      // time to send to the PC
      if (forcedTrigger || (PatternInterruptMask == 0 && InputBitInterruptMask == 0))
      {
        capture_ReportSGPIODone(
          pSampleBuffer,
          0, // forced trigger or triggered by VADC
          triggered_pos,
          activeChannels | (actualChannelsToCopy << 16));
      }
      else
      {
        capture_ReportSGPIODone(
          pSampleBuffer,
          sgpio_cfg_GetDioForSliceInterrupt(triggered),
          triggered_pos,
          activeChannels | (actualChannelsToCopy << 16));
      }
    }
  }

  // Input Bit Match Interrupt - A real trigger on rising/falling/low/high
  // depending on the value of the DATA_CAPTURE_MODE
  else if (((LPC_SGPIO->STATUS_3) & InputBitInterruptMask))
  {
    // extract information about which channel caused the trigger
    uint32_t tmp = LPC_SGPIO->STATUS_3 & InputBitInterruptMask;

    // Must lower the interrupt flag
    LPC_SGPIO->CTR_STATUS_3 = InputBitInterruptMask;

    if (CAP_PREFILL_IS_PREFILL_DONE() && !triggered)
    {
      //SET_MEAS_PIN_2();
      triggered = tmp;

      LPC_SGPIO->CTR_STATUS_3 = InputBitInterruptMask;

      // Found a trigger which means:
      // 1) Stop looking for triggers
      // 2) Calculate how many more samples to collect
      // 3) Save trigger position
      LPC_SGPIO->CLR_EN_3 = InputBitInterruptMask;
      cap_sgpio_Triggered();

      // In case both VADC and SGPIO are being sampled, notify VADC as well
      cap_vadc_Triggered();

      //CLR_MEAS_PIN_2();
    }
  }
  CLR_MEAS_PIN_1();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Prepares SGPIO for a new capture
 *
 * Resets the SGPIO block, disables interrupts and then initializes SGPIO
 * with the \a pConfig data.
 *
 * @param [in] pConfig   The SGPIO configuration
 *
 *****************************************************************************/
static void cap_sgpio_Setup(const sgpio_channel_config_t* const pConfig)
{
  int i;

  // Reset the SGPIO block
  RGU_SoftReset(RGU_SIG_SGPIO);
  while(RGU_GetSignalStatus(RGU_SIG_SGPIO));

  // Initialize the SGPIO interrupt (shared by shift/capture/match/input)
  NVIC_DisableIRQ(SGPIO_IINT_IRQn);

  // clear interrupt status and wait for it to clear
  LPC_SGPIO->CTR_STATUS_1 = 0xffff;
  while(LPC_SGPIO->STATUS_1 & 0xffff);
  LPC_SGPIO->CTR_STATUS_2 = 0xffff;
  while(LPC_SGPIO->STATUS_2 & 0xffff);
  LPC_SGPIO->CTR_STATUS_3 = 0xffff;
  while(LPC_SGPIO->STATUS_3 & 0xffff);

  // disable all SGPIO interrupts
  LPC_SGPIO->CLR_EN_1 = 0xffff;
  while(LPC_SGPIO->ENABLE_1 & 0xffff);
  LPC_SGPIO->CLR_EN_2 = 0xffff;
  while(LPC_SGPIO->ENABLE_2 & 0xffff);
  LPC_SGPIO->CLR_EN_3 = 0xffff;
  while(LPC_SGPIO->ENABLE_3 & 0xffff);

  NVIC_EnableIRQ(SGPIO_IINT_IRQn);

  triggered             = 0;
  activeChannels        = 0;
  CaptureInterruptMask  = 0;
  PatternInterruptMask  = 0;
  InputBitInterruptMask = 0;
  slicesToEnable        = 0;

  // Disable all slices
  LPC_SGPIO->CTRL_ENABLED &= ~0xffff;

  for (i = 0; i < MAX_NUM_SLICES; i++)
  {
    const sgpio_channel_config_t* p = &(pConfig[i]);

    if (p->enabled)
    {
      LPC_SGPIO->SLICE_MUX_CFG[p->slice] = p->slice_mux_cfg;
      LPC_SGPIO->SGPIO_MUX_CFG[p->slice] = p->sgpio_mux_cfg;
      LPC_SGPIO->OUT_MUX_CFG[p->pin] = p->out_mux_cfg;

      LPC_SGPIO->GPIO_OENREG &= ~p->gpio_oenreg;
      LPC_SGPIO->GPIO_OENREG |=  p->gpio_oenreg;

      LPC_SGPIO->COUNT[p->slice] = 0;

      LPC_SGPIO->POS[p->slice] = p->pos;
      LPC_SGPIO->PRESET[p->slice] = p->preset;

      LPC_SGPIO->REG[p->slice] = p->reg;
      LPC_SGPIO->REG_SS[p->slice] = p->reg_ss;

      if (p->slice == SLICE_A)
      {
        LPC_SGPIO->MASK_A = p->mask;
      }
      if (p->slice == SLICE_P)
      {
        LPC_SGPIO->MASK_P = p->mask;
      }

      // Shift clock interrupt for the slice
      LPC_SGPIO->SET_EN_0 |= p->set_en_0;

      // Capture interrupt for the slice
      LPC_SGPIO->SET_EN_1  |= p->set_en_1;
      CaptureInterruptMask |= p->set_en_1;

      // Pattern match interrupt for the slice
      LPC_SGPIO->SET_EN_2 |= p->set_en_2;
      PatternInterruptMask |= p->set_en_2;

      // Enable the input bit match interrupt for the slice
      LPC_SGPIO->SET_EN_3   |= p->set_en_3;
      InputBitInterruptMask |= p->set_en_3;

      slicesToEnable |= (1 << p->slice);
      if (!p->internal)
      {
        activeChannels |= (1 << p->dio);
      }
    }
  }
}

/**************************************************************************//**
 *
 * @brief  Calculates the number of samples to collect after the trigger is found
 *
 * Post fill configuration. The lower 8 bits specify the percent of the
 * maximum buffer size that will be used for samples taken AFTER the trigger.
 * The upper 24 bits specifies the maximum number of samples to gather after
 * a trigger has been found.
 *
 * The result is stored in \ref circbuff_post_fill.
 *
 * @param [in] postFill   The post fill configuration
 *
 * @retval CMD_STATUS_OK                           If successfully configured
 * @retval CMD_STATUS_ERR_INVALID_POSTFILLPERCENT  If the level is invalid
 *
 *****************************************************************************/
static cmd_status_t cap_sgpio_CalculatePostFill(uint32_t postFill)
{
  uint32_t postFillPercent = postFill & 0xff;
  uint32_t postFillSamples = (postFill >> 8) & 0xffffff;

  if (postFillPercent > 100)
  {
    return CMD_STATUS_ERR_INVALID_POSTFILLPERCENT;
  }

  // Apply percent limit
  circbuff_post_fill = (circbuff_sample_limit * postFillPercent)/100;

  // Apply time limit
  circbuff_post_fill = MIN(circbuff_post_fill, postFillSamples);

  // Need at least one sample after the trigger is found
  circbuff_post_fill = MAX(1, circbuff_post_fill);

  // Need at least five samples before the trigger is found, due to the fact that
  // the "input bit interrupt" for the trigger seems to occur ca 3 "exchange interrupts"
  // after the value change
  circbuff_post_fill = MIN(circbuff_post_fill, circbuff_sample_limit - 5);

  return CMD_STATUS_OK;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Enables the clock for SGPIO and specifies IRQ handler
 *
 *****************************************************************************/
void cap_sgpio_Init(void)
{
  /*
   * PLL0AUDIO is configured and enabled in capture.c.
   */

  /* connect the SGPIO block to PLL0AUDIO */
  CGU_EntityConnect(CGU_CLKSRC_PLL0_AUDIO, CGU_BASE_PERIPH);
  CGU_EnableEntity(CGU_BASE_PERIPH, ENABLE);

  // Change IRQ handler by manipulating the VectorTable, code from
  // lpc43xx.git:\Examples\SGPIO\SGPIO_DMA_PatternGenerator\setup_M0.c
  uint32_t *VectorTable_ptr_M4;
  VectorTable_ptr_M4 = ((uint32_t *) 0x10000000);
  VectorTable_ptr_M4[47] = ((uint32_t)SGPIO_IRQHandler);
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * The "force trigger mode" means that no trigger is used and instead the entire
 * capture buffer should be filled and then returned to the client.
 *
 * @param [in] buff              Circular buffer to store captured data in
 * @param [in] cfg               Configuration to apply
 * @param [in] postFill          How many samples to take after a trigger is found
 * @param [in] forceTrigger      TRUE if "force trigger mode" should be used
 * @param [in] shiftClockPreset  Clocking information
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t cap_sgpio_Configure(circbuff_t* buff,
                                 cap_sgpio_cfg_t* cfg,
                                 uint32_t postFill,
                                 Bool forceTrigger,
                                 uint32_t shiftClockPreset)
{
  int i;
  pSampleBuffer = buff;
  cmd_status_t result;
  validConfiguration = FALSE;
  forcedTrigger = forceTrigger;

  for (i = 0; i < MAX_NUM_SLICES; i++)
  {
    config[i].enabled = FALSE;
  }

  do
  {
    result = sgpio_cfg_SetupInputChannels(config, &concatenation, cfg, shiftClockPreset);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    actualChannelsToCopy = 0;
    switch (concatenation)
    {
      case SGPIO_CONCAT_NONE:
        for (i = 0; i < MAX_NUM_SLICES; i++)
        {
          if (config[i].enabled && !config[i].internal)
          {
            actualChannelsToCopy = MAX(actualChannelsToCopy, config[i].dio + 1);
          }
        }
        virtualChannelsToCopy = actualChannelsToCopy;
        break;

      case SGPIO_CONCAT_TWO:
        actualChannelsToCopy = 8;
        virtualChannelsToCopy = 16;
        break;

      case SGPIO_CONCAT_FOUR:
        actualChannelsToCopy = 4;
        virtualChannelsToCopy = 16;
        break;

      case SGPIO_CONCAT_EIGHT:
        actualChannelsToCopy = 2;
        virtualChannelsToCopy = 16;
        break;
    }


    if (actualChannelsToCopy == 0)
    {
      result = CMD_STATUS_ERR;
      break;
    }

    // Configure the circular buffer data for use by the interrupt handler
    circbuff_addr = (uint32_t*)pSampleBuffer->data;
    circbuff_sample_limit = pSampleBuffer->maxSize/(virtualChannelsToCopy * 4);// * concatenation);

    log_i("Actual %2d, Virtual %2d, Sample Limit %4d\r\n", actualChannelsToCopy, virtualChannelsToCopy, circbuff_sample_limit);

    // Trim the size of the circular buffer to be an even multiple of the number
    // of channels in this capture
    circbuff_Resize(pSampleBuffer, circbuff_sample_limit * virtualChannelsToCopy * 4);// * concatenation);

    circbuff_last_addr = (uint32_t)pSampleBuffer->data + pSampleBuffer->size;

    // Determine how much of the buffer should be used for PRE- resp POST-trigger samples
    result = cap_sgpio_CalculatePostFill(postFill);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    validConfiguration = TRUE;
  } while (0);

  return result;
}

/**************************************************************************//**
 *
 * @brief  Do all time consuming parts of arming.
 *
 * This function is used to get a better synchronization between analog and
 * digital signal capturing. First *_PrepareToArm will be called on both and
 * then when everything is prepared the *_Arm functions are called to start.
 *
 * @retval CMD_STATUS_OK    If successfully prepared
 * @retval CMD_STATUS_ERR   If not properly configured
 *
 *****************************************************************************/
cmd_status_t cap_sgpio_PrepareToArm(void)
{
  if (!validConfiguration)
  {
    // no point in arming if the configuration is invalid
    return CMD_STATUS_ERR;
  }

  circbuff_addr = (uint32_t*)pSampleBuffer->data;
  circbuff_num_samples = 0;
  circbuff_last_sample = 0xffffffff;
  triggered_pos = 0xffffffff;

  circbuff_Reset(pSampleBuffer);

  CLR_MEAS_PIN_1();
  //CLR_MEAS_PIN_2();
  //CLR_MEAS_PIN_3();
  cap_sgpio_Setup(config);
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Do the actual arming (start the capture).
 *
 *****************************************************************************/
void cap_sgpio_Arm(void)
{
  // Enable the slice(s)
  LPC_SGPIO->CTRL_ENABLED |= slicesToEnable;
  LPC_SGPIO->CTRL_DISABLED &= ~slicesToEnable;
}

/**************************************************************************//**
 *
 * @brief  Disarms (stops) the signal capturing.
 *
 * @retval CMD_STATUS_OK      If successfully stopped, or alreay stopped
 * @retval CMD_STATUS_ERR_*   If the capture could not be stopped
 *
 *****************************************************************************/
cmd_status_t cap_sgpio_Disarm(void)
{
  // Disable all slices
  LPC_SGPIO->CTRL_ENABLED &= ~0xffff;

  // Disable the capture interrupt for all slices
  LPC_SGPIO->CLR_EN_1 = 0xffff;

  // Disable the input bit match interrupt for all slices
  LPC_SGPIO->CLR_EN_3 = 0xffff;

  // Disable the SGPIO interrupt (shared by shift/capture/match/input)
  NVIC_DisableIRQ(SGPIO_IINT_IRQn);
  LPC_SGPIO->CTR_STATUS_1 = 0xffff;    // clear capture clock interrupt status
  while(LPC_SGPIO->STATUS_1 & 0xffff); // wait for status to clear

  // Reset the SGPIO block
  RGU_SoftReset(RGU_SIG_SGPIO);
  while(RGU_GetSignalStatus(RGU_SIG_SGPIO));

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Flags the current capture as triggered
 *
 * This function is used when both analog and digital signals are being captured
 * and a trigger has been detected in the analog signal. The purpose is to
 * immediately start with the post trigger sampling.
 *
 * As this function is called from inside an interrupt handler it must be kept
 * as fast as possible.
 *
 *****************************************************************************/
void inline cap_sgpio_Triggered(void)
{
  circbuff_last_sample = circbuff_num_samples + circbuff_post_fill;
  triggered_pos = circbuff_num_samples;
}
