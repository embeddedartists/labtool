/*!
 * @file
 * @brief   Handles calibration of analog signals (both in and out)
 * @ingroup FUNC_CAL
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

#include "log.h"
#include "led.h"
#include "calibrate.h"
#include "usb_handler.h"
#include "statemachine.h"
#include "spi_dac.h"
#include "spi_eeprom.h"
#include "capture_vadc.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

/*! @brief Parameters for calibration of analog outputs. */
typedef struct
{
  uint32_t level; /*!< 12 bit value to send on both analog channels */
} calib_analog_out_parameters_t;

/*! @brief Parameters for calibration of analog inputs. */
typedef struct
{
  uint32_t levels[3]; /*!< DAC values in 10-bit format used for analog out calibration */
  int measA0[3];      /*!< Meassured A0 value for each of the levels. Values is in mv */
  int measA1[3];      /*!< Meassured A1 value for each of the levels. Values is in mv */
} calib_analog_in_parameters_t;

/*! @brief Used as index when collecting statistics for the calibration data. */
typedef enum
{
  STATS_NUM,    /*!< Number of samples */
  STATS_MIN,    /*!< Lowest found sample value */
  STATS_MAX,    /*!< Highest found sample value */
  STATS_SUM,    /*!< Sum of all samples */

  NUMBER_OF_STATS, /*!< Number of enum values, used for initialization */
} stats_t;

/*! @brief Current version of the calibration data format.
 * Stored in the EEPROM with the calibration data and verified against during
 * loading of data. */
#define CALIBRATION_VERSION 0x0000ea01

/*! Token to use in the calibration data for CRC and Version to indicate that
 * the data is default data and not loaded from EEPROM.
 */
#define DEFAULT_TOKEN  0x00dead00

/*!
 * The width of the CRC calculation and result.
 * Modify the typedef for a 16 or 32-bit CRC standard.
 */
typedef uint32_t crc;

/*! Number of bits in CRC calculation (32) */
#define CRC_WIDTH          (8 * sizeof(crc))

/*! CRC internal aid */
#define CRC_TOPBIT         (1UL << (CRC_WIDTH - 1))

/*! CRC initial seed value */
#define CRC_POLYNOMIAL     0x04C11DB7

/******************************************************************************
 * Global variables
 *****************************************************************************/

calib_states_t calibrationState = CALIB_STATE_STOPPED;

/******************************************************************************
 * Local variables
 *****************************************************************************/

/*! Default calibration data. It is far from perfect but at least it correctly
 * inverts the analog signal. Used if the user erases the EEPROM, the EEPROM 
 * is has invalid data (mismatch in CRC), the data is of wrong version. */
static const calib_result_t DEFAULT_CALIBRATION = {
  .checksum = DEFAULT_TOKEN,
  .version = DEFAULT_TOKEN,
  .dacValOut = {256, 512, 768},
  .userOut = {
    { 2500, 0, -2500 },
    { 2500, 0, -2500 },
  },
  .voltsInLow = { -80, -200, -400, -800, -2000, -2500, -2500, -2500 },
  .voltsInHigh = { 80,  200,  400,  800,  2000,  2500,  2500,  2500 },
  .inLow = {
    { 2700, 2900, 3050, 3250, 3050, 2700, 2400, 2200 },
    { 2500, 2900, 3000, 3150, 3050, 2700, 2400, 2200 },
  },
  .inHigh = {
    { 570, 830, 850, 830, 1000, 1400, 1700, 1900 },
    { 500, 750, 850, 840, 1000, 1400, 1700, 1900 },
  },
};

/*! @brief Holds the active calibration data.
 *
 * If a calibrations is ongoing then the contents will be incomplete
 * and will be filled as the calibration progresses. 
 *
 * At startup the calibration data is loaded from EEPROM and is stored
 * here. If the EEPROM contains no data, the CRC does not match or
 * the version is unsupported then this struct will be filled with the
 * default parameters. 
 *
 * After a completed calibration sequence the result is restored from
 * EEPROM - regardless of how the calibration went.
 */
static calib_result_t calibrationResult;

/*! Current Volt/div setting to test. During calibration of analog inputs
 * all different Volts/div settings must be tested and this points to the 
 * current one. 
 */
