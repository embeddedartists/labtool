/*!
 * @file
 * @brief   Captures I2C communication
 * @ingroup FUNC_I2C_MON
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
#include "lpc43xx_i2c.h"
#include "lpc43xx_rgu.h"
#include "lpc43xx_gpio.h"
#include "lpc43xx_gpdma.h"
#include "lpc43xx_timer.h"

#include "led.h"
#include "log.h"
#include "meas.h"
#include "monitor_i2c.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief One sample taken from the I2C bus */
typedef struct
{
  uint32_t timestamp; /*!< timestamp in micro seconds */
  uint32_t status;    /*!< I2C status register */
  uint32_t data;      /*!< I2C data, only lowest 8 bits are used */
} sample_t;

#define SAMPLE_SIZE   (sizeof(sample_t))

/******************************************************************************
 * Global variables
 *****************************************************************************/

/******************************************************************************
 * Local variables
 *****************************************************************************/

static TIM_TIMERCFG_Type timerCfg;

static circbuff_t* pSampleBuffer = NULL;

static Bool validConfiguration = FALSE;

static Bool done = FALSE;
static uint32_t bytesToCapture = 100;

static uint32_t* pSampleData = NULL;

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  I2C Interrupt handler, saves all I2C samples
 * @ingroup RES_IRQ
 *
 *****************************************************************************/
