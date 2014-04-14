/*!
 * @file
 * @brief   Program entry point
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

#ifdef __CODE_RED
#include <cr_section_macros.h>
#endif

#include "lpc_types.h"
#include "lpc43xx_adc.h"
#include "lpc43xx_cgu_improved.h"
#include "lpc43xx_i2c.h"
#include "lpc43xx_rgu.h"
#include "lpc43xx_scu.h"
#include "lpc43xx_timer.h"
#include "lpc43xx_uart.h"

#include "debug_frmwrk.h"

#include "spi_control.h"
#include "led.h"
#include "log.h"
#include "usb_handler.h"

#include "capture.h"
#include "generator.h"
#include "labtool_config.h"
#include "statemachine.h"
#include "experiments.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*!
 * I2C bus to use.
 */
#define I2C_PORT (LPC_I2C0)

/*! \name SCU settings for the pins
 * \{
 */
#define SETTINGS_GPIO_IN    (PUP_DISABLE | PDN_DISABLE | SLEWRATE_SLOW | INBUF_ENABLE  | FILTER_ENABLE)
#define SETTINGS_GPIO_OUT   (PUP_DISABLE | PDN_DISABLE | SLEWRATE_SLOW |                 FILTER_ENABLE)
#define SETTINGS_SGPIO      (PDN_DISABLE | PUP_DISABLE |                 INBUF_ENABLE                 )
#define SETTINGS_SPIFI      (PUP_DISABLE | PDN_DISABLE | SLEWRATE_SLOW | INBUF_ENABLE  | FILTER_ENABLE)
#define SETTINGS_SSP        (PUP_DISABLE | PDN_DISABLE | SLEWRATE_SLOW | INBUF_ENABLE  | FILTER_ENABLE)
/* \} */

/******************************************************************************
 * Local variables
 *****************************************************************************/

static volatile uint32_t mstick = 0;


/******************************************************************************
 * Global variables
 *****************************************************************************/


/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief   Systick callback function.
 *
 *****************************************************************************/
void SysTick_Handler(void)
{
  mstick++;
}

#ifdef DEBUG
/**************************************************************************//**
 *
 * @brief   Called from drivers with parameter errors
 *
 *****************************************************************************/
void check_failed(uint8_t *file, uint32_t line)
{
#if (ENABLE_LOGGING == OPT_ENABLED)
  log_i("check_failed at %s:%d\r\n", file, line);
#else
  (void)file;
  (void)line;
#endif
}
#endif

/******************************************************************************
 * Local Functions
 *****************************************************************************/

#if (ENABLE_LOGGING == OPT_ENABLED)
static void PrintGreeting(void)
{
  log_i("\r\n***************************************************\r\n");
  log_i("*                                                 *\r\n");
  log_i("* LabTool for LPC4357 OEM Boards...                *\r\n");
  log_i("* (C) Embedded Artists 2001-2012                  *\r\n");
  log_i("*                                                 *\r\n");
  log_i("*                           " __DATE__ ", " __TIME__ " *\r\n");
  log_i("***************************************************\r\n");

#if (ENABLE_MEASSURING == OPT_ENABLED)
  log_i("\r\n!!!!\r\n");
  log_i("!!!! Measuring is enabled, Analog IN/OUT and e2prom MAY FAIL\r\n");
  log_i("!!!!\r\n");
#endif
}
#endif

/**************************************************************************//**
 *
 * @brief   Configuration of all pins.
 *
 *****************************************************************************/