static int currentVdivIndex = -1;

/*! For each of the Volts/div settings measured during analog input 
 * calibration two measurements must be made. One at a high output
 * and one at a low output. The \a measuringLowLevel keeps track of 
 * which.
 */
static Bool measuringLowLevel;

/*! Statistics gathered during calibration of analog inputs. */
static uint32_t stats[2][NUMBER_OF_STATS];

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
 * @brief  Converts the wanted output in mV into a value to be used by the DAC
 *
 * The output value is calculated for the specified channel and is based on
 * the measurements that the user took during calibration of the analog outputs.
 *
 * A = (vout1 - (vout2*hex1/hex2)) / (1 - (hex1/hex2))
 * B = (vout2 - a) / hex2
 *
 * and then
 *
 * hexOut = (vWanted - a) / b
 *
 * @param [in] ch         The channel to calculate for
 * @param [in] wantedmv   The wanted output in mV
 *
 * @return The DAC's value as the upper 10 bits of a 12 bit value
 *
 *****************************************************************************/
static uint16_t calibrate_12BitCalibratedDAC(int ch, int wantedmv)
{
  double a;
  double b;
  double wanted = wantedmv;
  double tmp;
  double vOut1 = calibrationResult.userOut[ch][0]; // low level
  double vOut2 = calibrationResult.userOut[ch][2]; // high level
  double hex1 = calibrationResult.dacValOut[0]; //256; // -2.75V
  double hex2 = calibrationResult.dacValOut[2]; //768; // +2.75V
  uint32_t res;

  wanted = wanted / 1000.0; // convert from mV to V
  vOut1  = vOut1 / 1000.0; // convert from mV to V
  vOut2  = vOut2 / 1000.0; // convert from mV to V

  a = (vOut1 - (vOut2*hex1/ hex2)) / (1 - (hex1/hex2));
  b = (vOut2 - a) / hex2;

  // result is: hexVal = (vOut - A) / B
  tmp = (wanted - a) / b;
  res = tmp;
  res = (res << 2) & 0xffc;
  return res;
}


/**************************************************************************//**
 *
 * @brief  Calculates a checksum (CRC32) on the specified data
 *
 * @param [in] message    The data to base the checksum on
 * @param [in] nBytes     The number of bytes of data
 *
 * @return The checksum
 *
 *****************************************************************************/
static crc crcSlow(uint8_t const message[], int nBytes)
{
    crc  remainder = 0;


    /*
     * Perform modulo-2 division, a byte at a time.
     */
    for (int byte = 0; byte < nBytes; ++byte)
    {
        /*
         * Bring the next byte into the remainder.
         */
        remainder ^= (message[byte] << (CRC_WIDTH - 8));

        /*
         * Perform modulo-2 division, a bit at a time.
         */
        for (uint8_t bit = 8; bit > 0; --bit)
        {
            /*
             * Try to divide the current data bit.
             */
            if (remainder & CRC_TOPBIT)
            {
                remainder = (remainder << 1) ^ CRC_POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    /*
     * The final remainder is the CRC result.
     */
    return (remainder);
}

/******************************************************************************
 * Public Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Initializes capture of both analog and digital signals.
 *
 * Loads calibration data from EEPROM.
 *
 *****************************************************************************/
cmd_status_t calibrate_Init(void)
{
  memset(&calibrationResult, 0, sizeof(calib_result_t));
//   haveAnalogOut = haveAnalogIn = saved = FALSE;
  log_d("ENTER");
  calibrationState = CALIB_STATE_STOPPED;
  currentVdivIndex = -1;

  // Must initialize capture before starting to collect samples
  capture_Init();

  spi_dac_init();

  spi_eeprom_init();

  return calibrate_LoadCalibrationData(&calibrationResult);
}

/**************************************************************************//**
 *
 * @brief  Enables the analog outputs and sets them to the specified level
 *
 * Used by the client software during calibration. This function is called 
 * multiple times and between each call the user must measure (using a multimeter)
 * what the actual value is on each of the analog outputs. 
 *
 * The user's measurements and the level each of them were set to will then
 * be used to calculate the calibration factors for any analog output.
 *
 * @param [in] cfg    Must be the calib_analog_out_parameters_t struct
 * @param [in] size   Must be the size of the calib_analog_out_parameters_t struct
 *
 * @retval CMD_STATUS_OK                            If successful
 * @retval CMD_STATUS_ERR_CAL_AOUT_INVALID_PARAMS   If the parameters were wrong
 *
 *****************************************************************************/
cmd_status_t calibrate_AnalogOut(uint8_t* cfg, uint32_t size)
{
  cmd_status_t status;
  calib_analog_out_parameters_t* params;

  log_d("ENTER");
  do {
    status = statemachine_RequestState(STATE_CALIBRATING);
    if (status != CMD_STATUS_OK)
    {
      break;
    }

    if (size != sizeof(calib_analog_out_parameters_t))
    {
      status = CMD_STATUS_ERR_CAL_AOUT_INVALID_PARAMS;
      break;
    }

    params = (calib_analog_out_parameters_t*)cfg;
    if (params->level > 0x3ff)
    {
      status = CMD_STATUS_ERR_CAL_AOUT_INVALID_PARAMS;
      break;
    }

    calibrationState = CALIB_STATE_AOUT;

    log_d("Setting level to 0x%03x", params->level);

    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_A, (params->level << 2)));
    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_B, (params->level << 2)));

  } while (FALSE);

  log_d("LEAVE, Status = %u", status);
  return status;
}

