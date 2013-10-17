/*!
 * @file
 * @brief   Contains some experimental functions, not yet enabled
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

#include "lpc_types.h"
#include "lpc43xx_scu.h"
#include "lpc43xx_timer.h"

#include "led.h"
#include "log.h"
#include "meas.h"
#include "spi_eeprom.h"

#include "sct_fsm.h"
#include "labtool_config.h"
#include "monitor_i2c.h"
#include "experiments.h"
#include "calibrate.h"

/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

#if (TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)
  #define NUM_CAPTURE 16
  #define ERR_MUL 10
  #define ROUND(__val) ((((__val)%(ERR_MUL)) >= 5)?(1+((__val)/ERR_MUL)):(0+((__val)/ERR_MUL)))
  #define DIFF(__a, __b) (((__a) > (__b)) ? ((__a) - (__b)) : ((__b) - (__a)))
  #define SETTINGS_SGPIO  (PDN_DISABLE | PUP_DISABLE | INBUF_ENABLE )
#endif

/******************************************************************************
 * Local variables
 *****************************************************************************/

#if (TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)
  uint32_t sample_buffer[NUM_CAPTURE];
  uint32_t freq_rising[NUM_CAPTURE/2];
  uint32_t freq_falling[NUM_CAPTURE/2];
  volatile int wait = 1;
#endif

/******************************************************************************
 * Global variables
 *****************************************************************************/

extern volatile uint32_t mstick;

/******************************************************************************
 * Global Functions
 *****************************************************************************/

#if (TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)
void SCT_IRQHandler (void)
{
  /* Acknowledge the interrupt souce */
  LPC_SCT->EVFLAG = (1 << SCT_IRQ_EVENT_samplingComplete);

  /* Let 'main' know that sampling has finished */
  wait = 0;
}
#endif

/******************************************************************************
 * Local Functions
 *****************************************************************************/

#if (TEST_SPI_EEPROM == OPT_ENABLED)
#define SPI_EEPROM_TEST_SIZE  (48) //(16)
static void experiments_testSpiEeprom(void)
{
  uint8_t buff_w[SPI_EEPROM_TEST_SIZE], buff_r[SPI_EEPROM_TEST_SIZE];
  int i;
  Bool useOldValues = TRUE;

  log_i("SPI EEPROM TEST...\r\n");
  spi_eeprom_init();
  log_d("Status: %#x", spi_eeprom_read_status());

  // See what is in the memory from previous runs
  for (i = 0; i < SPI_EEPROM_TEST_SIZE; i++)
  {
    buff_r[i] = 0;
  }
  spi_eeprom_read(buff_r, 0, SPI_EEPROM_TEST_SIZE);
  log_d("Status: %#x", spi_eeprom_read_status());
  log_i("Old E2PROM Content:\r\n");
  display_buffer_hex(buff_r, SPI_EEPROM_TEST_SIZE);
  for (i = 1; i < SPI_EEPROM_TEST_SIZE; i++)
  {
    if (buff_r[i] == buff_r[i-1]) {
      // the old values were identical (could be because the
      // memory has never been written)
      useOldValues = FALSE;
    }
  }


  // Write new content
  for (i = 0; i < SPI_EEPROM_TEST_SIZE; i++)
  {
    if (useOldValues)
    {
      buff_w[i] = buff_r[i] + 1;
    }
    else
    {
      buff_w[i] = i;
    }
  }
  buff_w[0] = 0xea;
  buff_w[SPI_EEPROM_TEST_SIZE - 1] = 0xae;
  spi_eeprom_write(buff_w, 0, SPI_EEPROM_TEST_SIZE);
  log_d("Status: %#x", spi_eeprom_read_status());
  log_i("Written To E2PROM:\r\n");
  display_buffer_hex(buff_w, SPI_EEPROM_TEST_SIZE);

  // Read content back
  for (i = 0; i < SPI_EEPROM_TEST_SIZE; i++)
  {
    buff_r[i] = 0;
  }
  TIM_Waitms(10);
  spi_eeprom_read(buff_r, 0, SPI_EEPROM_TEST_SIZE);
  log_d("Status: %#x", spi_eeprom_read_status());
  log_i("Read Back From E2PROM:\r\n");
  display_buffer_hex(buff_r, SPI_EEPROM_TEST_SIZE);

  // verify
  for (i = 0; i < SPI_EEPROM_TEST_SIZE; i++)
  {
    if (buff_w[i] != buff_r[i])
    {
      log_i("FAILED!!\r\n");
      return;
    }
  }
  log_i("PASSED!!\r\n");
}
#endif //if(TEST_SPI_EEPROM == OPT_ENABLED)