void I2C0_IRQHandler(void)
{
  SET_MEAS_PIN_1();

  *pSampleData++ = LPC_TIMER3->TC;
  *pSampleData++ = LPC_I2C0->STAT;
  *pSampleData++ = LPC_I2C0->DATA_BUFFER;

  /* As (soon to be) explained in LPC43xx User Manual Errata:

     Introduction:
        The I2C monitor allows the device to monitor the I2C traffic on the
        I2C bus in a non-intrusive way.

     Problem:
        In the slave-transmitter mode, the device set in the monitor mode must
        write a dummy value of 0xFF into the DAT register.  If this is not done,
        the received data from the slave device will be corrupted.
        To allow the monitor mode to have sufficient time to process the data on
        the I2C bus, the device may need to have the ability to stretch the I2C
        clock. Under this condition, the I2C monitor mode is not 100% non-intrusive.
  */
  switch (LPC_I2C0->STAT)
  {
    case 0xA8:                                   // Own SLA + R has been received, ACK returned
    case 0xB0:
    case 0xB8:                                   // data byte in DAT transmitted, ACK received
    case 0xC0:                                   // (last) data byte transmitted, NACK received
    case 0xC8:                                   // last data byte in DAT transmitted, ACK received
      LPC_I2C0->DAT = 0xFF;                      // Pretend to shift out 0xFF
      break;
  }
  LPC_I2C0->CONCLR = I2C_I2CONCLR_SIC;

  if (--bytesToCapture == 0)
  {
    I2C_MonitorModeCmd(LPC_I2C0, DISABLE);
    I2C_IntCmd(LPC_I2C0, FALSE);
    done = TRUE;
  }
  CLR_MEAS_PIN_1();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/


/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes the timer that will be used for timestamping samples
 *
 *****************************************************************************/
void monitor_i2c_Init(void)
{
  // Initialize timer 3, prescale count time of 1uS
  timerCfg.PrescaleOption = TIM_PRESCALE_USVAL;
  timerCfg.PrescaleValue  = 1;
}

/**************************************************************************//**
 *
 * @brief  Applies the configuration data (comes from the client).
 *
 * @param [in] buff           Circular buffer to store captured data in
 * @param [in] cfg            Configuration to apply
 *
 * @retval CMD_STATUS_OK      If successfully configured
 * @retval CMD_STATUS_ERR_*   When the configuration could not be applied
 *
 *****************************************************************************/
cmd_status_t monitor_i2c_Configure(circbuff_t* buff, monitor_i2c_cfg_t* cfg)
{
  cmd_status_t result = CMD_STATUS_OK;

  pSampleBuffer = buff;

  validConfiguration = FALSE;

  do
  {
    if (cfg->clockrate <= 400000)
    {
      // Enable use of I2C buffer for max 400KHz
      LPC_GPIO_PORT->DIR[5] |= (1UL << 8);
      LPC_GPIO_PORT->SET[5] |= (1UL << 8);
    }
//     else if (cfg->clockrate <= 1000000)
//     {
//       // Enable use of I2C buffer for max 1MHz
//       if (pca9555_rawValues(CTRL_I2C_EN2, CTRL_I2C_EN1 | CTRL_I2C_EN2) != SUCCESS)
//       {
//         result = CMD_STATUS_ERR_MON_I2C_PCA95555_FAILED;
//         break;
//       }
//     }
    else
    {
      result = CMD_STATUS_ERR_MON_I2C_INVALID_RATE;
      break;
    }

    I2C_DeInit(LPC_I2C0);
    I2C_Init(LPC_I2C0, cfg->clockrate);
    I2C_Cmd(LPC_I2C0, ENABLE);
    LPC_I2C0->ADR0 = 0xc0;
    LPC_I2C0->ADR1 = 0xc1;

    /* Match all addresses and control the SCL output */
    I2C_MonitorModeConfig(LPC_I2C0, I2C_MONITOR_CFG_SCL_OUTPUT | I2C_MONITOR_CFG_MATCHALL, ENABLE);

    validConfiguration = TRUE;

  } while (FALSE);

  return result;
}

/**************************************************************************//**
 *
 * @brief  Starts the I2C monitor
 *
 * @retval CMD_STATUS_OK      If successfully started
 * @retval CMD_STATUS_ERR_*   If the I2C monitor could not be started
 *
 *****************************************************************************/
cmd_status_t monitor_i2c_Start(void)
{
  if (!validConfiguration)
  {
    // no point in arming if the configuration is invalid
    return CMD_STATUS_ERR_MON_I2C_NOT_CONFIGURED;
  }

  CLR_MEAS_PIN_1();

  TIM_Init(LPC_TIMER3, TIM_TIMER_MODE, &timerCfg);
  TIM_Cmd(LPC_TIMER3, ENABLE);

  circbuff_Reset(pSampleBuffer);
  bytesToCapture = pSampleBuffer->size / SAMPLE_SIZE;
  pSampleData = (uint32_t*)pSampleBuffer->data;
  done = FALSE;
  I2C_IntCmd(LPC_I2C0, TRUE);
  I2C_MonitorModeCmd(LPC_I2C0, ENABLE);

  while (!done)
  {
    TIM_Waitms(10);
  }

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Stops the I2C monitor
 *
 * @retval CMD_STATUS_OK      If successfully stopped
 * @retval CMD_STATUS_ERR_*   If the I2C monitor could not be stopped
 *
 *****************************************************************************/
cmd_status_t monitor_i2c_Stop(void)
{
  I2C_MonitorModeCmd(LPC_I2C0, DISABLE);

  TIM_Cmd(LPC_TIMER3, DISABLE);
  TIM_DeInit(LPC_TIMER3);

  // Disable use of I2C buffers
  LPC_GPIO_PORT->DIR[5] |= (1UL << 8);
  LPC_GPIO_PORT->CLR[5] |= (1UL << 8);

  return CMD_STATUS_OK;
}

#if (TEST_I2C_MONITOR == OPT_ENABLED)
circbuff_t testBuffer;
monitor_i2c_cfg_t testCfg;
void monitor_i2c_Test(void)
{
  cmd_status_t result;
  int i;
  int j;
  sample_t* pData;

  monitor_i2c_Init();

  testCfg.clockrate = 100000;
  testCfg.bytesToCapture = 1000;
  circbuff_Init(&testBuffer, 0x20000000, testCfg.bytesToCapture * SAMPLE_SIZE);

  result = monitor_i2c_Configure(&testBuffer, &testCfg);
  if (result != CMD_STATUS_OK)
  {
    log_i("Failed to configure I2C monitor. Error code %d. Entering infinite loop...\r\n", result);
    while(1);
  }

  log_i("Starting I2C monitor...\r\n");
  j = 1;
  while (j--)
  {
    result = monitor_i2c_Start();
    if (result != CMD_STATUS_OK)
    {
      log_i("Failed to configure I2C monitor. Error code %d. Entering infinite loop...\r\n", result);
      break;
    }

    log_i("Got I2C data...\r\n");
    log_i("Timestamp  Data  Status  Extra\r\n");
    log_i("---------  ----  ------  -----\r\n");
    pData = (sample_t*)testBuffer.data;
    for (i = 0; i < testCfg.bytesToCapture; i++)
    {
      switch (pData[i].data)
      {
        case 0xc0:
        case 0xac:
          log_i("%9u  0x%02x   0x%02x   W:%02xh\r\n", pData[i].timestamp, pData[i].data, pData[i].status, pData[i].data>>1);
          break;

        case 0xc1:
        case 0xad:
          log_i("%9u  0x%02x   0x%02x   R:%02xh\r\n", pData[i].timestamp, pData[i].data, pData[i].status, pData[i].data>>1);
          break;

        default:
          log_i("%9u  0x%02x   0x%02x\r\n", pData[i].timestamp, pData[i].data, pData[i].status);
          break;
      }
      TIM_Waitms(2);// to prevent lost printouts
    }
  }
  if (j==0)
  {
    log_i("Done sampling, entering infinite loop...\r\n");
  }
  while(1);
}
#endif //if(TEST_I2C_MONITOR == OPT_ENABLED)