/**************************************************************************//**
 *
 * @brief  Starts the calibration of the analog inputs
 *
 * Note: Before this function is called the user should have connected 
 *       A_OUT1 to OSC_IN1 and A_OUT2 to OSC_IN2. Otherwise the measurements
 *       will be off, resulting in bad calibration data.
 *
 * The calibration is done in these steps:
 *  -# Analog outputs 1 and 2 are set to LOW level for 20mV/div
 *  -# Analog inputs capture data on both channels
 *  -# The data is inspected to find and save the average level for each channel
 *  -# Analog outputs 1 and 2 are set to HIGH level for 20mV/div
 *  -# Analog inputs capture data on both channels
 *  -# The data is inspected to find and save the average level for each channel
 *
 * The steps are then repeated for each of the Volts/div levels and when all 
 * measurements have been taken the result is sent to the client.
 *
 * Each of these steps takes time to complete and running them all after each
 * other would break the USB stack which needs to be called at a regular interval.
 * The solution is to let the \ref usb_handler_Run function repeatedly call 
 * \ref calibrate_Feed until all processing is done.
 *
 * @param [in] cfg    Must be the calib_analog_in_parameters_t struct
 * @param [in] size   Must be the size of the calib_analog_in_parameters_t struct
 *
 * @retval CMD_STATUS_OK                            If successful
 * @retval CMD_STATUS_ERR_CAL_AIN_INVALID_PARAMS    If the parameters were wrong
 *
 *****************************************************************************/
cmd_status_t calibrate_AnalogIn(uint8_t* cfg, uint32_t size)
{
  cmd_status_t status;
  calib_analog_in_parameters_t* params;
  int i;

  log_d("ENTER");
  do {
    status = statemachine_RequestState(STATE_CALIBRATING);
    if (status != CMD_STATUS_OK)
    {
      break;
    }

    if (size != sizeof(calib_analog_in_parameters_t))
    {
      status = CMD_STATUS_ERR_CAL_AIN_INVALID_PARAMS;
      break;
    }

    params = (calib_analog_in_parameters_t*)cfg;
    for (i = 0; i < 3; i++)
    {
      calibrationResult.userOut[0][i] = params->measA0[i];
      calibrationResult.userOut[1][i] = params->measA1[i];
      calibrationResult.dacValOut[i] = params->levels[i];
      log_d("Analog out: DAC 0x%3x => A0 %4dmV,  A1 %4dmV",
            calibrationResult.dacValOut[i],
            calibrationResult.userOut[0][i],
            calibrationResult.userOut[1][i]);
    }
//     haveAnalogOut = TRUE;

//     log_d("A0: { %d, %d, %d }", params->measLowA0, params->measZeroA0, params->measHighA0);
//     log_d("A1: { %d, %d, %d }", params->measLowA1, params->measZeroA1, params->measHighA1);

    // Set the target output LOW voltage in mV for each of the different Volts/div levels
    calibrationResult.voltsInLow[0] = -80;
    calibrationResult.voltsInLow[1] = -200;
    calibrationResult.voltsInLow[2] = -400;
    calibrationResult.voltsInLow[3] = -800;
    calibrationResult.voltsInLow[4] = -2000;
    calibrationResult.voltsInLow[5] = -2500;
    calibrationResult.voltsInLow[6] = -2500;
    calibrationResult.voltsInLow[7] = -2500;

    // Set the target output HIGH voltage in mV for each of the different Volts/div levels.
    // The high levels are the positive 
    for (i = 0; i < 8; i++)
    {
      calibrationResult.voltsInHigh[i] = - calibrationResult.voltsInLow[i];
    }

    calibrationState = CALIB_STATE_AIN_SETUP_LOW;
    currentVdivIndex = 0;

  } while (FALSE);

  log_d("LEAVE, Status = %u", status);
  return status;
}