#if (TEST_CALIB_DATA_STORE_LOAD == OPT_ENABLED)
static uint8_t calib_buff[256];
static calib_result_t* calibRead = (calib_result_t*)calib_buff;
static void experiments_testCalibDataStoreLoad(void)
{
  int i;
  cmd_status_t res;

  log_i("CALIBRATION DATA TEST...\r\n");
  spi_eeprom_init();

  // See what is in the memory from previous runs
  for (i = 0; i < 256; i++)
  {
    calib_buff[i] = 0;
  }
  spi_eeprom_read(calib_buff, 0, 256);
  log_i("Old E2PROM Content:\r\n");
  display_buffer_hex(calib_buff, 256);

  // Attempt to load calibration data
  res = calibrate_LoadCalibrationData(calibRead);
  if (res != CMD_STATUS_OK)
  {
    log_i("Failed to load calibration data. Error code %u\r\n", res);
  }

  // Modify the default data and store it
  calibRead->dacValOut[0] = 0xcafebabe;
  calibRead->dacValOut[1] = 0xAAAAAAAA;
  calibRead->dacValOut[2] = 0x55555555;
  calibRead->inHigh[1][7] = 0x12345678;
  calibRead->inHigh[0][7] = 0xa1a1a1a1;

  res = calibrate_StoreCalibrationData(calibRead);
  if (res != CMD_STATUS_OK)
  {
    log_i("Failed to store calibration data. Error code %u\r\n", res);
  }

  // See what is in the memory after this run
  for (i = 256; i < 512; i++)
  {
    calib_buff[i] = 0;
  }
  spi_eeprom_read(calib_buff, 0, 256);
  log_i("New E2PROM Content:\r\n");
  display_buffer_hex(calib_buff, 256);

  if (res == CMD_STATUS_OK)
  {
    log_i("PASSED!!\r\n");
  }
  else
  {
    log_i("FAILED!!\r\n");
  }
}
#endif


#if (TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)
static uint32_t experiments_getMedian(uint32_t* buff, int size)
{
  static uint32_t copy[NUM_CAPTURE/2];
  int pos = 0;
  int i;
  int j;
  int median = 0;

  for (i = 0; i < size; i++)
  {
    copy[i] = buff[i];
  }

  for (j = 0; j < (size/3); j++)
  {
    // find and remove one MIN value
    for (i = 1, pos = 0; i < size; i++)
    {
      if (copy[i] != 0)
      {
        if ((copy[i] < copy[pos]) || (copy[pos] == 0))
        {
          pos = i;
        }
      }
    }
    copy[pos] = 0;

    // find and remove one MAX value
    for (i = 1, pos = 0; i < size; i++)
    {
      if (copy[i] != 0)
      {
        if ((copy[i] > copy[pos]) || (copy[pos] == 0))
        {
          pos = i;
        }
      }
    }
    copy[pos] = 0;
  }

  for (i = 0; i < size; i++)
  {
    median += copy[i];
  }

  if (median > 200000000)
  {
    // Cannot to any rounding here as the value will overflow
    median = median/(size-2*(size/3));
  }
  else
  {
    median = median*ERR_MUL;
    median = ROUND(median/(size-2*(size/3)));
  }
  return median;
}

