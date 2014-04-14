/*!
 * @file
 * @brief   Handles capturing of analog signals using the 12-bit VADC
 * @ingroup IP_VADC
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
#include "lpc43xx_timer.h"

#include "led.h"
#include "log.h"
#include "capture_vadc.h"
#include "capture_sgpio.h"
#include "meas.h"
#include "spi_control.h"

#include <string.h>

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#define STATUS0_FIFO_FULL_MASK      (1<<0)
#define STATUS0_FIFO_EMPTY_MASK     (1<<1)
#define STATUS0_FIFO_OVERFLOW_MASK  (1<<2)
#define STATUS0_DESCR_DONE_MASK     (1<<3)
#define STATUS0_DESCR_ERROR_MASK    (1<<4)
#define STATUS0_ADC_OVF_MASK        (1<<5)
#define STATUS0_ADC_UNF_MASK        (1<<6)

#define STATUS0_CLEAR_MASK          0x7f

#define STATUS1_THCMP_BRANGE(__ch)  ((1<<0) << (5 * (__ch)))
#define STATUS1_THCMP_ARANGE(__ch)  ((1<<1) << (5 * (__ch)))
#define STATUS1_THCMP_DCROSS(__ch)  ((1<<2) << (5 * (__ch)))
#define STATUS1_THCMP_UCROSS(__ch)  ((1<<3) << (5 * (__ch)))
#define STATUS1_THCMP_OVERRUN(__ch) ((1<<4) << (5 * (__ch)))

#define STATUS1_CLEAR_MASK          0x1fffffff


#define VDIV_CH1_MASK   ( CTRL_CH1_GN0 | CTRL_CH1_GN1 | CTRL_CH1_GN2 )
#define VDIV_CH1_5000   (                               CTRL_CH1_GN2 )
#define VDIV_CH1_2000   ( CTRL_CH1_GN0 |                CTRL_CH1_GN2 )
#define VDIV_CH1_1000   (                CTRL_CH1_GN1 | CTRL_CH1_GN2 )
#define VDIV_CH1_0500   ( CTRL_CH1_GN0 | CTRL_CH1_GN1 | CTRL_CH1_GN2 )
#define VDIV_CH1_0200   (                    0UL                     )
#define VDIV_CH1_0100   ( CTRL_CH1_GN0                               )
#define VDIV_CH1_0050   (                CTRL_CH1_GN1                )
#define VDIV_CH1_0020   ( CTRL_CH1_GN0 | CTRL_CH1_GN1                )

#define VDIV_CH2_MASK   ( CTRL_CH2_GN0 | CTRL_CH2_GN1 | CTRL_CH2_GN2 )
#define VDIV_CH2_5000   (                               CTRL_CH2_GN2 )
#define VDIV_CH2_2000   ( CTRL_CH2_GN0 |                CTRL_CH2_GN2 )
#define VDIV_CH2_1000   (                CTRL_CH2_GN1 | CTRL_CH2_GN2 )
#define VDIV_CH2_0500   ( CTRL_CH2_GN0 | CTRL_CH2_GN1 | CTRL_CH2_GN2 )
#define VDIV_CH2_0200   (                    0UL                     )
#define VDIV_CH2_0100   ( CTRL_CH2_GN0                               )
#define VDIV_CH2_0050   (                CTRL_CH2_GN1                )
#define VDIV_CH2_0020   ( CTRL_CH2_GN0 | CTRL_CH2_GN1                )

#define VADC_DMA_WRITE  7
#define VADC_DMA_READ   8
#define VADC_DMA_READ_SRC  (LPC_VADC_BASE + 512)  /* VADC FIFO */

#define FIFO_SIZE       8

#define LAST_CH_1   0
#define LAST_CH_2   1

#define NOISE_REDUCTION_ENABLED(__val)  ((__val) & (1UL<<31))
#define NOISE_REDUCTION_LEVEL(__val)    ((__val) & 0xfff)

#define TRIG_MIN_VALUE   (0x010)
#define TRIG_MAX_VALUE   (0xfef)

#define PREFILL_TIME_IN_MS  (100)

/*! @brief Active configuration for analog channel sampling */
typedef struct
{
  Bool            valid;                 /*!< True if the VADC is ready to be armed */

  /*! True if the capturing should stop when the buffer is full and not wait
   * for a trigger to be found */
  Bool            forcedTrigger;

  uint32_t        matchValue;            /*!< Counter needed to achieve the wanted sample rate */
  uint32_t        crs;                   /*!< Current Setting Signal for the POWER_CONTROL register */
  uint32_t        DGECi;                 /*!< Value for the speed register ADC_SPEED */
  uint32_t        sample_size;           /*!< Bytes needed to store on sample for each of the enabled channels */
  uint32_t        numEnabledChannels;    /*!< Number of enabled analog channels (1 or 2) */

  /*! Number of times to completely fill the capture buffer to reach the wanted
   * \ref PREFILL_TIME_IN_MS time of prefill. This value will be decremented until zero. */
  uint32_t        prefillBuffersToFill;

  /*! Original value of prefillBuffersToFill and the value to restore it to after each stop of VADC */
  uint32_t        prefillBuffersToRestore;

  cap_vadc_cfg_t  from_client;           /*!< Copy of configuration received from the client */
} internal_vadc_cfg_t;

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static const uint32_t VDIV_CONFIG[8][3] =
{
  {   20, VDIV_CH1_0020, VDIV_CH2_0020 },
  {   50, VDIV_CH1_0050, VDIV_CH2_0050 },
  {  100, VDIV_CH1_0100, VDIV_CH2_0100 },
  {  200, VDIV_CH1_0200, VDIV_CH2_0200 },
  {  500, VDIV_CH1_0500, VDIV_CH2_0500 },
  { 1000, VDIV_CH1_1000, VDIV_CH2_1000 },
  { 2000, VDIV_CH1_2000, VDIV_CH2_2000 },
  { 5000, VDIV_CH1_5000, VDIV_CH2_5000 },
};

static volatile uint32_t triggered = 0;
static volatile Bool     started = FALSE;
static volatile uint32_t* circbuff_addr;
static volatile uint32_t  circbuff_sample_limit;
static volatile uint32_t  circbuff_num_samples;
static volatile uint32_t  circbuff_last_addr;
static volatile uint32_t  triggeredSampleAddr = 0;

static uint32_t noiseReductionEnabled = 0;
static uint32_t noiseReductionCounter = 0;
static uint32_t noiseReductionMask = 0;
static uint32_t noiseReductionValue1 = 0;
static uint32_t noiseReductionValue2 = 0;

static uint32_t Interrupt1Mask = 0;

static circbuff_t* pSampleBuffer = NULL;

static internal_vadc_cfg_t activeCfg;