/**************************************************************************//**
 *
 * @brief  Terminates the calibration sequence and returns everything to "normal"
 *
 *****************************************************************************/
void calibrate_Stop(void)
{
  calibrationState = CALIB_STATE_STOPPED;
  statemachine_RequestState(STATE_IDLE);
}

/**************************************************************************//**
 *
 * @brief  Drives the calibration of analog inputs
 *
 * Called repeatedly by \ref usb_handler_Run until all needed measurements
 * have been taken. 
 *
 * The user's measurements and the level each of them were set to will then
 * be used to calculate the calibration factors for any analog output.
 *
 *  \dot
 *  digraph example {
 *      node [shape=oval, fontname=Helvetica, fontsize=10];
 *      init [ label="INIT", shape="doublecircle", URL="\ref calibrate_AnalogIn" ];
 *      low [ label="CALIB_STATE_AIN_SETUP_LOW" ];
 *      high [ label="CALIB_STATE_AIN_SETUP_HIGH" ];
 *      proc [ label="CALIB_STATE_AIN_PROCESS" ];
 *      sleep [ label="CALIB_STATE_AIN_SLEEP" ];
 *      wait [ label="CALIB_STATE_AIN_WAIT" ];
 *      stop [ label="CALIB_STATE_AIN_STOPPING" ];
 *      init -> low [ arrowhead="open", style="solid", label="start with low output for first V/div" ];
 *      low -> sleep [ arrowhead="open", style="solid", label="changed analog output" ];
 *      sleep -> sleep [ arrowhead="open", style="solid" ];
 *      sleep -> proc [ arrowhead="open", style="solid" ];
 *      proc -> wait [ arrowhead="open", style="solid", label="request analog input sampling" ];
 *      wait -> wait [ arrowhead="open", style="solid" ];
 *      wait -> stop [ arrowhead="open", style="solid", label="calibration done" ];
 *      proc -> stop [ arrowhead="open", style="solid", label="an error ocurred" ];
 *      wait -> high [ arrowhead="open", style="solid", label="proceed with high output for current V/div" ];
 *      wait -> low [ arrowhead="open", style="solid", label="proceed with low output for next V/div" ];
 *      high -> sleep [ arrowhead="open", style="solid", label="changed analog output" ];
 *  }
 *  \enddot
 *
 *****************************************************************************/
