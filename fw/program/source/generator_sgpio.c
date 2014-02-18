/*!
 * @file
 * @brief   Handles digital signal generation using SGPIO
 * @ingroup IP_SGPIO
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
#include "lpc43xx_timer.h"

#include "led.h"
#include "log.h"
#include "generator_sgpio.h"
#include "sgpio_cfg.h"
#include "meas.h"

#include <string.h> // for bitwise copy operations

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief A representation of one set of values for all 16 SGPIO slices. */
typedef struct
{
  uint32_t REG_SS_data[16];
} dma_copy_set_t;

/*! The start of the memory region available for signal generation data */
#define DMA_MEM ((dma_copy_set_t*) 0x10080000)

/*! For the continuous mode with > 32 states a temporary buffer is needed
 *  where the complete, repeatable, sequence can be placed. The size of that
 *  buffer is a maximum of 255bits * 32 = 1020 bytes. The buffer is placed
 *  at the end of the memory range.
 */
#define TMP_DEST_MEM  ((uint32_t*) 0x10089C00)

/*! For the continuous mode with > 32 states a temporary buffer is needed
 *  where the complete sequence of states can be placed once. The reason
 *  is that the sequence coming from the PC is mixed with the other enabled
 *  signals.
 */
#define TMP_SRC_MEM   ((uint32_t*) 0x10089B00)

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static sgpio_channel_config_t config[MAX_NUM_SLICES];

static Bool validConfiguration = FALSE;
static Bool running = FALSE;
static Bool singleShot = FALSE;

static uint32_t slicesToEnable = 0;

static uint32_t numDmaBuffers = 0;
static uint32_t nextDmaBuffer = 0;
static uint32_t numLeftToCopy = 0;

static uint32_t* thisDmaChannel = NULL;
static uint32_t* nextDmaChannel = NULL;

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
 * @brief  SGPIO IRQ Handler.
 * @ingroup RES_IRQ
 *
 * This interrupt handler is not named SGPIO_IRQHandler as that is already used
 * in the capture_sgpio.c file. To use this implementation instead, replace
 * the interrupt handler before starting to use it like this:
 *
 *     uint32_t *VectorTable_ptr_M4;
 *     VectorTable_ptr_M4 = ((uint32_t *) 0x10000000);
 *     VectorTable_ptr_M4[47] = ((uint32_t)gen_SGPIO_IRQ_Handler);
 *
 * This interrupt handler handles two different DMA channels for copying of
 * the generated signal. As the interrupt occurs one channel will be triggered
 * to start copying generation data to the SGPIO's REG_SS registers and then
 * the other DMA channel will be prepared for next interrupt.
 *
 *****************************************************************************/
static void gen_SGPIO_IRQ_Handler()
{
  uint32_t* swap;

  // enable next channel to start that copying
  nextDmaChannel[4] =     (0x1)        |         // Enable bit
                          (0x0 << 1)   |         // SRCPERIPHERAL
                          (0x0 << 6)   |         // Destination peripheral - memory - no setting
                          (0x0 << 11)  |         // Flow control - memory to memory - DMA control
                          (0x1 << 14)  |         // Int error mask
                          (0x1 << 15);           // ITC - term count error mask

  // clear interrupt status
  LPC_SGPIO->CTR_STATUS_1 = 0xFFFF;

  // prepare next channel
  nextDmaBuffer = nextDmaBuffer % numDmaBuffers;
  thisDmaChannel[0] = (uint32_t) &(DMA_MEM[nextDmaBuffer].REG_SS_data[0]); //CSRCADDR
  thisDmaChannel[1] = (uint32_t) &(LPC_SGPIO->REG_SS[0]);                  //CDESTADDR
  thisDmaChannel[3] =     (0x10 << 0)  |         // Transfersize
                          (0x4 << 12)  |         // Source Burst Size
                          (0x4 << 15)  |         // Destination Burst Size
                          (0x2 << 18)  |         // Source width // 32 bit width
                          (0x2 << 21)  |         // Destination width   // 32 bits -
                          (0x1 << 24)  |         // Source AHB master 0 / 1
                          (0x0 << 25)  |         // Dest AHB master 0 / 1
                          (0x1 << 26)  |         // Source increment(LAST Sample)
                          (0x1 << 27)  |         // Destination increment
                          (0x1 << 29)  |         // PROT2: bufferable access
                          (0x0UL << 31);         // Terminal count interrupt enabled

  // swap this with next
  swap = nextDmaChannel;
  nextDmaChannel = thisDmaChannel;
  thisDmaChannel = swap;
  nextDmaBuffer++;

  // 2 buffers, starts with 0 and is 1 first time here
  // 3 buffers, starts with 2 and is 1 second time here
  // 4 buffers, starts with 2 and is 1 third time here
  if (singleShot && (--numLeftToCopy == 0))
  {
    NVIC_DisableIRQ(SGPIO_IINT_IRQn);

    // Disable all slices
    LPC_SGPIO->CTRL_ENABLED &= ~0xffff;

    running = FALSE;
  }
}