/*! The number of LLIs is important as the transfer size for each
    LLI must be an even multiple of the FIFO size. E.g. by using
    21 instead of 20 the "extra" size on the last LLI is reduced:

    65536 bytes buffer, 20 LLIs => each transfer is 3272 bytes.
    Multiplying 20*3272 = 65440 which means that the last LLI
    must be 3272+96 = 3368

    65536 bytes buffer, 21 LLIs => each transfer is 3120 bytes.
    Multiplying 20*3120 = 65520 which means that the last LLI
    must be 3120 + 16 = 3136
*/
#define DMA_NUM_LLI_TO_USE    21
static GPDMA_LLI_Type DMA_Stuff[DMA_NUM_LLI_TO_USE];
static uint32_t post_fill_llis = 0;

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Interrupt handler for the VADC's DMA transfers
 *
 * The interrupt handler processes only the DMA's Terminal Count (TC) interrupt.
 *
 * The TC interrupt is fired in two different circumstances:
 *  -# The last byte in the DMA's LLI chain has been copied meaning that the
 *     end of the circular buffer has been reached and that the next copy will
 *     be to the start of the circular buffer. Used to detect end of prefill.
 *  -# The very last entry of the DMA's LLI chain has been completed. This
 *     happens when a trigger has been found, see \ref cap_vadc_Triggered.
 *     When it happens the VADC is stopped and the result is reported through
 *     the \ref capture_ReportVADCDone
 *     
 *****************************************************************************/
void DMA_IRQHandler (void)
{
  SET_MEAS_PIN_3();
  if (LPC_GPDMA->INTTCSTAT & 1)
  {
    LPC_GPDMA->INTTCCLEAR = 1;
    if (CAP_PREFILL_IS_PREFILL_DONE())
    {
      if (activeCfg.forcedTrigger || !triggered)
      {
        // DMA CH0 has an terminal count interrupt in the last LLI to notify that
        // the end of the buffer has been reached. Mark the sample buffer and then
        // disable further use of this interrupt.
        DMA_Stuff[DMA_NUM_LLI_TO_USE - 1].Control &= ~(0x1UL << 31); // Terminal count interrupt disabled
        pSampleBuffer->empty = FALSE;
        //NVIC_DisableIRQ(DMA_IRQn);
        triggeredSampleAddr = ((uint32_t)circbuff_addr);
      }

      if (triggered)
      {
        LPC_GPDMA->C0CONFIG |= (1 << 18);         //halt further requests
        while (LPC_GPDMA->C0CONFIG & (1<<17)) {}; // wait for the current dma transaction to complete
        LPC_GPDMA->C0CONFIG &= ~(1<<0);           //disable

        // End up here when the post-trigger-DMA-postfill-LLIs have completed
        NVIC_DisableIRQ(DMA_IRQn);
        NVIC_DisableIRQ(VADC_IRQn);

        // update sample buffer with correct positions
        //pSampleBuffer->empty = (circbuff_num_samples < circbuff_sample_limit)?TRUE:FALSE;
        pSampleBuffer->last = LPC_GPDMA->C0DESTADDR - ((uint32_t)circbuff_addr) + 4; //+4 as the DMA address is already used

        if (activeCfg.forcedTrigger)
        {
          triggeredSampleAddr = 0; // forced trigger means the trigger is the first sample
        }
        else
        {
          // Convert the triggeredSampleAddr (which holds the address currently copied to at the
          // time the trigger was found) into a address offset in the straightened-out circular
          // buffer
          triggeredSampleAddr = circbuff_ConvertAddress(pSampleBuffer, triggeredSampleAddr);
        }

        // time to send to the PC
        capture_ReportVADCDone(
          pSampleBuffer,
          0, // <-- ch that caused trigger, probably useless for VADC?
          triggeredSampleAddr / activeCfg.sample_size, // <-- want sample index, not an address
          activeCfg.from_client.enabledChannels | (activeCfg.numEnabledChannels << 16));
      }
    }
    else
    {
      if (activeCfg.prefillBuffersToFill == 0)
      {
        CAP_PREFILL_MARK_VADC_DONE();
        
        LPC_VADC->CLR_STAT1 = STATUS1_CLEAR_MASK;  // clear interrupt status
        LPC_VADC->SET_EN1 = Interrupt1Mask;
      }
      else
      {
        activeCfg.prefillBuffersToFill--;
      }
      pSampleBuffer->empty = FALSE;
    }
  }
  CLR_MEAS_PIN_3();
}

/**************************************************************************//**
 *
 * @brief  Interrupt handler for the VADC's interrupts
 *
 * The interrupt handler processes only the VADC's STATUS_1 interrupt which
 * has the status bit for both channels' threshold crossings.
 *
 * This function is used when the client has not selected to use noise 
 * reduction. Only one threshold is used and this function considers a trigger 
 * to be found the first time that threshold is crossed in the correct direction.
 *
 * When a trigger is found the DMA's current destination address is saved and 
 * SGPIO is notified (in case digital sampling is done in parallel).
 * \ref cap_vadc_Triggered modifies the DMA's LLI to stop after the desired
 * post fill and then the sampling continues.
 *     
 *****************************************************************************/
void VADC_IRQHandler_Normal(void)
{
  SET_MEAS_PIN_2();

  if (triggered)
  {
    return;
  }
  if (!CAP_PREFILL_IS_PREFILL_DONE())
  {
    return;
  }

  //              --
  //             /  \
  //          1 /    \ 2
  // ----------*------*-------------- THR_LOW_*
  //          /        \
  //         /          \  /
  //       --            --
  //
  // Without noise reduction only one threshold is used and this interrupt
  // indicates the crossing of that threshold (pos 1 for rising and pos 2 for
  // falling edge).

  NVIC_DisableIRQ(VADC_IRQn);

  // Found a trigger which means:
  // 1) Stop looking for triggers
  // 2) Calculate how many more samples to collect
  // 3) Save trigger position
  cap_vadc_Triggered();

  // In case both VADC and SGPIO are being sampled, notify SGPIO as well
  cap_sgpio_Triggered();

  CLR_MEAS_PIN_2();
}

/**************************************************************************//**
 *
 * @brief  Interrupt handler for the VADC's interrupts
 *
 * The interrupt handler processes only the VADC's STATUS_1 interrupt which
 * has the status bit for both channels' threshold crossings.
 *
 * This handler is used when the client has requested noise reduction for 
 * the triggering and it will process the interrupts for two threshold levels 
 * and a trigger is only found if both levels are crossed in sequence. 
 * This is a bit more resource heavy but can elminate a lot of the false 
 * triggers.
 *
 * When a trigger is found the DMA's current destination address is saved and 
 * SGPIO is notified (in case digital sampling is done in parallel).
 * \ref cap_vadc_Triggered modifies the DMA's LLI to stop after the desired
 * post fill and then the sampling continues.
 *     
 *****************************************************************************/