void calibrate_Feed(void)
{
  static int sleepTime = 0;
  uint32_t tmpA, tmpB;

  if (calibrationState == CALIB_STATE_AIN_SETUP_LOW)
  {
    tmpA = calibrate_12BitCalibratedDAC(0, calibrationResult.voltsInLow[currentVdivIndex]);
    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_A, tmpA));
    tmpB = calibrate_12BitCalibratedDAC(1, calibrationResult.voltsInLow[currentVdivIndex]);
    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_B, tmpB));
    log_d("Changed output to %dmV  (A 0x%03x, B 0x%03x)", calibrationResult.voltsInLow[currentVdivIndex], tmpA, tmpB);

    measuringLowLevel = TRUE;

    //TIM_Waitms(10);
    sleepTime = 10;
    calibrationState = CALIB_STATE_SLEEP;
  }
  else if (calibrationState == CALIB_STATE_AIN_SETUP_HIGH)
  {
    tmpA = calibrate_12BitCalibratedDAC(0, calibrationResult.voltsInHigh[currentVdivIndex]);
    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_A, tmpA));
    tmpB = calibrate_12BitCalibratedDAC(1, calibrationResult.voltsInHigh[currentVdivIndex]);
    spi_dac_write(SPI_DAC_VALUE(SPI_DAC_OUT_B, tmpB));
    log_d("Changed output to %dmV  (A 0x%03x, B 0x%03x)", calibrationResult.voltsInHigh[currentVdivIndex], tmpA, tmpB);

    measuringLowLevel = FALSE;

    sleepTime = 10;
    calibrationState = CALIB_STATE_SLEEP;
  }

  else if (calibrationState == CALIB_STATE_AIN_PROCESS)
  {
    cmd_status_t res = capture_ConfigureForCalibration(currentVdivIndex);
    if (res == CMD_STATUS_OK)
    {
      calibrationState = CALIB_STATE_AIN_WAIT;
    }
    else
    {
      usb_handler_SignalFailedCalibration(res);
      calibrationState = CALIB_STATE_STOPPING;
    }
  }
  else if (calibrationState == CALIB_STATE_SLEEP)
  {
    if (sleepTime <= 0)
    {
      calibrationState = CALIB_STATE_AIN_PROCESS;
      log_d("Waking up from sleep");
    }
    else
    {
      // This is not exact, but that is not needed. The important thing
      // is that a long sleep (ca 1 second) can be divided into small
      // chunks allowing USB polling at the same time
      TIM_Waitms(10);
      sleepTime -= 10;
    }
  }
  else if (calibrationState == CALIB_STATE_STOPPING)
  {
    // The usb_handler have had enough time to send the result/error now
    calibrationState = CALIB_STATE_STOPPED;
  }
}

/**************************************************************************//**
 *
 * @brief  Processes the collected analog input samples and calculates averages
 *
 * Finds and stores the average values for each of the analog input channels
 * available in the collected samples.
 *
 * @param [in] status   Result of the last capture of analog inputs
 * @param [in] buff     The captured samples
 *
 *****************************************************************************/