static void gen_sgpio_SetupDMA(void)
{
  /* clear all interrupts on channel 2 and 3 */
  LPC_GPDMA->INTTCCLEAR = 0x0c;
  LPC_GPDMA->INTERRCLR = 0x0c;

  LPC_GPDMA->CONFIG = 0x01;  /* Enable DMA channels, little endian */
  while ( !(LPC_GPDMA->CONFIG & 0x01) );

  /* Setup GPDMA channel 2 for copying to REG_SS, src address does not matter here as it will
     be overwritten in the gen_SGPIO_IRQ_Handler() before it's first use. */
  LPC_GPDMA->C2SRCADDR = (uint32_t) &(DMA_MEM[0].REG_SS_data[0]);
  LPC_GPDMA->C2DESTADDR = (uint32_t) &(LPC_SGPIO->REG_SS[0]);
  LPC_GPDMA->C2LLI = 0;
  LPC_GPDMA->C2CONTROL =  (0x10 << 0)  |         // Transfersize
                          (0x4 << 12)  |         // Source Burst Size
                          (0x4 << 15)  |         // Destination Burst Size
                          (0x2 << 18)  |         // Source width // 32 bit width
                          (0x2 << 21)  |         // Destination width   // 32 bits -
                          (0x1 << 24)  |         // Source AHB master 0 / 1
                          (0x0 << 25)  |         // Dest AHB master 0 / 1
                          (0x1 << 26)  |         // Source increment(LAST Sample)
                          (0x1 << 27)  |         // Destination increment
                          (0x1 << 29)  |         // PROT2: bufferable access
                          (0x0UL << 31);         // Terminal count interrupt enabled
  LPC_GPDMA->C2CONFIG  =  (0x0)        |         // Enable bit
                          (0x0 << 1)   |         // SRCPERIPHERAL
                          (0x0 << 6)   |         // Destination peripheral - memory - no setting
                          (0x0 << 11)  |         // Flow control - memory to memory - DMA control
                          (0x0 << 14)  |         // Int error mask
                          (0x0 << 15);           // ITC - term count error mask

  /* Setup GPDMA channel 3 for copying to REG_SS */
  LPC_GPDMA->C3SRCADDR = (uint32_t) &(DMA_MEM[1].REG_SS_data[0]);
  LPC_GPDMA->C3DESTADDR = (uint32_t) &(LPC_SGPIO->REG_SS[0]);
  LPC_GPDMA->C3LLI = 0;
  LPC_GPDMA->C3CONTROL =  (0x10 << 0)  |         // Transfersize
                          (0x4 << 12)  |         // Source Burst Size
                          (0x4 << 15)  |         // Destination Burst Size
                          (0x2 << 18)  |         // Source width // 32 bit width
                          (0x2 << 21)  |         // Destination width   // 32 bits -
                          (0x1 << 24)  |         // Source AHB master 0 / 1
                          (0x0 << 25)  |         // Dest AHB master 0 / 1
                          (0x1 << 26)  |         // Source increment(LAST Sample)
                          (0x1 << 27)  |         // Destination increment
                          (0x1 << 29)  |         // PROT2: bufferable access
                          (0x0UL << 31);         // Terminal count interrupt enabled
  LPC_GPDMA->C3CONFIG  =  (0x0)        |         // Enable bit
                          (0x0 << 1)   |         // SRCPERIPHERAL
                          (0x0 << 6)   |         // Destination peripheral - memory - no setting
                          (0x0 << 11)  |         // Flow control - memory to memory - DMA control
                          (0x0 << 14)  |         // Int error mask
                          (0x0 << 15);           // ITC - term count error mask

  // Setup which buffer to start copying from
  nextDmaBuffer = (2 % numDmaBuffers);
  thisDmaChannel = (uint32_t*) &(LPC_GPDMA->C2SRCADDR);
  nextDmaChannel = (uint32_t*) &(LPC_GPDMA->C3SRCADDR);

  if (singleShot)
  {
    numLeftToCopy = numDmaBuffers;
  }

  NVIC_DisableIRQ(DMA_IRQn);
}