static void pinConfig(void)
{

  //-------------------------------------------------------------------------
  // SGPIO
  //-------------------------------------------------------------------------

  scu_pinmux(0x0,  0, SETTINGS_SGPIO, FUNC3); //SGPIO_0,  DIO0, J4-16
  scu_pinmux(0x2,  4, SETTINGS_SGPIO, FUNC0); //SGPIO_13, DIO1, J4-15
  scu_pinmux(0x2,  1, SETTINGS_SGPIO, FUNC0); //SGPIO_5,  DIO2, J4-14
  scu_pinmux(0x1,  6, SETTINGS_SGPIO, FUNC6); //SGPIO_14, DIO3, J4-13
  scu_pinmux(0x1, 15, SETTINGS_SGPIO, FUNC2); //SGPIO_2,  DIO4, J4-12
  scu_pinmux(0x1,  0, SETTINGS_SGPIO, FUNC6); //SGPIO_7,  DIO5, J4-11
  scu_pinmux(0x1, 14, SETTINGS_SGPIO, FUNC6); //SGPIO_10, DIO6, J4-10
  scu_pinmux(0x1, 13, SETTINGS_SGPIO, FUNC6); //SGPIO_9,  DIO7, J4-9
  scu_pinmux(0x1, 17, SETTINGS_SGPIO, FUNC6); //SGPIO_11, DIO8, J4-8
  scu_pinmux(0x1, 18, SETTINGS_SGPIO, FUNC6); //SGPIO_12, DIO9, J4-7
  scu_pinmux(0x1, 12, SETTINGS_SGPIO, FUNC6); //SGPIO_8,  DIO_CLK, J4-6

  /* Set doubled SGPIOs as input GPIOs to avoid conflicts */
  scu_pinmux(0x2,  0, SETTINGS_GPIO_IN, FUNC4); //GPIO5[0], ----, J4-16    (SGPIO_4)
  scu_pinmux(0x0,  1, SETTINGS_GPIO_IN, FUNC0); //GPIO0[1], ----, J4-14    (SGPIO_1)
  scu_pinmux(0x2,  2, SETTINGS_GPIO_IN, FUNC4); //GPIO5[2], ----, J4-12    (SGPIO_6)
  scu_pinmux(0x1, 16, SETTINGS_GPIO_IN, FUNC0); //GPIO0[3], ----, J4-11    (SGPIO_3)
  LPC_GPIO_PORT->DIR[5] &= ~(1UL << 0);
  LPC_GPIO_PORT->DIR[0] &= ~(1UL << 1);
  LPC_GPIO_PORT->DIR[5] &= ~(1UL << 2);
  LPC_GPIO_PORT->DIR[0] &= ~(1UL << 3);


  //-------------------------------------------------------------------------
  // GPIO
  //-------------------------------------------------------------------------

  /* Inputs */
  scu_pinmux(0x1,  2, SETTINGS_GPIO_IN, FUNC0); //GPIO0[9],  BOOT1
  scu_pinmux(0x1,  7, SETTINGS_GPIO_IN, FUNC0); //GPIO1[0],  GPIO1_0
  scu_pinmux(0x1,  8, SETTINGS_GPIO_IN, FUNC0); //GPIO1[1],  GPIO1_1
  scu_pinmux(0x1,  9, SETTINGS_GPIO_IN, FUNC0); //GPIO1[2],  GPIO1_2
  scu_pinmux(0x1, 10, SETTINGS_GPIO_IN, FUNC0); //GPIO1[3],  GPIO1_3  available on J9-10
  scu_pinmux(0x1, 11, SETTINGS_GPIO_IN, FUNC0); //GPIO1[4],  GPIO1_4

  scu_pinmux(0x6,  1, SETTINGS_GPIO_IN, FUNC0); //GPIO3[0],  GPIO3_0
  scu_pinmux(0x6,  2, SETTINGS_GPIO_IN, FUNC0); //GPIO3[1],  GPIO3_1  available on J9-20
  scu_pinmux(0x6,  4, SETTINGS_GPIO_IN, FUNC0); //GPIO3[3],  GPIO3_3
  scu_pinmux(0x6,  5, SETTINGS_GPIO_IN, FUNC0); //GPIO3[4],  GPIO3_4  available on J9-24

  scu_pinmux(0x2,  5, SETTINGS_GPIO_IN, FUNC4); //GPIO5[5],  GPIO5_5_JTAG_RESET
  scu_pinmux(0x2,  7, SETTINGS_GPIO_IN, FUNC0); //GPIO0[7],  ISP_N
  scu_pinmux(0x2,  8, SETTINGS_GPIO_IN, FUNC4); //GPIO5[7],  BOOT2
  scu_pinmux(0x2,  9, SETTINGS_GPIO_IN, FUNC0); //GPIO1[10], BOOT3
  scu_pinmux(0x2, 12, SETTINGS_GPIO_IN, FUNC0); //GPIO1[12], GPIO1_12
  scu_pinmux(0x2, 13, SETTINGS_GPIO_IN, FUNC0); //GPIO1[13], GPIO1_13 available on J9-16


  /* Outputs */
  scu_pinmux(0x2,  3, SETTINGS_GPIO_OUT, FUNC6); //GPIO5[3],  ISP_CTRL
  LPC_GPIO_PORT->DIR[5] |= (1UL << 3);
  LPC_GPIO_PORT->SET[5] |= (1UL << 3);

  scu_pinmux(0x2,  6, SETTINGS_GPIO_OUT, FUNC4); //GPIO5[6],  GPIO5_6_JTAG_RESET_TXEN
  LPC_GPIO_PORT->DIR[5] |= (1UL << 6);
  LPC_GPIO_PORT->CLR[5] |= (1UL << 6);

  //-------------------------------------------------------------------------
  // Controls for DIO direction  (1-0  for Digital in, 0-1 for Digital out)
  //-------------------------------------------------------------------------

  scu_pinmux(0x1,  5, SETTINGS_GPIO_OUT, FUNC0); //GPIO1[8],  SGPIO15_TMS_SWDIO_TXEN, Controls buffer U7
  LPC_GPIO_PORT->DIR[1] |= (1UL << 8);
  LPC_GPIO_PORT->CLR[1] |= (1UL << 8);

  scu_pinmux(0x2, 10, SETTINGS_GPIO_OUT, FUNC0); //GPIO0[14], UART2_TXD, Controls buffer U4
  LPC_GPIO_PORT->DIR[0] |= (1UL << 14);
  LPC_GPIO_PORT->SET[0] |= (1UL << 14);

  scu_pinmux(0x2, 11, SETTINGS_GPIO_OUT, FUNC0); //GPIO1[11], UART2_RXD, Controls buffer U3
  LPC_GPIO_PORT->DIR[1] |= (1UL << 11);
  LPC_GPIO_PORT->CLR[1] |= (1UL << 11);

  //-------------------------------------------------------------------------
  // SSP
  //-------------------------------------------------------------------------

  scu_pinmux(0x1,  3, SETTINGS_SSP, FUNC5); //SSP1_MISO
  scu_pinmux(0x1,  4, SETTINGS_SSP, FUNC5); //SSP1_MOSI
  scu_pinmux(0xF,  4, SETTINGS_SSP, FUNC0); //SSP1_SCK

  // CS for DAC is controlled via P1.20 and not SSP1_SSEL.
  // Pin P1.20 should be configured as GPIO and set high.
  scu_pinmux(0x1, 20, SETTINGS_GPIO_OUT, FUNC0); //GPIO0[15], available on J7-6
  LPC_GPIO_PORT->DIR[0] |= (1UL << 15);
  LPC_GPIO_PORT->SET[0] |= (1UL << 15);

  // CS for E2PROM is controlled via P3.2 and not SSP1_SSEL.
  // Pin P3.2 should be configured as GPIO and set high.
  scu_pinmux(0x3,  2, SETTINGS_GPIO_OUT, FUNC4); //GPIO5[9], available on J7-12
  LPC_GPIO_PORT->DIR[5] |= (1UL << 9);
  LPC_GPIO_PORT->SET[5] |= (1UL << 9);

  // CS for GPO is controlled via P6.11 and not SSP1_SSEL.
  // Pin P6.11 should be configured as GPIO and set high.
  scu_pinmux(0x6, 11, SETTINGS_GPIO_OUT, FUNC0); //GPIO3[7], available on J7-14
  LPC_GPIO_PORT->DIR[3] |= (1UL << 7);
  LPC_GPIO_PORT->SET[3] |= (1UL << 7);

  //-------------------------------------------------------------------------
  // SPIFI
  //-------------------------------------------------------------------------

//   scu_pinmux(0x3,  3, SETTINGS_SPIFI, FUNC3); //SPIFI_SCK
//   scu_pinmux(0x3,  4, SETTINGS_SPIFI, FUNC3); //SPIFI_SIO3
//   scu_pinmux(0x3,  5, SETTINGS_SPIFI, FUNC3); //SPIFI_SIO2
//   scu_pinmux(0x3,  6, SETTINGS_SPIFI, FUNC3); //SPIFI_MISO
//   scu_pinmux(0x3,  7, SETTINGS_SPIFI, FUNC3); //SPIFI_MOSI
//   scu_pinmux(0x3,  8, SETTINGS_SPIFI, FUNC3); //SPIFI_CS

  //-------------------------------------------------------------------------
  // LED
  //-------------------------------------------------------------------------

  /* Temporary use of the BOOT LED on the LPC Link-II as trigger LED to
     be able to set it from within an interrupt. The PCA9555 LEDs require
     I2C communication which should not be used from within an interrupt. */
  scu_pinmux(0x1, 1, MD_PDN, FUNC0); // P1_1,FUNC0 => GPIO0[8]
  LPC_GPIO_PORT->DIR[0] |= (1UL << 8);

  //-------------------------------------------------------------------------
  // Enabling/Disabling of I2C Monitoring
  //-------------------------------------------------------------------------

  /* Control signal is GPIO output, default low (=off) */
  scu_pinmux(0x3,  1, SETTINGS_GPIO_OUT, FUNC4); //GPIO5[8],  CTRL_I2C_EN available on J7-11
  LPC_GPIO_PORT->DIR[5] |= (1UL << 8);
  LPC_GPIO_PORT->CLR[5] |= (1UL << 8);

  //-------------------------------------------------------------------------
  // CALIBRATION SIGNAL
  //-------------------------------------------------------------------------

  scu_pinmux(0x6,  9, SETTINGS_GPIO_OUT, FUNC5); //T2_MAT,  CAL_SIG available on J7-13

  //-------------------------------------------------------------------------
  // CLOCK OUTPUT
  //-------------------------------------------------------------------------

//  scu_pinmux(0x1, 19, ?, FUNC4); //CLKOUT, CGU_CLKOUT available on TP_P1_19
//  scu_pinmux(0x6,  0, ?, ?); // available on TP_P6_0
//  scu_pinmux(0x3,  0, ?, ?); // available on TP_P3_0

}