void calibrate_ProcessResult(cmd_status_t status, circbuff_t* buff)
{
  uint16_t* pSamples;
  uint16_t  ch;
  uint16_t  tmp;
  int       numSamples;

  if ((status != CMD_STATUS_OK) || (buff == NULL))
  {
    // aborting
    usb_handler_SignalFailedCalibration(status);
    return;
  }

  memset(stats, 0, sizeof(uint32_t)*2*NUMBER_OF_STATS);
  stats[0][STATS_MIN] = stats[1][STATS_MIN] = 0xffffff; // way above any valid value

  // case where the circular buffer hasn't wrapped yet
  if (buff->empty)
  {
    numSamples = buff->last/2; //last is in bytes
    pSamples = (uint16_t*)buff->data;
    while (--numSamples > 0)
    {
      ch = (*pSamples & 0x7000)>>12;
      tmp = *pSamples & 0x0fff;

      stats[ch][STATS_NUM]++;
      stats[ch][STATS_SUM] += tmp;
      if (tmp < stats[ch][STATS_MIN])
      {
        stats[ch][STATS_MIN] = tmp;
      }
      if (tmp > stats[ch][STATS_MAX])
      {
        stats[ch][STATS_MAX] = tmp;
      }

      pSamples++;
    }
  }
  else
  {
    numSamples = (buff->size - buff->last)/2; //size is in bytes
    pSamples = (uint16_t*)circbuff_GetFirstAddr(buff);
    while (--numSamples > 0)
    {
      ch = (*pSamples & 0x7000)>>12;
      tmp = *pSamples & 0x0fff;

      stats[ch][STATS_NUM]++;
      stats[ch][STATS_SUM] += tmp;
      if (tmp < stats[ch][STATS_MIN])
      {
        stats[ch][STATS_MIN] = tmp;
      }
      if (tmp > stats[ch][STATS_MAX])
      {
        stats[ch][STATS_MAX] = tmp;
      }

      pSamples++;
    }
    numSamples = buff->last/2; //last is in bytes
    pSamples = (uint16_t*)buff->data;
    while (--numSamples > 0)
    {
      ch = (*pSamples & 0x7000)>>12;
      tmp = *pSamples & 0x0fff;

      stats[ch][STATS_NUM]++;
      stats[ch][STATS_SUM] += tmp;
      if (tmp < stats[ch][STATS_MIN])
      {
        stats[ch][STATS_MIN] = tmp;
      }
      if (tmp > stats[ch][STATS_MAX])
      {
        stats[ch][STATS_MAX] = tmp;
      }

      pSamples++;
    }
  }

  for (ch = 0; ch < 2; ch++)
  {
    log_i("Stats: V/div %4dmV: %4dmV: CH%d: Num: %5u, Min %4u (0x%03x), Max %4u (0x%03x), Avg: %4u (0x%03x)\r\n",
          cap_vadc_GetMilliVoltsPerDiv(ch),
          (measuringLowLevel ? calibrationResult.voltsInLow[currentVdivIndex] : calibrationResult.voltsInHigh[currentVdivIndex]),
          ch, stats[ch][STATS_NUM],
          stats[ch][STATS_MIN], stats[ch][STATS_MIN],
          stats[ch][STATS_MAX], stats[ch][STATS_MAX],
          (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]),
          (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]));

    // save the result
    if (measuringLowLevel)
    {
      calibrationResult.inLow[ch][currentVdivIndex] = (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]);
    }
    else
    {
      calibrationResult.inHigh[ch][currentVdivIndex] = (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]);
    }
  }

  if (measuringLowLevel)
  {
    // measure at high level as well
    calibrationState = CALIB_STATE_AIN_SETUP_HIGH;
  }
  else
  {
    currentVdivIndex++;
    if (currentVdivIndex == 8)
    {
      // done with all measurements
      usb_handler_SendCalibrationResult(&calibrationResult);
      calibrationState = CALIB_STATE_STOPPING;
    }
    else
    {
      // measure at low level for the new V/div setting
      calibrationState = CALIB_STATE_AIN_SETUP_LOW;
    }
  }
}

/**************************************************************************//**
 *
 * @brief  Loads the calibration data from the EEPROM
 *
 * If the EEPROM cannot be read or doesn't contain valid calibration data that
 * this version of the software can load the \a data parameter will be filled
 * with the default values.
 *
 * The \a data parameter will always contain vaild data after this call.
 *
 * @param [out] data   Valid calibration data
 *
 * @retval CMD_STATUS_OK
 *
 *****************************************************************************/
cmd_status_t calibrate_LoadCalibrationData(calib_result_t* data)
{
  uint32_t actualCrc;
  uint16_t len;

  //spi_eeprom_init();
  len = spi_eeprom_read((uint8_t*)data, 0, sizeof(calib_result_t));

//   log_i("After reading...\r\n");
//   display_buffer_hex((uint8_t*)data, sizeof(calib_result_t));

  if (len != sizeof(calib_result_t))
  {
    log_i("Loading of calibration data failed. Only read %u bytes, expected %u. Reverting to default values\r\n",
          len, sizeof(calib_result_t));
  }
  else
  {
    // Calculate checksum on all but the checksum field itself
    actualCrc = crcSlow((uint8_t*) &data->version, sizeof(calib_result_t)-sizeof(uint32_t));

    if (actualCrc != data->checksum)
    {
      log_i("Loading of calibration data failed. Invalid CRC. Reverting to default values\r\n");
    }
    else if (data->version != CALIBRATION_VERSION)
    {
      log_i("Loading of calibration data failed. Invalid version %#x, expected %#x. Reverting to default values\r\n");
    }
    else
    {
      log_i("Successfully loaded calibration data from EEPROM\r\n");
      return CMD_STATUS_OK;
    }
  }

  // prepare default calibration data
  memcpy(data, &DEFAULT_CALIBRATION, sizeof(calib_result_t));

  return CMD_STATUS_OK;
}