static void gen_sgpio_Setup(sgpio_channel_config_t* pConfig)
{
  int i;

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

  slicesToEnable        = 0;

  // Disable all slices
  LPC_SGPIO->CTRL_ENABLED = 0;

  for (i = 0; i < MAX_NUM_SLICES; i++)
  {
    sgpio_channel_config_t* p = &(pConfig[i]);

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

      LPC_SGPIO->REG[p->slice] = 0;
      LPC_SGPIO->REG_SS[p->slice] = DMA_MEM[0].REG_SS_data[p->slice];//p->reg_ss;

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
      LPC_SGPIO->SET_EN_1 |= p->set_en_1;

      // Pattern match interrupt for the slice
      LPC_SGPIO->SET_EN_2 |= p->set_en_2;

      // Enable the input bit match interrupt for the slice
      LPC_SGPIO->SET_EN_3 |= p->set_en_3;

      slicesToEnable |= (1 << p->slice);
    }
  }

  gen_sgpio_SetupDMA();
}

/* KEIL uVision have a _membitcpywl() function that will copy numbits of bits from pSrc to
   pDest, each with bit offsets of srcoff and destoff respectively.

   That function is not available in GNU ARM GCC or LPCXpresso so this, less effective,
   implementation is used insted. The code is only used during setup so it is not time
   critical.
*/
#if defined(__CC_ARM)
  #define gen_sgpio_copybits  _membitcpywl
#else
static void gen_sgpio_copybits(uint32_t* pDest, uint32_t* pSrc, uint32_t destoff, uint32_t srcoff, uint32_t numbits)
{
  uint32_t i;
#define GET_BIT_X(__data, __x)  ((((__data)[(__x)/32]) >> ((__x) % 32)) & 1)
  for (i = 0; i < numbits; i++)
  {
    pDest[(destoff + i)/32] |= (GET_BIT_X(pSrc, srcoff+i) << ((destoff + i) % 32));
  }
#undef GET_BIT_X
}
#endif