void VADC_IRQHandler_NoiseReduction(void)
{
  uint32_t tmp;

  SET_MEAS_PIN_2();

  tmp = LPC_VADC->STATUS1;
  LPC_VADC->CLR_STAT1 = STATUS1_CLEAR_MASK;

  if (triggered)
  {
    return;
  }
  if (!CAP_PREFILL_IS_PREFILL_DONE())
  {
    return;
  }

  if ((noiseReductionCounter == 0) && ((tmp & noiseReductionMask) == noiseReductionValue1))
  {
    //             --
    //          2 /  \ 3
    // ----------*----*-------------- THR_HIGH_*
    //          /      \
    //       1 /        \ 4
    // -------*----------*----------- THR_LOW_*
    //       /            \  /
    //     --              --
    //
    // If looking for falling edge: Just found pos 3 - the downward crossing of THR_HIGH_*
    // If looking for rising edge:  Just found pos 1 - the upward crossing of THR_LOW_*
    noiseReductionCounter++;
    CLR_MEAS_PIN_2();
    return;
  }
  else if ((noiseReductionCounter == 1) && ((tmp & noiseReductionMask) == noiseReductionValue2))
  {
    //             --
    //          2 /  \ 3
    // ----------*----*-------------- THR_HIGH_*
    //          /      \
    //       1 /        \ 4
    // -------*----------*----------- THR_LOW_*
    //       /            \  /
    //     --              --
    //
    // If looking for falling edge: Just found pos 4 - the downward crossing of THR_LOW_*
    // If looking for rising edge:  Just found pos 2 - the upward crossing of THR_HIGH_*

    // This means that the trigger is found
  }
  else
  {
    //             --       -
    //          2 /  \   3 /
    // ----------*----\---*---------     ----------------------------- THR_HIGH_*
    //          /      \ /                        --
    //       1 /        -                        /  \ 4
    // -------*---------------------     -------/----*---------------- THR_LOW_*
    //       /                                 /      \
    //     --                                --
    //
    // Found an unwanted crossing of a threshold. The drawing shows 4 possible invalid
    // crossings when looking for a falling edge.

    noiseReductionCounter = 0;
    CLR_MEAS_PIN_2();
    return;
  }

  NVIC_DisableIRQ(VADC_IRQn);

  // Found a trigger which means:
  // 1) Stop looking for triggers
  // 2) Calculate how many more samples to collect
  // 3) Save trigger position
  LPC_VADC->CLR_EN1 = Interrupt1Mask;
  cap_vadc_Triggered();

  // In case both VADC and SGPIO are being sampled, notify SGPIO as well
  cap_sgpio_Triggered();

  CLR_MEAS_PIN_2();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Reconfigures and enables DMA
 *
 * VADC sampling uses DMA channel 0 to copy the samples from the VADC FIFO into
 * the circular capture buffer. The transfers are done under DMA's flow control
 * and by using a linked list of dma items (LLI). To setup the LLIs the available
 * capture buffer is divided into DMA_NUM_LLI_TO_USE chunks (the last chunk has
 * a slightly larger/smaller size to make sure the entire capture buffer is 
 * utilized). Each LLI is then assigned a chunk and is linked to the next LLI.
 * The last LLI is linked to the first one to create a circular buffer. The last
 * LLI is also the only LLI setup to cause a terminal count (TC) interrupt when
 * completed. That interrupt is used to detect when the buffer is completely
 * filled and to detect when the prefill phase is completed.
 *
 * The LLI remains unmodified until a trigger is found at which time the linked
 * list is broken by \ref cap_vadc_Triggered to end the capture after a specified
 * amount of post fill.
 *
 *****************************************************************************/
static void VADC_SetupDMA(void)
{
  int i;
  uint32_t defaultTransferSize;

  NVIC_DisableIRQ(DMA_IRQn);
  LPC_GPDMA->C0CONFIG = 0;

  /* clear all interrupts on channel 0 */
  LPC_GPDMA->INTTCCLEAR = 0x01;
  LPC_GPDMA->INTERRCLR = 0x01;

  /* Setup the DMAMUX */
  LPC_CREG->DMAMUX &= ~(0x3<<(VADC_DMA_WRITE*2));
  LPC_CREG->DMAMUX |= 0x3<<(VADC_DMA_WRITE*2);  /* peripheral 7 vADC Write(0x3) */
  LPC_CREG->DMAMUX &= ~(0x3<<(VADC_DMA_READ*2));
  LPC_CREG->DMAMUX |= 0x3<<(VADC_DMA_READ*2);  /* peripheral 8 vADC read(0x3) */

  LPC_GPDMA->CONFIG = 0x01;  /* Enable DMA channels, little endian */
  while ( !(LPC_GPDMA->CONFIG & 0x01) );

  // The size of the transfer is in multiples of 32bit copies (hence the /4)
  // and must be even multiples of FIFO_SIZE.
  defaultTransferSize = pSampleBuffer->size / (FIFO_SIZE * DMA_NUM_LLI_TO_USE);
  defaultTransferSize = (defaultTransferSize * FIFO_SIZE) / 4;

  for (i = 0; i < DMA_NUM_LLI_TO_USE; i++)
  {
    uint32_t transSize = defaultTransferSize;
    if (i == (DMA_NUM_LLI_TO_USE - 1))
    {
      // Add the leftover (due to the need for the transfer size to be an even
      // multiple of FIFO_SIZE) to the last LLI
      transSize += (pSampleBuffer->size % (defaultTransferSize*4))/4;
    }
    DMA_Stuff[i].SrcAddr = VADC_DMA_READ_SRC;
    DMA_Stuff[i].DstAddr = ((uint32_t)circbuff_addr) + defaultTransferSize*4*i;
    DMA_Stuff[i].NextLLI = (uint32_t)(&DMA_Stuff[(i+1)%DMA_NUM_LLI_TO_USE]);
    DMA_Stuff[i].Control = (transSize << 0) |      // Transfersize (does not matter when flow control is handled by peripheral)
                           (0x2 << 12)  |          // Source Burst Size
                           (0x2 << 15)  |          // Destination Burst Size
                           (0x2 << 18)  |          // Source width // 32 bit width
                           (0x2 << 21)  |          // Destination width   // 32 bits
                           (0x1 << 24)  |          // Source AHB master 0 / 1
                           (0x1 << 25)  |          // Dest AHB master 0 / 1
                           (0x0 << 26)  |          // Source increment(LAST Sample)
                           (0x1 << 27)  |          // Destination increment
                           (0x0UL << 31);          // Terminal count interrupt disabled
    //log_i("DMA_Stuff[%d] on address %#x, destination %#x, transfer size %#x (%d)\r\n", i, (uint32_t)&DMA_Stuff[i], DMA_Stuff[i].DstAddr, transSize, transSize);
  }
//   log_i("Post FILL (%d LLIs)will be between %d (%%%d) and %d (%%%d) samples\r\n",
//         post_fill_llis,
//         ((post_fill_llis+0)*defaultTransferSize*4),
//         ((post_fill_llis+0)*defaultTransferSize*4*100)/pSampleBuffer->size,
//         ((post_fill_llis+1)*defaultTransferSize*4),
//         ((post_fill_llis+1)*defaultTransferSize*4*100)/pSampleBuffer->size);

  // Let the last LLI in the chain cause a terminal count interrupt to
  // notify when the capture buffer is completely filled
  DMA_Stuff[DMA_NUM_LLI_TO_USE - 1].Control |= (0x1UL << 31); // Terminal count interrupt enabled

  LPC_GPDMA->C0SRCADDR = DMA_Stuff[0].SrcAddr;
  LPC_GPDMA->C0DESTADDR = DMA_Stuff[0].DstAddr;
  LPC_GPDMA->C0CONTROL = DMA_Stuff[0].Control;
  LPC_GPDMA->C0LLI     = (uint32_t)(&DMA_Stuff[1]); // must be pointing to the second LLI as the first is used when initializing
  LPC_GPDMA->C0CONFIG  =  (0x1)        |          // Enable bit
                          (VADC_DMA_READ << 1) |  // SRCPERIPHERAL - set to 8 - VADC
                          (0x0 << 6)   |          // Destination peripheral - memory - no setting
                          (0x2 << 11)  |          // Flow control - peripheral to memory - DMA control
                          (0x1 << 14)  |          // Int error mask
                          (0x1 << 15);            // ITC - term count error mask

  NVIC_EnableIRQ(DMA_IRQn);
}

/**************************************************************************//**
 *
 * @brief  Configures triggering conditions
 *
 * Sets the global variables \ref Interrupt1Mask, \ref noiseReductionMask,
 * \ref noiseReductionValue1 and \ref noiseReductionValue2 depending on
 * rising or falling edge and wether or not noise reduction filter is requested
 * or not.
 *
 * @param [in] ch           Which channel to modify
 * @param [in] triggerType  0 or 1 (see description of triggerSetup in \ref cap_vadc_cfg_t)
 *
 *****************************************************************************/
static void VADC_SetupTriggerInterrupt(int ch, uint32_t triggerType)
{
  // triggerType should be 00 or 01 as described in the triggerSetup member
  // of cap_vadc_cfg_t (see capture_vadc.h)

  // Here is where it gets complicated. The captured signal is inverted compared
  // which is later corrected for with the calibration information (done on the
  // client side) but that means that a Falling Edge trigger must be treated as
  // a Rising Edge and the other way around.

  if (triggerType == 1)
  {
    // Falling => want downward threshold crossing. Due to inverted value we look
    //            for upwards crossing (Rising)
    Interrupt1Mask |= STATUS1_THCMP_UCROSS(ch);
    if (NOISE_REDUCTION_ENABLED(activeCfg.from_client.noiseReduction))
    {
      // Want interrupts from opposite direction as well, to cancel out
      // faulty triggers
      Interrupt1Mask |= STATUS1_THCMP_DCROSS(ch);

      // Rising means look at "upward crossing" and "above level" flags
      // First value should be "upward crossing" but not "above level"
      // Second value should be both "upward crossing" and "above level"
      noiseReductionMask = STATUS1_THCMP_UCROSS(ch) | STATUS1_THCMP_ARANGE(ch);
      noiseReductionValue1 = STATUS1_THCMP_UCROSS(ch);
      noiseReductionValue2 = STATUS1_THCMP_UCROSS(ch) | STATUS1_THCMP_ARANGE(ch);
    }
  }
  else
  {
    // Rising => want upward threshold crossing. Due to inverted value we look
    //            for downwards crossing (Falling)
    Interrupt1Mask |= STATUS1_THCMP_DCROSS(ch);
    if (NOISE_REDUCTION_ENABLED(activeCfg.from_client.noiseReduction))
    {
      // Want interrupts from opposite direction as well, to cancel out
      // faulty triggers
      Interrupt1Mask |= STATUS1_THCMP_UCROSS(ch);

      // Falling means look at "downward crossing" and "below level" flags
      // First value should be "downward crossing" but not "below level"
      // Second value should be both "downward crossing" and "below level"
      noiseReductionMask = STATUS1_THCMP_DCROSS(ch) | STATUS1_THCMP_BRANGE(ch);
      noiseReductionValue1 = STATUS1_THCMP_DCROSS(ch);
      noiseReductionValue2 = STATUS1_THCMP_DCROSS(ch) | STATUS1_THCMP_BRANGE(ch);
    }
  }
}

/**************************************************************************//**
 *
 * @brief  Prepares VADC for a new capture
 *
 * Resets the VADC block, disables interrupts and then initializes VADC
 * with the \ref activeCfg data.
 *
 *****************************************************************************/
static void VADC_Init(void)
{
  uint32_t tmp;
  uint32_t thr_ch_1 = 0;
  uint32_t thr_ch_2 = 0;

  // Reset the VADC block
  RGU_SoftReset(RGU_SIG_VADC);
  while(RGU_GetSignalStatus(RGU_SIG_VADC));

  // Clear FIFO
  //LPC_VADC->FLUSH = 1;

  // Disable the VADC interrupt
  NVIC_DisableIRQ(VADC_IRQn);
  LPC_VADC->CLR_EN0 = STATUS0_CLEAR_MASK;         // disable interrupt0
  LPC_VADC->CLR_STAT0 = STATUS0_CLEAR_MASK;       // clear interrupt status
  while(LPC_VADC->STATUS0 & 0x7d);  // wait for status to clear, have to exclude FIFO_EMPTY (bit 1)
  LPC_VADC->CLR_EN1 = STATUS1_CLEAR_MASK;          // disable interrupt1
  LPC_VADC->CLR_STAT1 = STATUS1_CLEAR_MASK;  // clear interrupt status
  while(LPC_VADC->STATUS1);         // wait for status to clear

  triggered             = 0;
  triggeredSampleAddr   = 0;
  started               = FALSE;

  noiseReductionEnabled = 0;
  noiseReductionCounter = 0;

  // Make sure the VADC is not powered down
  LPC_VADC->POWER_DOWN =
    (0<<0);        /* PD_CTRL:      0=disable power down, 1=enable power down */

  // Clear FIFO
  LPC_VADC->FLUSH = 1;

  // FIFO Settings
  LPC_VADC->FIFO_CFG =
    (1<<0) |         /* PACKED_READ:      0= 1 sample packed into 32 bit, 1= 2 samples packed into 32 bit */
    (FIFO_SIZE<<1);  /* FIFO_LEVEL:       When FIFO contains this or more samples raise FIFO_FULL irq and DMA_Read_Req, default is 8 */

  // Descriptors:
  if (activeCfg.matchValue == 0)
  {
    // A matchValue of 0 requires special handling to prevent a automatic start.
    // For more information see the "Appendix A Errata" of the VADC manual.
    LPC_VADC->DSCR_STS =
      (1<<0) |       /* ACT_TABLE:        0=table 0 is active, 1=table 1 is active */
      (0<<1);        /* ACT_DESCRIPTOR:   ID of the descriptor that is active */

    LPC_VADC->DESCRIPTOR_1[0] =
      (0<<0) |       /* CHANNEL_NR:    0=convert input 0, 1=convert input 1, ..., 5=convert input 5 */
      (0<<3) |       /* HALT:          0=continue with next descriptor after this one, 1=halt after this and restart at a new trigger */
      (0<<4) |       /* INTERRUPT:     1=raise interrupt when ADC result is available */
      (0<<5) |       /* POWER_DOWN:    1=power down after this conversion */
      (2<<6) |       /* BRANCH:        0=continue with next descriptor (wraps around after top) */
                     /*                1=branch to the first descriptor in this table */
                     /*                2=swap tables and branch to the first descriptor of the new table */
                     /*                3=reserved (do not store sample). continue with next descriptor (wraps around the top) */
      (1<<8)  |      /* MATCH_VALUE:   Evaluate this desciptor when descriptor timer value is equal to match value */
      (0<<22) |      /* THRESHOLD_SEL: 0=no comparison, 1=THR_A, 2=THR_B */
      (1<<24) |      /* RESET_TIME:    1=reset descriptor timer */
      (1UL<<31);     /* UPDATE_TABLE:  1=update table with all 8 descriptors of this table */
  }
  else
  {
    LPC_VADC->DSCR_STS =
      (0<<0) |       /* ACT_TABLE:        0=table 0 is active, 1=table 1 is active */
      (0<<1);        /* ACT_DESCRIPTOR:   ID of the descriptor that is active */
  }

  LPC_VADC->CONFIG = /* configuration register */
    (1<<0) |        /* TRIGGER_MASK:     0=triggers off, 1=SW trigger, 2=EXT trigger, 3=both triggers */
    (0<<2) |        /* TRIGGER_MODE:     0=rising, 1=falling, 2=low, 3=high external trigger */
    (0<<4) |        /* TRIGGER_SYNC:     0=no sync, 1=sync external trigger input */
    (1<<5) |        /* CHANNEL_ID_EN:    0=don't add, 1=add channel id to FIFO output data */
    (0x90<<6);      /* RECOVERY_TIME:    ADC recovery time from power down, default is 0x90 */

  if (activeCfg.from_client.enabledTriggers & 1)
  {
    thr_ch_1 = 1; // Use THR_A
    tmp = activeCfg.from_client.triggerSetup & 0xfff;
    if (NOISE_REDUCTION_ENABLED(activeCfg.from_client.noiseReduction))
    {
      noiseReductionEnabled = 1;
      LPC_VADC->THR_A =
        ((tmp - NOISE_REDUCTION_LEVEL(activeCfg.from_client.noiseReduction))<<0) |   /* THR_LOW_A:        Low compare threashold register A, default 0x000 */
        ((tmp + NOISE_REDUCTION_LEVEL(activeCfg.from_client.noiseReduction))<<16);   /* THR_HIGH_A:       High compare threashold register A, default 0xfff */
    }
    else
    {
      LPC_VADC->THR_A =
        (tmp<<0) |     /* THR_LOW_A:      Low compare threashold register A, default 0x000 */
        (tmp<<16);     /* THR_HIGH_A:     High compare threashold register A, default 0xfff */
    }
  }
  if (activeCfg.from_client.enabledTriggers & 2)
  {
    thr_ch_2 = 2; // Use THR_B
    tmp = (activeCfg.from_client.triggerSetup >> 16) & 0xfff;
    if (NOISE_REDUCTION_ENABLED(activeCfg.from_client.noiseReduction))
    {
      noiseReductionEnabled = 1;
      LPC_VADC->THR_B =
        ((tmp - NOISE_REDUCTION_LEVEL(activeCfg.from_client.noiseReduction))<<0) |   /* THR_LOW_B:        Low compare threashold register A, default 0x000 */
        ((tmp + NOISE_REDUCTION_LEVEL(activeCfg.from_client.noiseReduction))<<16);   /* THR_HIGH_B:       High compare threashold register A, default 0xfff */
    }
    else
    {
      LPC_VADC->THR_B =
        (tmp<<0) |     /* THR_LOW_B:        Low compare threashold register B, default 0x000 */
        (tmp<<16);     /* THR_HIGH_B:       High compare threashold register B, default 0xfff */
    }
  }


  // both VADC1 and VADC2 are enabled
  if (activeCfg.from_client.enabledChannels == 0x3)
  {
    LPC_VADC->DESCRIPTOR_0[0] =
      (LAST_CH_1<<0) |       /* CHANNEL_NR:    0=convert input 0, 1=convert input 1, ..., 5=convert input 5 */
      (0<<3) |       /* HALT:          0=continue with next descriptor after this one, 1=halt after this and restart at a new trigger */
      (0<<4) |       /* INTERRUPT:     1=raise interrupt when ADC result is available */
      (0<<5) |       /* POWER_DOWN:    1=power down after this conversion */
      (0<<6) |       /* BRANCH:        0=continue with next descriptor (wraps around after top) */
                     /*                1=branch to the first descriptor in this table */
                     /*                2=swap tables and branch to the first descriptor of the new table */
                     /*                3=reserved (do not store sample). continue with next descriptor (wraps around the top) */
      (activeCfg.matchValue<<8)  |    /* MATCH_VALUE:   Evaluate this desciptor when descriptor timer value is equal to match value */
      (thr_ch_1<<22) |      /* THRESHOLD_SEL: 0=no comparison, 1=THR_A, 2=THR_B */
      (1<<24) |      /* RESET_TIME:    1=reset descriptor timer */
      (0UL<<31);       /* UPDATE_TABLE:  1=update table with all 8 descriptors of this table */
    LPC_VADC->DESCRIPTOR_0[1] =
      (LAST_CH_2<<0) |       /* CHANNEL_NR:    0=convert input 0, 1=convert input 1, ..., 5=convert input 5 */
      (0<<3) |       /* HALT:          0=continue with next descriptor after this one, 1=halt after this and restart at a new trigger */
      (0<<4) |       /* INTERRUPT:     1=raise interrupt when ADC result is available */
      (0<<5) |       /* POWER_DOWN:    1=power down after this conversion */
      (1<<6) |       /* BRANCH:        0=continue with next descriptor (wraps around after top) */
                     /*                1=branch to the first descriptor in this table */
                     /*                2=swap tables and branch to the first descriptor of the new table */
                     /*                3=reserved (do not store sample). continue with next descriptor (wraps around the top) */
      (activeCfg.matchValue<<8)  |    /* MATCH_VALUE:   Evaluate this desciptor when descriptor timer value is equal to match value */
      (thr_ch_2<<22) |      /* THRESHOLD_SEL: 0=no comparison, 1=THR_A, 2=THR_B */
      (1<<24) |      /* RESET_TIME:    1=reset descriptor timer */
      (1UL<<31);       /* UPDATE_TABLE:  1=update table with all 8 descriptors of this table */
  }

  // only VADC1 enabled
  else if (activeCfg.from_client.enabledChannels == 0x1)
  {
    LPC_VADC->DESCRIPTOR_0[0] =
      (LAST_CH_1<<0) |       /* CHANNEL_NR:    0=convert input 0, 1=convert input 1, ..., 5=convert input 5 */
      (0<<3) |       /* HALT:          0=continue with next descriptor after this one, 1=halt after this and restart at a new trigger */
      (0<<4) |       /* INTERRUPT:     1=raise interrupt when ADC result is available */
      (0<<5) |       /* POWER_DOWN:    1=power down after this conversion */
      (1<<6) |       /* BRANCH:        0=continue with next descriptor (wraps around after top) */
                     /*                1=branch to the first descriptor in this table */
                     /*                2=swap tables and branch to the first descriptor of the new table */
                     /*                3=reserved (do not store sample). continue with next descriptor (wraps around the top) */
      (activeCfg.matchValue<<8)  |    /* MATCH_VALUE:   Evaluate this desciptor when descriptor timer value is equal to match value */
      (thr_ch_1<<22) |      /* THRESHOLD_SEL: 0=no comparison, 1=THR_A, 2=THR_B */
      (1<<24) |      /* RESET_TIME:    1=reset descriptor timer */
      (1UL<<31);       /* UPDATE_TABLE:  1=update table with all 8 descriptors of this table */
  }

  // only VADC2 enabled
  else
  {
    LPC_VADC->DESCRIPTOR_0[0] =
      (LAST_CH_2<<0) |       /* CHANNEL_NR:    0=convert input 0, 1=convert input 1, ..., 5=convert input 5 */
      (0<<3) |       /* HALT:          0=continue with next descriptor after this one, 1=halt after this and restart at a new trigger */
      (0<<4) |       /* INTERRUPT:     1=raise interrupt when ADC result is available */
      (0<<5) |       /* POWER_DOWN:    1=power down after this conversion */
      (1<<6) |       /* BRANCH:        0=continue with next descriptor (wraps around after top) */
                     /*                1=branch to the first descriptor in this table */
                     /*                2=swap tables and branch to the first descriptor of the new table */
                     /*                3=reserved (do not store sample). continue with next descriptor (wraps around the top) */
      (activeCfg.matchValue<<8)  |    /* MATCH_VALUE:   Evaluate this desciptor when descriptor timer value is equal to match value */
      (thr_ch_2<<22) |      /* THRESHOLD_SEL: 0=no comparison, 1=THR_A, 2=THR_B */
      (1<<24) |      /* RESET_TIME:    1=reset descriptor timer */
      (1UL<<31);       /* UPDATE_TABLE:  1=update table with all 8 descriptors of this table */
  }

  LPC_VADC->ADC_SPEED =
    activeCfg.DGECi;   /* DGECx:      For CRS=3 all should be 0xF, for CRS=4 all should be 0xE, */
                       /*             for all other cases it should be 0 */

  LPC_VADC->POWER_CONTROL =
    (activeCfg.crs << 0) |    /* CRS:          current setting for power versus speed programming */
    (1 << 4) |      /* DCINNEG:      0=no dc bias, 1=dc bias on vin_neg slide */
    (0 << 10) |     /* DCINPOS:      0=no dc bias, 1=dc bias on vin_pos slide */
    (0 << 16) |     /* TWOS:         0=offset binary, 1=two's complement */
    (1 << 17) |     /* POWER_SWITCH: 0=ADC is power gated, 1=ADC is active */
    (1 << 18);      /* BGAP_SWITCH:  0=ADC bandgap reg is power gated, 1=ADC bandgap is active */


  // Setup correct interrupt handler
  if (noiseReductionEnabled)
  {
    // Change IRQ handler by manipulating the VectorTable, code from
    // lpc43xx.git:\Examples\SGPIO\SGPIO_DMA_PatternGenerator\setup_M0.c
    uint32_t *VectorTable_ptr_M4;
    VectorTable_ptr_M4 = ((uint32_t *) 0x10000000);
    VectorTable_ptr_M4[61] = ((uint32_t)VADC_IRQHandler_NoiseReduction);
  }
  else
  {
    // Change IRQ handler by manipulating the VectorTable, code from
    // lpc43xx.git:\Examples\SGPIO\SGPIO_DMA_PatternGenerator\setup_M0.c
    uint32_t *VectorTable_ptr_M4;
    VectorTable_ptr_M4 = ((uint32_t *) 0x10000000);
    VectorTable_ptr_M4[61] = ((uint32_t)VADC_IRQHandler_Normal);
  }
  

  // Enable interrupts
  NVIC_EnableIRQ(VADC_IRQn);

  // Determine which threshold interrupt bits are needed
  Interrupt1Mask = 0;
  if (activeCfg.from_client.enabledTriggers & 1)
  {
    VADC_SetupTriggerInterrupt(LAST_CH_1, ((activeCfg.from_client.triggerSetup >> 14) & 0x3));
  }
  if (activeCfg.from_client.enabledTriggers & 2)
  {
    VADC_SetupTriggerInterrupt(LAST_CH_2, ((activeCfg.from_client.triggerSetup >> 30) & 0x3));
  }

//  LPC_VADC->SET_EN0 = STATUS0_FIFO_FULL_MASK;// only care about FIFO_FULL

  VADC_SetupDMA();

  // If no triggers are selected then just capture enough to fill the
  // buffer once and report that back to the UI. This is called "forced trigger".
  if (activeCfg.forcedTrigger)
  {
    //circbuff_last_sample = circbuff_sample_limit - FIFO_SIZE;
    triggered = 1;
  }
}

/**************************************************************************//**
 *
 * @brief  Stops any ongoing VADC capture
 *
 * Resets the VADC block and disables interrupts.
 *
 *****************************************************************************/
static void VADC_Stop(void)
{
  NVIC_DisableIRQ(DMA_IRQn);
  NVIC_DisableIRQ(VADC_IRQn);

  // disable DMA
  LPC_GPDMA->C0CONFIG |= (1 << 18); //halt further requests

  // power down VADC
  LPC_VADC->POWER_CONTROL = 0;

  // Clear FIFO
  LPC_VADC->FLUSH = 1;

  // Disable the VADC interrupt
  LPC_VADC->CLR_EN0 = STATUS0_CLEAR_MASK;         // disable interrupt0
  LPC_VADC->CLR_STAT0 = STATUS0_CLEAR_MASK;       // clear interrupt status
//  while(LPC_VADC->STATUS0 & 0x7d);  // wait for status to clear, have to exclude FIFO_EMPTY (bit 1)
  LPC_VADC->CLR_EN1 = STATUS1_CLEAR_MASK;          // disable interrupt1
  LPC_VADC->CLR_STAT1 = STATUS1_CLEAR_MASK;  // clear interrupt status
//  while(LPC_VADC->STATUS1);         // wait for status to clear

  // Reset the VADC block
  RGU_SoftReset(RGU_SIG_VADC);
  while(RGU_GetSignalStatus(RGU_SIG_VADC));
  
  // Make sure that the next time the VADC is started it gets enough time
  // to "boot up"
  activeCfg.prefillBuffersToFill = activeCfg.prefillBuffersToRestore;
}

/**************************************************************************//**
 *
 * @brief  Calculates the number of samples to collect before/after the trigger is found
 *
 * Post fill configuration. The lower 8 bits specify the percent of the
 * maximum buffer size that will be used for samples taken AFTER the trigger.
 * The upper 24 bits specifies the maximum number of samples to gather after
 * a trigger has been found.
 *
 * After finding out the number of samples it is converted into the number of
 * DMA LLIs to use and that information is stored in \ref post_fill_llis.
 *
 * Pre fill is the number of samples to collect before accepting a trigger.
 * This is simplified into a number of complete fills of the circular buffer.
 * The number is based on \ref PREFILL_TIME_IN_MS and is stored in the 
 * prefillBuffersToFill member of the \ref internal_vadc_cfg_t structure.
 *
 * @param [in] postFill   The post fill configuration
 *
 * @retval CMD_STATUS_OK                           If successfully configured
 * @retval CMD_STATUS_ERR_INVALID_POSTFILLPERCENT  If the level is invalid
 *
 *****************************************************************************/
static cmd_status_t VADC_CalculatePreAndPostFill(uint32_t postFill)
{
  uint32_t postFillPercent = postFill & 0xff;
  uint32_t postFillSamples = (postFill >> 8) & 0xffffff;
  uint32_t circbuff_post_fill;

  if (postFillPercent > 100)
  {
    return CMD_STATUS_ERR_INVALID_POSTFILLPERCENT;
  }

  // Apply percent limit
  circbuff_post_fill = (circbuff_sample_limit * postFillPercent)/100;

  // Apply time limit
  circbuff_post_fill = MIN(circbuff_post_fill, postFillSamples);

//   // Need at least one sample after the trigger is found
//   circbuff_post_fill = MAX(1, circbuff_post_fill);

//   // Need at least five samples before the trigger is found. This is not
//   // confirmed, but used for the SGPIO capture so the same value is used here.
//   circbuff_post_fill = MIN(circbuff_post_fill, circbuff_sample_limit - 5);

  // Convert to the number of extra LLIs to use
  post_fill_llis = (DMA_NUM_LLI_TO_USE * circbuff_post_fill) / circbuff_sample_limit;
  post_fill_llis = MAX(post_fill_llis, 1);
  post_fill_llis = MIN(post_fill_llis, DMA_NUM_LLI_TO_USE - 2);


  // Calculate how many complete buffers must be filled to reach the PREFILL_TIME_IN_MS
  // level. The amount depends on the available buffer size and sample rate. For low rates
  // and or small buffers this value will be 0 meaning a prefill of one complete buffer.
  // For 80MHz and only one VADC channel it will mean ca 240 complete buffer fills.
  //
  // This value is set during configuration and will only affect the first sampling after
  // the configuration has changed. Repeated captures will start with a value of 0, meaning
  // that the triggering can start once the buffer has been completely filled once.
  activeCfg.prefillBuffersToRestore = (capture_GetSampleRate() / 1000) * PREFILL_TIME_IN_MS;
  activeCfg.prefillBuffersToRestore = activeCfg.prefillBuffersToRestore / circbuff_sample_limit;
  activeCfg.prefillBuffersToFill = activeCfg.prefillBuffersToRestore;

  return CMD_STATUS_OK;
}


/**************************************************************************//**
 *
 * @brief  Sets up the Volts/div
 *
 * Controls the \ref CTRL_CH1_GN0, \ref CTRL_CH1_GN1, \ref CTRL_CH1_GN2, 
 * \ref CTRL_CH2_GN0, \ref CTRL_CH2_GN1, \ref CTRL_CH2_GN2 pins of the 
 * U22 shift register.
 *
 * @param [in] cfg   The configuration
 *
 * @retval CMD_STATUS_OK                 If successfully configured
 * @retval CMD_STATUS_ERR_INVALID_VDIV   If the level is invalid
 *
 *****************************************************************************/
static cmd_status_t VADC_SetupVoltsPerDiv(const cap_vadc_cfg_t* const cfg)
{
  static uint32_t lastValue = 0x100;
  static uint32_t lastCh    = 0;

  if (cfg->voltPerDiv != lastValue || lastCh != cfg->enabledChannels)
  {
    uint32_t val = 0;
    int ch;
    for (ch = 0; ch < 2; ch++)
    {
      if (cfg->enabledChannels & (1<<ch))
      {
        uint8_t idx = (cfg->voltPerDiv >> (ch*4)) & 0xf;
        if (idx >= 8)
        {
          return CMD_STATUS_ERR_INVALID_VDIV;
        }
        val |= VDIV_CONFIG[idx][ch+1];
      }
      else
      {
        // A disabled channel must be set to the 5V/div setting to prevent distorsion
        // of the enabled channel.
        val |= VDIV_CONFIG[7][ch+1];
      }
    }

    spi_control_write(val, (VDIV_CH1_MASK | VDIV_CH2_MASK));

    lastValue = cfg->voltPerDiv;
    lastCh = cfg->enabledChannels;
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Sets up the AC/DC coupling
 *
 * Controls the \ref CTRL_CH1_AC_DC and \ref CTRL_CH2_AC_DC pins of the 
 * U22 shift register.
 *
 * @param [in] cfg  Configuration
 *
 * @retval CMD_STATUS_OK      If the configuration could be applied
 * @retval CMD_STATUS_ERR_*   If the setting failed
 *
 *****************************************************************************/
static cmd_status_t VADC_SetupCoupling(const cap_vadc_cfg_t* const cfg)
{
  static uint32_t lastValue = 0x100; // any invalid value will do

  if (lastValue != cfg->couplings)
  {
    uint32_t val = 0;
    if (cfg->couplings & 1)
    {
      val |= CTRL_CH1_AC_DC;
    }
    if (cfg->couplings & 2)
    {
      val |= CTRL_CH2_AC_DC;
    }
    spi_control_write(val, (CTRL_CH1_AC_DC | CTRL_CH2_AC_DC));
    lastValue = cfg->couplings;
  }
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Validates the trigger level(s)
 *
 * The trigger level must be between \ref TRIG_MIN_VALUE and \ref TRIG_MAX_VALUE.
 * If noise reduction has been enabled then the range is further reduced by
 * the noise reduction level.
 *
 * @param [in] cfg  Configuration to validate
 *
 * @retval CMD_STATUS_OK      If the trigger level is valid
 * @retval CMD_STATUS_ERR_*   When the trigger level is invalid
 *
 *****************************************************************************/
static cmd_status_t VADC_ValidateTriggerLevels(cap_vadc_cfg_t* cfg)
{
  int ch;
  uint32_t trigLvl;

  for (ch = 0; ch < 2; ch++)
  {
    if ((cfg->enabledChannels & (1<<ch)) && (cfg->enabledTriggers & (1<<ch)))
    {
      trigLvl = (cfg->triggerSetup >> (ch*16)) & 0xfff;
      if (trigLvl < TRIG_MIN_VALUE)
      {
        return CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_LOW;
      }
      else if (trigLvl > TRIG_MAX_VALUE)
      {
        return CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_HIGH;
      }
      else if (NOISE_REDUCTION_ENABLED(cfg->noiseReduction))
      {
        if ((trigLvl < (NOISE_REDUCTION_LEVEL(cfg->noiseReduction) + TRIG_MIN_VALUE)) ||
            ((trigLvl + NOISE_REDUCTION_LEVEL(cfg->noiseReduction)) > TRIG_MAX_VALUE))
        {
          return CMD_STATUS_ERR_NOISE_REDUCTION_LEVEL_TOO_HIGH;
        }
      }
    }
  }
  return CMD_STATUS_OK;
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Enables the clock for VADC
 *
 *****************************************************************************/
void cap_vadc_Init(void)
{
  CGU_EntityConnect(CGU_CLKSRC_PLL0_AUDIO, CGU_BASE_VADC);
  CGU_EnableEntity(CGU_BASE_VADC, ENABLE);

  memset(&activeCfg, 0x00, sizeof(internal_vadc_cfg_t));
  activeCfg.valid = FALSE;
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
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t cap_vadc_Configure(circbuff_t* buff, cap_vadc_cfg_t* cfg, uint32_t postFill, Bool forceTrigger)
{
  uint32_t fADC;
  cmd_status_t result = CMD_STATUS_OK;

  pSampleBuffer = buff;
  activeCfg.valid = FALSE;
  activeCfg.forcedTrigger = forceTrigger;

  memcpy(&activeCfg.from_client, cfg, sizeof(cap_vadc_cfg_t));

  do
  {
    activeCfg.numEnabledChannels = 1;
    if (cfg->enabledChannels == 0x3)
    {
      activeCfg.numEnabledChannels = 2;
    }

    /* The descriptor timer is counting from 0 so the value should be one less
       than the value in the RATECONFIG table. Update the value to match the
       currently selected sample rate. */
    activeCfg.matchValue = capture_GetVadcMatchValue()/activeCfg.numEnabledChannels - 1;

    result = VADC_SetupVoltsPerDiv(cfg);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    result = VADC_SetupCoupling(cfg);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    result = VADC_ValidateTriggerLevels(cfg);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    // Calculate parameters based on sample rate
    activeCfg.crs = 0;
    activeCfg.DGECi = 0;
    fADC = capture_GetFadc();
    if (fADC <= 20000000)
    {
      activeCfg.crs = 0;
      activeCfg.DGECi = 0;
    }
    else if (fADC <= 30000000)
    {
      activeCfg.crs = 1;
      activeCfg.DGECi = 0;
    }
    else if (fADC <= 50000000)
    {
      activeCfg.crs = 2;
      activeCfg.DGECi = 0;
    }
    else if (fADC <= 65000000)
    {
      activeCfg.crs = 3;
      activeCfg.DGECi = 0xFFFFFF;
    }
    else if (fADC <= 80000000)
    {
      activeCfg.crs = 4;
      activeCfg.DGECi = 0xEEEEEE;
    }
    else
    {
      // error fADC should never be above 80MHz
      log_i("Invalid VADC sample rate %u Hz\r\n", fADC);
      result = CMD_STATUS_ERR_UNSUPPORTED_SAMPLE_RATE;
      break;
    }

    //log_i("fADC = %dHz, crs = %d, DGECi = %#x\r\n", fADC, activeCfg.crs, activeCfg.DGECi);


    // Configure the circular buffer data for use by the interrupt handler
    circbuff_addr = (uint32_t*)pSampleBuffer->data;

    // 2 bytes per sample per channel
    activeCfg.sample_size = activeCfg.numEnabledChannels * 2;

    circbuff_sample_limit = pSampleBuffer->maxSize/activeCfg.sample_size;

    // Trim the size of the circular buffer to be an even multiple of the number
    // of channels in this capture
    circbuff_Resize(pSampleBuffer, circbuff_sample_limit * activeCfg.sample_size);

    circbuff_last_addr = (uint32_t)pSampleBuffer->data + pSampleBuffer->size;

    // Determine how much of the buffer should be used for PRE- resp POST-trigger samples
    result = VADC_CalculatePreAndPostFill(postFill);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    activeCfg.valid = TRUE;

    // Delay to make sure that the SPI controlled V/div and coupling settings
    // have time to reach the correct levels
    TIM_Waitms(100);

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
cmd_status_t cap_vadc_PrepareToArm(void)
{
  if (!activeCfg.valid)
  {
    // no point in arming if the configuration is invalid
    return CMD_STATUS_ERR;
  }

  circbuff_addr = (uint32_t*)pSampleBuffer->data;

  circbuff_Reset(pSampleBuffer);

  CLR_MEAS_PIN_2();
  CLR_MEAS_PIN_3();
  VADC_Init();
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Do the actual arming (start the capture).
 *
 *****************************************************************************/
void cap_vadc_Arm(void)
{
  started = TRUE;
  // Start descriptor timer and descriptor table processing
  LPC_VADC->TRIGGER = 1;
}

/**************************************************************************//**
 *
 * @brief  Disarms (stops) the signal capturing.
 *
 * @retval CMD_STATUS_OK      If successfully stopped, or alreay stopped
 * @retval CMD_STATUS_ERR_*   If the capture could not be stopped
 *
 *****************************************************************************/
cmd_status_t cap_vadc_Disarm(void)
{
  started = FALSE;
  VADC_Stop();
  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Flags the current capture as triggered
 *
 * This function is used when a trigger has been detected in either the 
 * digital signal or by the VADC. The purpose is to immediately start with
 * the post trigger sampling.
 *
 * As this function is called from inside an interrupt handler it must be kept
 * as fast as possible.
 *
 * The post fill is achieved by modifying the DMA's LLI so that \ref post_fill_llis
 * items ahead of the current LLI the link is terminated and marked to generate
 * a terminal count interrupt.
 *
 *****************************************************************************/
void inline cap_vadc_Triggered(void)
{
  uint32_t tmp;

  // Important to only setup DMA if VADC is really enabled.
  if (started)
  {
    //SET_MEAS_PIN_3();
    triggeredSampleAddr = LPC_GPDMA->C0DESTADDR;

    // tmp will hold the address to the current LLI item
    tmp = LPC_GPDMA->C0LLI;

    // tmp will hold the index of the current LLI item. (Each LLI item is 16 bytes)
    tmp = (tmp - ((uint32_t)&DMA_Stuff[0])) / 16;

    // add enough LLIs to cover post fill
    tmp += post_fill_llis;

    // cover the case where the trigger occurs before the circular buffer is filled
    // but the post fill will cause it to be filled.
    if (tmp >= DMA_NUM_LLI_TO_USE)
    {
      pSampleBuffer->empty = FALSE;
      tmp = tmp % DMA_NUM_LLI_TO_USE;
    }

    if (tmp != (DMA_NUM_LLI_TO_USE - 1))
    {
      // remove the terminal count interrupt from the last LLI (normally used to detect
      // when the buffer is completely filled, but now it would interfer with the
      // terminal count marker used by the trigger
      DMA_Stuff[DMA_NUM_LLI_TO_USE - 1].Control &= ~(0x1UL << 31);
    }

    // Let the LLI item at the end of the post fill trigger a terminal count interrupt
    DMA_Stuff[tmp].Control |= (0x1UL << 31);
    DMA_Stuff[tmp].NextLLI = 0;

    triggered = 1;
  }
}

/**************************************************************************//**
 *
 * @brief  Returns the used current volts/div setting for a channel
 *
 * @param [in] ch  The channel to retrieve the value for
 *
 * @return The volts/div setting or 0 if not configured
 *
 *****************************************************************************/
uint32_t cap_vadc_GetMilliVoltsPerDiv(int ch)
{
  if (activeCfg.valid)
  {
    uint8_t idx = (activeCfg.from_client.voltPerDiv >> (ch*4)) & 0xf;
    return VDIV_CONFIG[idx][0];
  }
  return 0;
}