/**************************************************************************//**
 *
 * @brief  Stores the specified calibration data in the EEPROM
 *
 * The calibration data is stamped with the \ref CALIBRATION_VERSION and then
 * a checksum is calculated. The checksum, version and calibration data is then
 * written to the EEPROM.
 *
 * After writing the data it is read back with \ref calibrate_LoadCalibrationData
 * and the checksum of the read data is compared with that of \a data to make
 * sure that the EEPROM really is updated.
 *
 * @param [in] data   Calibration data to store persistently
 *
 * @retval CMD_STATUS_OK                            If successful
 * @retval CMD_STATUS_ERR_CAL_FAILED_TO_STORE_DATA  If the data could not be reread
 *
 *****************************************************************************/
cmd_status_t calibrate_StoreCalibrationData(const calib_result_t * const data)
{
  cmd_status_t res;
  uint32_t crcWritten;

  //spi_eeprom_init();
  // create a copy to not manipulate the
  memcpy(&calibrationResult, data, sizeof(calib_result_t));

  calibrationResult.version = CALIBRATION_VERSION;

  // Calculate checksum on all but the checksum field itself
  crcWritten = crcSlow((uint8_t*) &calibrationResult.version, sizeof(calib_result_t)-sizeof(uint32_t));
  calibrationResult.checksum = crcWritten;

//   log_i("Before writing...\r\n");
//   display_buffer_hex((uint8_t*)&calibrationResult, sizeof(calib_result_t));

  spi_eeprom_write((uint8_t*)&calibrationResult, 0, sizeof(calib_result_t));

  TIM_Waitms(200);

  memset(&calibrationResult, 0, sizeof(calib_result_t));
  res = calibrate_LoadCalibrationData(&calibrationResult);
  if (res == CMD_STATUS_OK)
  {
    if (crcWritten != calibrationResult.checksum)
    {
      log_i("Readback of written calibration data does not match\r\n");
      res = CMD_STATUS_ERR_CAL_FAILED_TO_STORE_DATA;
    }
  }
  return res;
}

/**************************************************************************//**
 *
 * @brief  Erases the calibration data stored in the EEPROM
 *
 * To reduce the number of writes only the checksum and version data is 
 * overwritten in the EEPROM. 
 *
 * @retval CMD_STATUS_OK     If successful
 * @retval CMD_STATUS_ERR_*  If the data could not be erased
 *
 *****************************************************************************/
cmd_status_t calibrate_EraseCalibrationData(void)
{
  // enough to erase the first bytes containing the version and checksum
  uint32_t data[2] = { DEFAULT_TOKEN, DEFAULT_TOKEN };

  //spi_eeprom_init();
  spi_eeprom_write((uint8_t*)data, 0, sizeof(data));

  TIM_Waitms(100);

  // Load the default calibration data
  memset(&calibrationResult, 0, sizeof(calib_result_t));
  return calibrate_LoadCalibrationData(&calibrationResult);
}

/**************************************************************************//**
 *
 * @brief  Returns a pointer to the active calibration data
 *
 *****************************************************************************/
const calib_result_t* calibrate_GetActiveCalibrationData(void)
{
  return &calibrationResult;
}

/**************************************************************************//**
 *
 * @brief  Calculates the scaling factors for the analog outputs
 *
 * The A and B factors can be used to calculate a value for dac like this:
 *
 * dac_val = (wantedVolts - a) / b
 *
 * @param [in] ch   Channel to get factors for (0 or 1)
 * @param [out] a   Scale factor A
 * @param [out] b   Scale factor B
 *
 *****************************************************************************/
void calibrate_GetFactorsForDAC(int ch, double* a, double* b)
{
  double vOut1 = calibrationResult.userOut[ch][0]; // low level
  double vOut2 = calibrationResult.userOut[ch][2]; // high level
  double hex1 = calibrationResult.dacValOut[0]; //256; // -2.5V
  double hex2 = calibrationResult.dacValOut[2]; //768; // +2.5V

  vOut1  = vOut1 / 1000.0; // convert from mV to V
  vOut2  = vOut2 / 1000.0; // convert from mV to V

  *a = (vOut1 - (vOut2*hex1/ hex2)) / (1 - (hex1/hex2));
  *b = (vOut2 - *a) / hex2;
}