static cmd_status_t gen_sgpio_PrepareContinuousData(gen_sgpio_cfg_t* cfg)
{
  int slice, i;
  uint32_t tmp, mask, val, pos;

  numDmaBuffers = 0;
  if (cfg->numStates > 0 && cfg->numStates < 32)
  {
    numDmaBuffers = 2;
    for (slice = 0; slice < MAX_NUM_SLICES; slice++)
    {
      mask = (1 << cfg->numStates) -1;
      val = cfg->patterns[0][config[slice].dio] & mask;
      tmp = 0;
      pos = 0;
      if (config[slice].enabled)
      {
        for (i = cfg->numStates; i < 32; i+=cfg->numStates)
        {
          tmp = (tmp << cfg->numStates) | val;
          pos += cfg->numStates;
        }
      }
      config[slice].sgpio_mux_cfg &= (0<<11) | (0<<12);
      config[slice].sgpio_mux_cfg |=
        (0<<11) |      /* CONCAT_ENABLE:         0=external data pin, 1=concatenate data */
        (0<<12);       /* CONCAT_ORDER:          0=self loop, 1=2 slices, 2=4 slices, 3=8 slices */
      config[slice].pos = ((pos - 1) << 8) | ((pos - 1) << 0);
      config[slice].reg = tmp;
      DMA_MEM[0].REG_SS_data[slice] = tmp;
      DMA_MEM[1].REG_SS_data[slice] = tmp;
    }
  }
  else if ((cfg->numStates % 32) == 0)
  {
    numDmaBuffers = cfg->numStates/32;
    for (slice = 0; slice < MAX_NUM_SLICES; slice++)
    {
      for (i = 0; i < numDmaBuffers; i++)
      {
        if (config[slice].enabled)
        {
          DMA_MEM[i].REG_SS_data[slice] = cfg->patterns[i][config[slice].dio];
        }
        else
        {
          DMA_MEM[i].REG_SS_data[slice] = 0;
        }
      }
    }
  }
  else if (cfg->numStates < 256)
  {
    int mult;

    // Find first multiple of numSlices that goes evenly into 32bit chunks
    for (mult = 1; ((cfg->numStates * mult) % 32) > 0; mult++)
    {
    }
    numDmaBuffers = (cfg->numStates * mult)/32;
    tmp = (cfg->numStates + 31)/32;
    for (slice = 0; slice < MAX_NUM_SLICES; slice++)
    {
      if (config[slice].enabled)
      {
        // arrange the states in a sequence
        for (i = 0; i < tmp; i++)
        {
          TMP_SRC_MEM[i] = cfg->patterns[i][config[slice].dio];
        }

        // repetedly copy the states into a long sequence
        for (i = 0; i < mult; i++)
        {
          gen_sgpio_copybits(TMP_DEST_MEM, TMP_SRC_MEM, i * cfg->numStates, 0, cfg->numStates);
        }

        // move into the DMA buffers
        for (i = 0; i < numDmaBuffers; i++)
        {
          DMA_MEM[i].REG_SS_data[slice] = TMP_DEST_MEM[i];
        }
      }
      else
      {
        for (i = 0; i < numDmaBuffers; i++)
        {
          DMA_MEM[i].REG_SS_data[slice] = 0;
        }
      }
    }
  }
  else
  {
    return CMD_STATUS_ERR_GEN_INVALID_NUMBER_OF_STATES;
  }
  return CMD_STATUS_OK;
}