/**************************************************************************//**
 *
 * @brief   Configuration of IRQ relative priorities.
 * @ingroup RES_IRQ
 *
 *****************************************************************************/
static void priorityConfig()
{
  /*
     The following modules use interrupts:

     SGPIO Capture:    SGPIO_IINT_IRQn, GINT0_IRQn
     VADC Capture:     VADC_IRQn + DMA_IRQn

     SGPIO Generator:  SGPIO_IINT_IRQn
     DAC Generator:    TIMER1_IRQn + TIMER3_IRQn

     Freq Counter:     SCT_IRQn

     I2C:              I2C0_IRQn + TIMER3_IRQn
     USB:              USB0_IRQn + USB1_IRQn

     Interprocess:     M0CORE_IRQn + M0_M4CORE_IRQn ??
     Systick:          SysTick_IRQn
     Timers:           ??
  */

  // Highest - Interprocess (to stop ongoing tasks, rarely occurs)
  //NVIC_SetPriority(M0CORE_IRQn,    0x00);
  //NVIC_SetPriority(M0_M4CORE_IRQn, 0x00);

  // High - Copying of samples
  NVIC_SetPriority(VADC_IRQn,        0x01);
  NVIC_SetPriority(SGPIO_IINT_IRQn,  0x01);
  NVIC_SetPriority(TIMER1_IRQn,      0x01);
  NVIC_SetPriority(TIMER3_IRQn,      0x01);

  // Standard - Trigger handling
  NVIC_SetPriority(DMA_IRQn,         0x02);
  NVIC_SetPriority(GINT0_IRQn,       0x02);
  NVIC_SetPriority(SCT_IRQn,         0x02);

  // Low - Communication
  NVIC_SetPriority(USB0_IRQn,        0x03);
  NVIC_SetPriority(USB1_IRQn,        0x03);
  NVIC_SetPriority(I2C0_IRQn,        0x03);
}