static void experiments_testFrequencyCounter(void)
{
  int i;
  Bool status = FALSE;
  uint32_t median;
  uint32_t time;

  // Using SCT input 0 (SGPIO3)
  scu_pinmux(0x1,  0, SETTINGS_SGPIO, FUNC1); //CTIN_3

  // Using SCT output 7 (overriding BOOT LED pin)
  scu_pinmux(0x1, 1, MD_PDN, FUNC1); // P1_1,FUNC1 => CTOUT_7

  // Global configuration of the SCT
  LPC_CCU1->CLK_M4_SCT_CFG = 0x00000001;      // Enable SCT branch clock in CCU1, RUN=1, AUTO=0, WAKE=0
  LPC_SCT->CONFIG = 0x00000001;               /* Configure as unified 32-bit timer, clocked internally */
  LPC_SCT->CTRL_L = 0x000C + ((1-1) << 5);    /* L counter: */
  /* Set prescaler = 1. Clear counter. Keep halted */

  /* Now use the FSM code to configure the state machine */
  sct_fsm_init();

  NVIC_EnableIRQ (SCT_IRQn);

  // Run in this loop forever and make measurements every x seconds
  while( 1 )
  {
    // Start the SCT
    LPC_SCT->CTRL_U &= ~(1 << 2);  /* Run L only */

    // run cpu into a loop waiting for completion of the sampling
    time = mstick;
    while (wait);
    time = mstick - time;


    // Now put the results from the SCT into the buffer
    sample_buffer[0] = SCT_CAPTURE_sample1;
    sample_buffer[1] = SCT_CAPTURE_sample2;
    sample_buffer[2] = SCT_CAPTURE_sample3;
    sample_buffer[3] = SCT_CAPTURE_sample4;
    sample_buffer[4] = SCT_CAPTURE_sample5;
    sample_buffer[5] = SCT_CAPTURE_sample6;
    sample_buffer[6] = SCT_CAPTURE_sample7;
    sample_buffer[7] = SCT_CAPTURE_sample8;
    sample_buffer[8] = SCT_CAPTURE_sample9;
    sample_buffer[9] = SCT_CAPTURE_sample10;
    sample_buffer[10] = SCT_CAPTURE_sample11;
    sample_buffer[11] = SCT_CAPTURE_sample12;
    sample_buffer[12] = SCT_CAPTURE_sample13;
    sample_buffer[13] = SCT_CAPTURE_sample14;
    sample_buffer[14] = SCT_CAPTURE_sample15;
    sample_buffer[15] = SCT_CAPTURE_sample16;

    for (i = 0; i < (NUM_CAPTURE/2 - 1); i++)
    {
      // This calculation includes a certain error, because of the division with integer values
      freq_rising[i] = (SystemCoreClock*ERR_MUL) / DIFF(sample_buffer[(i*2)+2], sample_buffer[i*2]);
    }

    for (i = 0; i < (NUM_CAPTURE/2 - 1); i++)
    {
      // This calculation includes a certain error, because of the division with integer values
      freq_falling[i] = (SystemCoreClock*ERR_MUL) / DIFF(sample_buffer[(i*2)+3], sample_buffer[(i*2)+1]);
    }

//     for (i = 0; i < (NUM_CAPTURE/2 - 1); i++)
//     {
//       // Make a simple printout of the detected values
//       log_i("fr[%d] = %d   ff[%d] = %d\r\n", i, ROUND(freq_rising[i]), i, ROUND(freq_falling[i]));
//     }


//     // Print the sample buffer content
//     for (i = 0; i < 16; i++)
//     {
//       // Make a simple printout of the detected values
//       log_i("sb[%d] = %d\r\n", i, sample_buffer[i]);
//     }

    // Make a simple printout of the detected values
    median = experiments_getMedian(freq_rising, (NUM_CAPTURE/2 - 1));
    log_i("Frequency = %d Hz  (took %d ms to detect)\r\n", ROUND(median), time);
//     log_i("Frequency = %d Hz  (median %d Hz)\r\n", ROUND(freq_rising[0]), ROUND(median));

    wait = 1;

    if (status)
    {
      LED_SPARE1_ON();
      status = FALSE;
    }
    else
    {
      LED_SPARE1_OFF();
      status = TRUE;
    }

    // Program the timer to wait 2 seconds
    TIM_Waitms(2000);
  }
}
#endif //if(TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)

/******************************************************************************
 * Public Functions
 *****************************************************************************/


/**************************************************************************//**
 *
 * @brief   Entry point to experimental functionality.
 *
 *****************************************************************************/
void experiments_Run(void)
{
#if (ENABLE_MEASSURING == OPT_ENABLED)
  CLR_MEAS_PIN_1();
  CLR_MEAS_PIN_2();
  CLR_MEAS_PIN_3();
  TIM_Waitms(10);
  SET_MEAS_PIN_1();
  TIM_Waitms(10);
  SET_MEAS_PIN_2();
  TIM_Waitms(10);
  SET_MEAS_PIN_3();
  TIM_Waitms(10);
  CLR_MEAS_PIN_3();
  TIM_Waitms(10);
  CLR_MEAS_PIN_2();
  TIM_Waitms(10);
  CLR_MEAS_PIN_1();
  TIM_Waitms(100);
  SET_MEAS_PIN_1();SET_MEAS_PIN_2();SET_MEAS_PIN_3();
  TIM_Waitms(100);
  CLR_MEAS_PIN_1();CLR_MEAS_PIN_2();CLR_MEAS_PIN_3();
  TIM_Waitms(100);
  SET_MEAS_PIN_1();SET_MEAS_PIN_2();SET_MEAS_PIN_3();
  TIM_Waitms(100);
  CLR_MEAS_PIN_1();CLR_MEAS_PIN_2();CLR_MEAS_PIN_3();
  TIM_Waitms(100);
  SET_MEAS_PIN_1();SET_MEAS_PIN_2();SET_MEAS_PIN_3();
  TIM_Waitms(100);
  CLR_MEAS_PIN_1();CLR_MEAS_PIN_2();CLR_MEAS_PIN_3();
#endif

#if (TEST_SCT_FREQUENCY_COUNTER == OPT_ENABLED)
  experiments_testFrequencyCounter();
#endif

#if (TEST_I2C_MONITOR == OPT_ENABLED)
  monitor_i2c_Test();
#endif

#if (TEST_SPI_EEPROM == OPT_ENABLED)
  experiments_testSpiEeprom();
#endif

#if (TEST_CALIB_DATA_STORE_LOAD == OPT_ENABLED)
  experiments_testCalibDataStoreLoad();
#endif
}