static cmd_status_t gen_sgpio_PrepareOneShotData(gen_sgpio_cfg_t* cfg)
{
  int slice, i;

  if (cfg->numStates == 0)
  {
    return CMD_STATUS_ERR_GEN_INVALID_NUMBER_OF_STATES;
  }

  numDmaBuffers = (cfg->numStates + 31)/32;
  for (slice = 0; slice < MAX_NUM_SLICES; slice++)
  {
    for (i = 0; i < numDmaBuffers; i++)
    {
      if (config[slice].enabled)
      {
        DMA_MEM[i].REG_SS_data[slice] = cfg->patterns[i][config[slice].dio];
      }
      else
      {
        DMA_MEM[i].REG_SS_data[slice] = 0;
      }
    }
    DMA_MEM[i].REG_SS_data[slice] = 0; // end data
  }
  // add end data buffers
  numDmaBuffers++;

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
void gen_sgpio_Init(void)
{
  /*
   * IDIVA and IDIVE are configured and enabled in generate.c.
   */

  /* connect the SGPIO block to IDIVE */
  CGU_EntityConnect(CGU_CLKSRC_IDIVE, CGU_BASE_PERIPH);
  CGU_EnableEntity(CGU_BASE_PERIPH, ENABLE);

  // Change IRQ handler by manipulating the VectorTable, code from
  // lpc43xx.git:\Examples\SGPIO\SGPIO_DMA_PatternGenerator\setup_M0.c
  uint32_t *VectorTable_ptr_M4;
  VectorTable_ptr_M4 = ((uint32_t *) 0x10000000);
  VectorTable_ptr_M4[47] = ((uint32_t)gen_SGPIO_IRQ_Handler);
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * The "force trigger mode" means that no trigger is used and instead the entire
 * capture buffer should be filled and then returned to the client.
 *
 * @param [in] cfg               Configuration to apply
 * @param [in] shiftClockPreset  Clocking information
 * @param [in] runCounter        0 for continuous signal, 1 for one shot
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t gen_sgpio_Configure(gen_sgpio_cfg_t* cfg, uint32_t shiftClockPreset, uint32_t runCounter)
{
  int i;
  cmd_status_t result;
  validConfiguration = FALSE;

  if (running)
  {
    gen_sgpio_Stop();
  }

  for (i = 0; i < MAX_NUM_SLICES; i++)
  {
    config[i].enabled = FALSE;
  }

  do
  {
    if (runCounter > 1)
    {
      result = CMD_STATUS_ERR_GEN_INVALID_RUN_COUNTER;
      break;
    }

    result = sgpio_cfg_SetupOutputChannels(config, cfg, shiftClockPreset);
    if (result != CMD_STATUS_OK)
    {
      break;
    }

    if (runCounter == 1)
    {
      singleShot = TRUE;
      result = gen_sgpio_PrepareOneShotData(cfg);
    }
    else
    {
      singleShot = FALSE;
      result = gen_sgpio_PrepareContinuousData(cfg);
    }
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
 * @brief  Starts the signal generation.
 *
 * @retval CMD_STATUS_OK      If successfully started
 * @retval CMD_STATUS_ERR_*   If the generation could not be started
 *
 *****************************************************************************/
cmd_status_t gen_sgpio_Start(void)
{
  if (!validConfiguration)
  {
    // no point in arming if the configuration is invalid
    return CMD_STATUS_ERR_NOTHING_TO_GENERATE;
  }

  gen_sgpio_Setup(config);

  // Enable the slice(s)
  LPC_SGPIO->CTRL_ENABLED |= slicesToEnable;
  LPC_SGPIO->CTRL_DISABLED &= ~slicesToEnable;

  running = TRUE;

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Disarms (stops) the signal generation.
 *
 *****************************************************************************/
void gen_sgpio_Stop(void)
{
  // Disable all slices
  LPC_SGPIO->CTRL_ENABLED &= ~0xffff;

  // Disable the capture interrupt for all slices
  LPC_SGPIO->CLR_EN_1 = 0xffff;

  // Disable the input bit match interrupt for all slices
  //LPC_SGPIO->CLR_EN_3 = 0xffff;

  // Disable the SGPIO interrupt (shared by shift/capture/match/input)
  NVIC_DisableIRQ(SGPIO_IINT_IRQn);
  LPC_SGPIO->CTR_STATUS_1 = 0xffff;    // clear capture clock interrupt status
  while(LPC_SGPIO->STATUS_1 & 0xffff); // wait for status to clear

  GPDMA_ChannelCmd(2, DISABLE);
  GPDMA_ChannelCmd(3, DISABLE);

  // Reset the SGPIO block
  //RGU_SoftReset(RGU_SIG_SGPIO);
  //while(RGU_GetSignalStatus(RGU_SIG_SGPIO));
  
  // Set all pins as inputs
  LPC_SGPIO->GPIO_OENREG = 0;

  running = FALSE;
}