/**************************************************************************//**
 *
 * @brief   Sets up a 2KHz calibration signal.
 *
 *****************************************************************************/
static void setupCalibrationSignal()
{
#if 1
  TIM_TIMERCFG_Type timerCfg;
  TIM_MATCHCFG_Type matchCfg;

  // Initialize timer 2, prescale count time of 100uS
  timerCfg.PrescaleOption = TIM_PRESCALE_USVAL;
  timerCfg.PrescaleValue  = 100;

  // use channel 2, MR2
  matchCfg.MatchChannel = 2;
  // Disable interrupt when MR2 matches the value in TC register
  matchCfg.IntOnMatch   = FALSE;
  //Enable reset on MR2: TIMER will reset if MR2 matches it
  matchCfg.ResetOnMatch = TRUE;
  //Stop on MR2 if MR2 matches it
  matchCfg.StopOnMatch  = FALSE;
  //Toggle MR2.2 pin if MR2 matches it
  matchCfg.ExtMatchOutputType = TIM_EXTMATCH_TOGGLE;
  // Set Match value, count value of 5 (5 * 100uS = 0.5mS --> 2KHz)
  matchCfg.MatchValue   = 4;

  TIM_Init(LPC_TIMER2, TIM_TIMER_MODE, &timerCfg);
  TIM_ConfigMatch(LPC_TIMER2, &matchCfg);
  TIM_Cmd(LPC_TIMER2, ENABLE);
#else
  scu_pinmux(0x6,  9, SETTINGS_GPIO_OUT, FUNC0); //GPIO3[5],  CAL_SIG available on J7-13
  LPC_GPIO_PORT->DIR[3] |= (1UL << 5);
  //LPC_GPIO_PORT->SET[3] |= (1UL << 5); //SET => 0V out
  LPC_GPIO_PORT->CLR[3] |= (1UL << 5); //CLR => 1.253V out
#endif
}

/******************************************************************************
 * Main method
 *****************************************************************************/


/**************************************************************************//**
 *
 * @brief   Entry point.
 *
 *****************************************************************************/
int main (void)
{
  TIM_TIMERCFG_Type timerCfg;

  SystemInit();
  CGU_Improved_Init();

  RGU_SoftReset(RGU_SIG_ADC0);
  RGU_SoftReset(RGU_SIG_DAC);
  RGU_SoftReset(RGU_SIG_LCD);
  RGU_SoftReset(RGU_SIG_DMA);
  RGU_SoftReset(RGU_SIG_GPIO);
  RGU_SoftReset(RGU_SIG_TIMER0);
  RGU_SoftReset(RGU_SIG_I2C0);
  RGU_SoftReset(RGU_SIG_USB0);
  RGU_SoftReset(RGU_SIG_SGPIO);
  RGU_SoftReset(RGU_SIG_VADC);
  //RGU_SoftReset(RGU_SIG_SPIFI);  //important to not do when booting from SPIFI

  /* Initialize timer */
  TIM_ConfigStructInit(TIM_TIMER_MODE, &timerCfg);
  TIM_Init(LPC_TIMER0, TIM_TIMER_MODE, &timerCfg);

  pinConfig();
  priorityConfig();

  /* Request systicks every ms */
  //SysTick_Config(SystemCoreClock/1000);

  setupCalibrationSignal();

#if (ENABLE_LOGGING == OPT_ENABLED)
  debug_frmwrk_init();
  PrintGreeting();
#endif  

  /* Initialize I2C */
  I2C_Init(I2C_PORT, 100000);
  I2C_Cmd(I2C_PORT, ENABLE);

  /* Initialize Control Signals via 74HC595PW */
  spi_control_init();

  experiments_Run();

  /* Make sure calibration data is loaded (or at least set to defaults) */
  calibrate_Init();

  statemachine_Init();

  usb_handler_InitUSB(capture_Disarm, capture_Configure, capture_Arm,
                      generator_Stop, generator_Configure, generator_Start);
  statemachine_RequestState(STATE_IDLE);
  usb_handler_Run();
}
