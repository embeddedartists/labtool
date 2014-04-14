/*!
 * @file
 * @brief   Handles all USB communication
 * @ingroup IP_USB
 * @ingroup FUNC_COMM
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

#include "usb_handler.h"
#include "lpc43xx_cgu.h"
#include "lpc43xx_timer.h"
#include "lpc43xx_wwdt.h"
#include "led.h"
#include "log.h"


/******************************************************************************
 * Typedefs and defines
 *****************************************************************************/

//Watchdog time out in 5 seconds (in us)
#define WDT_INTERRUPT_TIMEOUT 	5000000 	// max value is (0xFFFFFF*4)/12000000 = 5.59s

//Watchdog warn in 3 seconds (in us)
#define WDT_WARNING_VALUE 		3000000

#define CMD_MAX_LEN     4

/*! @brief Maximum size of a received block of data
 * @see LabTool_ReadData
 * @todo Remove 512 byte limitation of received data?
 */
#define DATA_MAX_LEN  512

#define HEADER_IDX_SIZE_LSB  0
#define HEADER_IDX_SIZE_MSB  1
#define HEADER_IDX_CMD       2
#define HEADER_IDX_PREFIX    3

#define CMD_SIZE(__buff)      (*((uint16_t*)(__buff)))
#define CMD_IS_VALID(__buff)  (((__buff)[HEADER_IDX_PREFIX])==0xea)
#define CMD_HAS_DATA(__buff)  (CMD_IS_VALID(__buff) && (CMD_SIZE(__buff) > 0) && (CMD_SIZE(__buff) <= DATA_MAX_LEN))

/*! @brief Callbacks used when client requests are received.
 * @see LabTool_ProcessCommand
 */
typedef struct
{
  cmdFunc      capStop;
  cmdFunc      capRun;
  cmdFuncParam capConfigure;

  cmdFunc      genStop;
  cmdFunc      genRun;
  cmdFuncParam genConfigure;
} callbacks_t;

/*! @brief Collection of captured samples and the status. */
typedef struct
{
  cmd_status_t       status;
  captured_samples_t cap;
} sample_data_t;

/*! @brief Collection of calibration data and the status. */
typedef struct
{
  cmd_status_t       status;
  calib_result_t     parameters;
} calibration_data_t;

/*! Commands sent on the USB Bulk interface */
typedef enum
{
  CMD_GEN_CFG     = 1, /*!< Configure signal generation */
  CMD_GEN_RUN     = 2, /*!< Start signal generation */

  CMD_CAP_CFG     = 3, /*!< Configure signal capturing */
  CMD_CAP_RUN     = 4, /*!< Start/Arm signal capturing */
  CMD_CAP_SAMPLES = 5, /*!< Send captured signal samples */

  CMD_CAL_INIT       =  7, /*!< Initialize the calibration sequence */
  CMD_CAL_ANALOG_OUT =  8, /*!< Calibrate the analog outputs */
  CMD_CAL_ANALOG_IN  =  9, /*!< Calibrate the analog inputs */
  CMD_CAL_RESULT     = 10, /*!< Result of the calibration */
  CMD_CAL_STORE      = 11, /*!< Store the calibration data in EEPROM */
  CMD_CAL_ERASE      = 12, /*!< Erase the calibration data from EEPROM */
  CMD_CAL_END        = 13, /*!< End the calibration sequence */

  CMD_NUM_COMMANDS
} protocol_commands_t;

/*! Commands sent as USB Control Requests */
typedef enum
{
  REQ_GetPll1Speed  = 1, /*!< Request for the speed of PLL1 */
  REQ_Ping          = 2, /*!< Ping to indicate active line */
  REQ_StopCapture   = 3, /*!< Request to stop ongoing signal capture */
  REQ_StopGenerator = 4, /*!< Request to stop ongoing signal generation */
  REQ_GetCalibData  = 5, /*!< Request for the persistent calibration data */
} control_requests_t;

/******************************************************************************
 * Global variables
 *****************************************************************************/

extern volatile uint32_t mstick;

/******************************************************************************
 * Local variables
 *****************************************************************************/

static uint8_t cmd_buff[CMD_MAX_LEN];
static uint8_t data_buff[DATA_MAX_LEN];

static callbacks_t callbacks;

// Samples to send back to PC
static sample_data_t samples = {CMD_STATUS_ERR,NULL,0,0};
static Bool haveSamplesToSend = FALSE;

// Calibration result to send back to PC
static calibration_data_t calibration;
static Bool haveCalibrationResultToSend = FALSE;

static volatile Bool usbConnected;

static Bool stopCaptureRequested = FALSE;
static Bool stopGeneratorRequested = FALSE;

/******************************************************************************
 * Forward Declarations of Local Functions
 *****************************************************************************/

void EVENT_USB_Device_Connect(void);
void EVENT_USB_Device_Disconnect(void);
void EVENT_USB_Device_ConfigurationChanged(void);
void EVENT_USB_Device_ControlRequest(void);

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Reads one command from the USB bulk endpoint
 *
 * The function is not blocking.
 *
 * A valid command contains have four bytes like this:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      start [label="0xEA | Command | {Payload Size | { LSB | MSB } }"];
 *  }
 *  \enddot
 *
 * If the received data doesn't start with such a header then it is discarded.
 *
 * @param [out] pCmd  The received command
 * @param [out] pSize The number of payload bytes in the command
 *
 * @retval TRUE  If a command was found
 * @retval FALSE If no command was found
 *
 *****************************************************************************/
static Bool LabTool_ReadCommand(uint8_t* pCmd, uint16_t* pSize)
{
  Bool validCommand = FALSE;

  if (Endpoint_IsOUTReceived())
  {
    Endpoint_Read_Stream_LE(cmd_buff, CMD_MAX_LEN, NULL);
    Endpoint_ClearOUT();

    if ((cmd_buff[HEADER_IDX_PREFIX] == 0xea) &&
        (cmd_buff[HEADER_IDX_CMD] < CMD_NUM_COMMANDS))
    {
      *pCmd = cmd_buff[HEADER_IDX_CMD];
      *pSize = cmd_buff[HEADER_IDX_SIZE_LSB];
      *pSize |= (cmd_buff[HEADER_IDX_SIZE_MSB] << 8);

      validCommand = TRUE;
    }
    else
    {
      log_i("Got invalid CMD from PC: 0x%08x  { 0x%02x, 0x%02x, 0x%02x, 0x%02x }\r\n", *((uint32_t*)cmd_buff), cmd_buff[0], cmd_buff[1], cmd_buff[2], cmd_buff[3]);
    }
  }
  return validCommand;
}

/**************************************************************************//**
 *
 * @brief  Reads up data on the USB bulk endpoint
 *
 * A maximum of 512 bytes can be read like this.
 *
 * If no data is available when the function is called, then the function will
 * block waiting for the data. The function will wakeup every 10ms to see if
 * anything has arrived. A maximum of 5 seconds will be spent retrying before
 * giving up.
 *
 * @todo Investigate if this delay causes disconnects between client and device
 *
 * @param [in,out] pBuff  The buffer to store read data in
 * @param [in]     size   The number of bytes to read, max 512 bytes
 *
 * @retval TRUE  If the data was successfully read
 * @retval FALSE If the data was not read
 *
 *****************************************************************************/
static Bool LabTool_ReadData(uint8_t* pBuff, uint16_t size)
{
  int retry = 500;

  if (size > DATA_MAX_LEN)
  {
    return FALSE;
  }
  do
  {
    if (Endpoint_IsOUTReceived())
    {
      Endpoint_Read_Stream_LE(pBuff, size, NULL);
      Endpoint_ClearOUT();
      return TRUE;
    }
    TIM_Waitms(10);
  } while (--retry > 0);

  return FALSE;
}

/**************************************************************************//**
 *
 * @brief  Sends a response to a command back to the client software
 *
 * The samples are sent on the bulk endpoint.
 *
 * The message is formatted like this:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      start [label="0xEA | Command to respond to | 0x00 | Error Code"];
 *  }
 *  \enddot
 * Which in total makes up one 32 bit word.
 *
 * Example 1: Receives command \a CMD_CAP_CFG (3) and successfully completes it:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="0xEA030000"];
 *  }
 *  \enddot
 *
 * Example 2: Receives command \a CMD_GEN_RUN (2) but fails with \a CMD_STATUS_ERR_GEN_INVALID_TYPE (26):
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="0xEA02001A"];
 *  }
 *  \enddot
 *
 *****************************************************************************/
static void LabTool_SendResponse(uint8_t cmd, cmd_status_t status)
{
  /* Select the IN stream endpoint */
  Endpoint_SelectEndpoint(LABTOOL_IN_EPNUM);

  Endpoint_Write_32_LE(0xEA000000 | (cmd<<16) | (status&0xff));
  Endpoint_ClearIN();
}

/**************************************************************************//**
 *
 * @brief  Preforms the action of a received command (if any)
 *
 * Tests if a new command has been received and if it has then the corresponding
 * action is taken. After the action has completed the result is sent back
 * to the client over USB.
 *
 * \see LabTool_ReadCommand
 * \see LabTool_SendResponse
 *
 *****************************************************************************/
static void LabTool_ProcessCommand(void)
{
  cmd_status_t status;
  uint8_t cmd;
  uint16_t size;

  /* Device must be connected and configured for the task to run */
  if (USB_DeviceState != DEVICE_STATE_Configured)
  {
    return;
  }

  /* Select the OUT stream endpoint */
  Endpoint_SelectEndpoint(LABTOOL_OUT_EPNUM);

  if (LabTool_ReadCommand(&cmd, &size))
  {
    switch(cmd)
    {
      case CMD_GEN_RUN:
        log_i("Got generator RUN command\r\n");
        stopGeneratorRequested = FALSE;
        status = callbacks.genRun();
        LabTool_SendResponse(CMD_GEN_RUN, status);
        break;

      case CMD_GEN_CFG:
        log_i("Got generator CFG command\r\n");
        stopGeneratorRequested = FALSE;
        if (LabTool_ReadData(data_buff, size))
        {
          status = callbacks.genConfigure(data_buff, size);
          LabTool_SendResponse(CMD_GEN_CFG, status);
        }
        else
        {
          log_i("Failed to read generator config payload, ignoring command\r\n");
          LabTool_SendResponse(CMD_GEN_CFG, CMD_STATUS_ERR);
        }
        break;

      case CMD_CAP_RUN:
        log_i("Got capture RUN command\r\n");
        stopCaptureRequested = FALSE;
        status = callbacks.capRun();
        LabTool_SendResponse(CMD_CAP_RUN, status);
        break;

      case CMD_CAP_CFG:
        log_i("Got capture CFG command\r\n");
        stopCaptureRequested = FALSE;
        if (LabTool_ReadData(data_buff, size))
        {
          status = callbacks.capConfigure(data_buff, size);
          LabTool_SendResponse(CMD_CAP_CFG, status);
        }
        else
        {
          log_i("Failed to read capture config payload, ignoring command\r\n");
          LabTool_SendResponse(CMD_CAP_CFG, CMD_STATUS_ERR);
        }
        break;

      case CMD_CAL_INIT:
        log_i("Got calibration INIT command\r\n");
        status = calibrate_Init();
        LabTool_SendResponse(CMD_CAL_INIT, status);
        break;

      case CMD_CAL_ANALOG_OUT:
        log_i("Got calibration ANALOG_OUT command\r\n");
        if (LabTool_ReadData(data_buff, size))
        {
          status = calibrate_AnalogOut(data_buff, size);
          LabTool_SendResponse(CMD_CAL_ANALOG_OUT, status);
        }
        else
        {
          log_i("Failed to read calibration config payload, ignoring command\r\n");
          LabTool_SendResponse(CMD_CAL_ANALOG_OUT, CMD_STATUS_ERR);
        }
        break;

      case CMD_CAL_ANALOG_IN:
        log_i("Got calibration ANALOG_IN command\r\n");
        if (LabTool_ReadData(data_buff, size))
        {
          status = calibrate_AnalogIn(data_buff, size);
          LabTool_SendResponse(CMD_CAL_ANALOG_IN, status);
        }
        else
        {
          log_i("Failed to read calibration config payload, ignoring command\r\n");
          LabTool_SendResponse(CMD_CAL_ANALOG_IN, CMD_STATUS_ERR);
        }
        break;

      case CMD_CAL_STORE:
        log_i("Got calibration STORE command\r\n");
        if (LabTool_ReadData(data_buff, size))
        {
          // Must skip the first 4 bytes coming from the client as it contains
          // invalid data (i.e. the "cmd" member of the client's calib_result_t)
          status = calibrate_StoreCalibrationData((calib_result_t*)(&data_buff[4]));
          LabTool_SendResponse(CMD_CAL_STORE, status);
          if (status == CMD_STATUS_OK)
          {
            // a successful saving of the data indicates that the calibration
            // sequence has ended
            calibrate_Stop();
          }
        }
        else
        {
          log_i("Failed to read calibration config payload, ignoring command\r\n");
          LabTool_SendResponse(CMD_CAL_STORE, CMD_STATUS_ERR);
        }
        break;

      case CMD_CAL_ERASE:
        log_i("Got calibration ERASE command\r\n");
        status = calibrate_EraseCalibrationData();
        if (status == CMD_STATUS_OK)
        {
          // a successful erasing of the data indicates that the calibration
          // sequence has ended
          calibrate_Stop();
        }
        LabTool_SendResponse(CMD_CAL_ERASE, status);
        break;

      case CMD_CAL_END:
        log_i("Got calibration STOP command\r\n");
        calibrate_Stop();
        LabTool_SendResponse(CMD_CAL_END, CMD_STATUS_OK);
        break;

      default:
        log_i("Ignoring unknown command 0x%02x\r\n", cmd);
        break;
    }
  }
}

/**************************************************************************//**
 *
 * @brief  Sends a chunk of data
 *
 * The function blocks until either all data is sent or an error occurs.
 *
 * @param [in] pData  The data to send
 * @param [in] off    The offset from \a pData to start sending from
 * @param [in] size   The number of bytes to send, starting at \a off
 *
 * @retval TRUE  If the data was successfully sent
 * @retval FALSE If the data was not sent
 *
 *****************************************************************************/
static Bool LabTool_SendData(const uint8_t* const pData, uint32_t off, uint32_t size)
{
  uint32_t left = size;
  uint32_t pos  = off;
  uint16_t chunk;
  uint16_t sent;
  int res;

  // Send from first sample until end of circ buffer
  while (left > 0)
  {
    chunk = MIN(0x0000ffff, left);

    res = Endpoint_Write_Stream_LE(pData+pos, chunk, &sent);
    if (res == ENDPOINT_RWSTREAM_NoError)
    {
      if (left == chunk)
      {
        // done
        //Endpoint_ClearIN(); not ok to do if two consecutive calls to SendData are made
        return TRUE;
      }
      else
      {
        // sent one chunk, but more to send
        Endpoint_ClearIN();
        left -= sent;
        pos += sent;
      }
    }
    else if (res == ENDPOINT_RWSTREAM_IncompleteTransfer)
    {
      // sent some more data, but the buffer got filled before all could be sent
      Endpoint_ClearIN();
      left -= sent;
      pos += sent;
    }
    else
    {
      log_i("Failed to send samples\r\n");

//       // If the failure is because the line went down then clearing IN will
//       // cause all to hang as the EP is "primed"
//       if (Endpoint_IsINReady())
//       {
      Endpoint_ClearIN();
//       }
//       else
//       {
//         // For some reason the EP must be "unprimed" to prevent hanging,
//         // and there is no API for this.
//         USB_REG(USB_PORT_SELECTED)->ENDPTFLUSH = 0xFFFFFFFF;
//         while (USB_REG(USB_PORT_SELECTED)->ENDPTFLUSH); /* Wait until all bits are 0 */
//       }
      return FALSE;
    }
  }
  return TRUE;
}

/**************************************************************************//**
 *
 * @brief  Sends the content of the circular buffer over USB
 *
 * The circular buffer is first straightened out to make it appear as one
 * continuous set of samples.
 *
 * @param [in] buff  The data to send
 *
 * @retval TRUE  If the data was successfully sent
 * @retval FALSE If the data was not sent
 *
 *****************************************************************************/
static Bool LabTool_SendBuffer(const circbuff_t * const buff)
{
  Bool success = FALSE;
  if (buff == NULL)
  {
    success = TRUE;
  }
  else
  {
    if (buff->empty)
    {
      if (LabTool_SendData(buff->data, 0, buff->last))
      {
        success = TRUE;
      }
    }
    else
    {
      if (LabTool_SendData((uint8_t*)circbuff_GetFirstAddr(buff), 0, buff->size - buff->last))
      {
        if (LabTool_SendData(buff->data, 0, buff->last))
        {
          success = TRUE;
          log_i("Sent %d (0x%x) bytes from 0x%08x followed by %d (0x%x) bytes from 0x%08x\r\n",
                buff->size - buff->last,
                buff->size - buff->last,
                circbuff_GetFirstAddr(buff),
                buff->last,
                buff->last,
                buff->data);
          log_i("Circbuff {data 0x%08x, size %d (0x%x), last %d (0x%x)}\r\n", (uint32_t)buff->data, buff->size, buff->size, buff->last, buff->last);
        }
      }
    }
  }
  return success;
}

/**************************************************************************//**
 *
 * @brief  Handles synchronization of both analog and digital signals
 *
 * When both analog and digital signals are being sampled they will be in
 * sync (i.e. the same number of samples taken) but the stopping conditions
 * may differ which can cause one channel to have more samples than the other
 * one. To fix this both channels will be streightened and then some samples
 * will be removed from either the end or the start depending on which signal
 * have the most samples. The actual removal is done in the PC client using
 * the \a sampleTrim parameter passed in the header. This function returns
 * that parameter after having repositioned the circular buffer for the analog
 * signal.
 *
 * @retval -X  If X samples should be removed from the start of the data
 * @retval  0  If the data should be left untouched
 * @retval +X  If X samples should be removed from the end of the data
 *
 *****************************************************************************/
static int LabTool_AlignSignals(void)
{
  int signalTrim = 0;

  // Only need to align if both analog and digital signals are sampled and both
  // buffers are filled.
  if (circbuff_Full(samples.cap.sgpio_samples) && circbuff_Full(samples.cap.vadc_samples))
  {
    // As the buffers are both filled, there is the same number of samples in each.
    // Calculate num samples in the buffer
    uint32_t numSamples = circbuff_GetUsedSize(samples.cap.vadc_samples) / (2 * (samples.cap.vadcActiveChannels >> 16));

    // Find last written position in each buffer
    uint32_t offSGPIO = circbuff_GetFirstAddr(samples.cap.sgpio_samples) - (uint32_t)samples.cap.sgpio_samples->data;
    uint32_t offVADC = circbuff_GetFirstAddr(samples.cap.vadc_samples) - (uint32_t)samples.cap.vadc_samples->data;

    // Convert position into a sample index
    uint32_t idxSGPIO = (32*offSGPIO) / ((samples.cap.sgpioActiveChannels >> 16) * 4);
    uint32_t idxVADC = offVADC / (2 * (samples.cap.vadcActiveChannels >> 16));

    // Move the position in the analog signal buffer so that it aligns with the digital one
    samples.cap.vadc_samples->last = idxSGPIO * 2 * (samples.cap.vadcActiveChannels >> 16);

    // Determine how many samples to discard based on how much further ahead one
    // circular buffer is compared to the other one.
    if (idxSGPIO > idxVADC) {
      uint32_t diffA = idxSGPIO - idxVADC;
      uint32_t diffB = numSamples + idxVADC - idxSGPIO;
      if (diffA < diffB)
      {
        // SGPIO have diffA more samples
        signalTrim = diffA; // remove diffA samples from end
      }
      else
      {
        // VADC has diffB more samples
        signalTrim = -diffB; // remove diffB samples from start
      }
    }
    else
    {
      uint32_t diffA = idxVADC - idxSGPIO;
      uint32_t diffB = numSamples + idxSGPIO - idxVADC;
      if (diffA < diffB)
      {
        // VADC have diffA more samples
        signalTrim = -diffA; // remove diffA samples from start
      }
      else
      {
        // SGPIO has diffB more samples
        signalTrim = diffB; // remove diffB samples from end
      }
    }
  }
  return signalTrim;
}

/**************************************************************************//**
 *
 * @brief  Sends the captured samples to the client software
 *
 * The samples are sent on the bulk endpoint.
 *
 * The message is formatted like this:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="START | Digital Size | Analog Size | Trigger | Digital Trig Sample | Analog Trig Sample | Active Digital Channels | Active Analog Channels | Signal Trim | Digital Data | Analog Data"];
 *  }
 *  \enddot
 * Where each part is 32 bits and \a START is divided into four bytes like this:
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      start [label="0xEA | CMD_CAP_SAMPLES | 0x00 | Error Code"];
 *  }
 *  \enddot
 *
 * Example 1: Sampling of \a DIO_0, \a DIO_2, \a DIO_7 and \a A1 failed with error 12:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="{START|0xEA05000C} | {Digital Size|0x00000000} | {Analog Size|0x00000000} | {Trigger|0x00000000} | {Digital Trig Sample|0x00000000} | {Analog Trig Sample|0x00000000} | {Active Digital Channels|0x00000085} | {Active Analog Channels|0x00000002} | {Signal Trim|0x00000000}"];
 *  }
 *  \enddot
 * Example 2: Successful sampling of \a DIO_3:
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="{START|0xEA050000} | {Digital Size|0x00010000} | {Analog Size|0x00000000} | {Trigger|0x00000000} | {Digital Trig Sample|0x00000100} | {Analog Trig Sample|0x00000000} | {Active Digital Channels|0x00000008} | {Active Analog Channels|0x00000000} | {Signal Trim|0x00000000} | {Digital Data|0x10000 bytes of samples}"];
 *  }
 *  \enddot
 * Example 3: Successful sampling of \a A0 and \a A1:
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="{START|0xEA050000} | {Digital Size|0x00000000} | {Analog Size|0x00010000} | {Trigger|0x00000000} | {Digital Trig Sample|0x00000000} | {Analog Trig Sample|0x00001034} | {Active Digital Channels|0x00000000} | {Active Analog Channels|0x00000003} | {Signal Trim|0x00000000} | {Analog Data|0x10000 bytes of samples}"];
 *  }
 *  \enddot
 * Example 4: Successful sampling of \a DIO_0 .. \a DIO_7 and both analog channels:
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="{START|0xEA050000} | {Digital Size|0x00003200} | {Analog Size|0x0000C800} | {Trigger|0x00000000} | {Digital Trig Sample|0x000000e4} | {Analog Trig Sample|0x00000102} | {Active Digital Channels|0x000000ff} | {Active Analog Channels|0x00000003} | {Signal Trim|0x00000042} | {Digital Data|0x3200 bytes of samples} | {Analog Data|0xC800 bytes of samples}"];
 *  }
 *  \enddot
 *
 *****************************************************************************/
static void LabTool_SendSamples(void)
{
  Bool success = FALSE;

  /* Select the IN stream endpoint */
  Endpoint_SelectEndpoint(LABTOOL_IN_EPNUM);

  // send header
  Endpoint_Write_32_LE(0xEA000000 | (CMD_CAP_SAMPLES<<16) | (samples.status&0xff));
  if (samples.status != CMD_STATUS_OK)
  {
    // Send error status without payload
    Endpoint_Write_32_LE(0); //num sgpio bytes
    Endpoint_Write_32_LE(0); //num vadc bytes
    Endpoint_Write_32_LE(0); //trigpoint
    Endpoint_Write_32_LE(0); //digital trigger sample
    Endpoint_Write_32_LE(0); //analog trigger sample
    Endpoint_Write_32_LE(0); //sgpio active channels
    Endpoint_Write_32_LE(0); //vadc active channels
    Endpoint_Write_32_LE(0); //signal trim
    Endpoint_ClearIN();
    haveSamplesToSend = FALSE;
    return;
  }
  Endpoint_Write_32_LE(circbuff_GetUsedSize(samples.cap.sgpio_samples));
  Endpoint_Write_32_LE(circbuff_GetUsedSize(samples.cap.vadc_samples));
  Endpoint_Write_32_LE(samples.cap.trigpoint);
  Endpoint_Write_32_LE(samples.cap.sgpioTrigSample);
  Endpoint_Write_32_LE(samples.cap.vadcTrigSample);
  Endpoint_Write_32_LE(samples.cap.sgpioActiveChannels);
  Endpoint_Write_32_LE(samples.cap.vadcActiveChannels);
  Endpoint_Write_32_LE(LabTool_AlignSignals());
  Endpoint_ClearIN();

  // send data
  success = LabTool_SendBuffer(samples.cap.sgpio_samples);
  if (success)
  {
    success = LabTool_SendBuffer(samples.cap.vadc_samples);
  }

  if (success)
  {
    Endpoint_ClearIN();
    log_i("All samples sent successfully. Trig %d, Active {SGPIO %#x, VADC %#x}\r\n",
          samples.cap.trigpoint, samples.cap.sgpioActiveChannels, samples.cap.vadcActiveChannels);
  }
  else
  {
    log_e("Failed to send samples to PC\r\n");
  }

  haveSamplesToSend = FALSE;
}

/**************************************************************************//**
 *
 * @brief  Sends the calibration result to the client software
 *
 * The samples are sent on the bulk endpoint.
 *
 * The message is formatted like this:
 *
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      message [label="START | Calibration data (\ref calib_result_t)"];
 *  }
 *  \enddot
 * Where each part is 32 bits and \a START is divided into four bytes like this:
 * \dot
 *  digraph structs {
 *      node [shape=record];
 *      start [label="0xEA | CMD_CAL_RESULT | 0x00 | Error Code"];
 *  }
 *  \enddot
 *
 *****************************************************************************/
static void LabTool_SendCalibrationResult(void)
{
  Bool success = FALSE;

  /* Select the IN stream endpoint */
  Endpoint_SelectEndpoint(LABTOOL_IN_EPNUM);

  // send header
  Endpoint_Write_32_LE(0xEA000000 | (CMD_CAL_RESULT<<16) | (calibration.status&0xff));

  success = LabTool_SendData((uint8_t*)&calibration.parameters, 0, sizeof(calib_result_t));
  if (success)
  {
    Endpoint_ClearIN();
    log_i("Calibration data sent successfully\r\n");
  }
  else
  {
    log_e("Failed to send samples to PC\r\n");
  }

  haveCalibrationResultToSend = FALSE;
}

/**************************************************************************//**
 *
 * @brief  Configures the USB hardware
 *
 *****************************************************************************/
static void SetupHardware(void)
{
  USB_Init();
}

#if (PRINT_ANALOG_HISTOGRAM == OPT_ENABLED)
#include "capture_vadc.h"
#define HIST_CENTER      0x7ff
#define HIST_SIZE        101
#define HIST_BUFF_SIZE   (1 + HIST_SIZE + 1) //above,below and histogram data
#define HIST_LOW_LEVEL   (HIST_CENTER - HIST_SIZE/2)
#define HIST_HIGH_LEVEL  (HIST_CENTER + HIST_SIZE/2)
#define HIST_BELOW_IDX   0
#define HIST_ABOVE_IDX   (HIST_BUFF_SIZE-1)
static uint32_t histBuff[2][HIST_BUFF_SIZE];
static char histPrintBuff[HIST_BUFF_SIZE+1];
static void LabTool_CreateHistogram(void)
{
  int i, ch;
  int numSamples;
  uint16_t* data;
  uint16_t val;
#define HIST_ACCED
#ifdef HIST_ACCED
  static int lastNumChannels = -1;
  static int lastSampleRate = -1;
  static int lastVdiv1 = -1;
  static int lastVdiv2 = -1;
  static int lastNumCollected = 0;
  if ((lastNumChannels != (samples.cap.vadcActiveChannels >> 16)) ||
      (lastSampleRate != capture_GetSampleRate()) ||
      (lastVdiv1 != cap_vadc_GetMilliVoltsPerDiv(0)) ||
      (lastVdiv2 != cap_vadc_GetMilliVoltsPerDiv(1)))
  {
    for (i = 0; i < HIST_BUFF_SIZE; i++)
    {
      histBuff[0][i] = 0;
      histBuff[1][i] = 0;
    }
    lastNumCollected = 0;
    lastNumChannels = (samples.cap.vadcActiveChannels >> 16);
    lastSampleRate = capture_GetSampleRate();
    lastVdiv1 = cap_vadc_GetMilliVoltsPerDiv(0);
    lastVdiv2 = cap_vadc_GetMilliVoltsPerDiv(1);
  }
  lastNumCollected++;
#else
  for (i = 0; i < HIST_BUFF_SIZE; i++)
  {
    histBuff[0][i] = 0;
    histBuff[1][i] = 0;
  }
#endif

  if (samples.cap.vadc_samples != NULL)
  {
    data = (uint16_t*)samples.cap.vadc_samples->data;

    // 2 bytes per sample
    numSamples = circbuff_GetUsedSize(samples.cap.vadc_samples) / 2;

    for (i = 0; i < numSamples; i++)
    {
      val = data[i] & 0xfff; // remove channel information
      ch = ((data[i] >> 12) & 0x7);
      if (val < HIST_LOW_LEVEL)
      {
        histBuff[ch][HIST_BELOW_IDX]++;
      }
      else if (val > HIST_HIGH_LEVEL)
      {
        histBuff[ch][HIST_ABOVE_IDX]++;
      }
      else
      {
        histBuff[ch][val - HIST_LOW_LEVEL]++;
      }
    }

    // show histogram
#if 0
    log_i("HIST[BELOW] = %u\r\n", histBuff[HIST_BELOW_IDX]);
    for (i = 0; i < HIST_SIZE; i++)
    {
      log_i("HIST[0x%03x] = %u%s\r\n", HIST_LOW_LEVEL+i, histBuff[i+1], ((HIST_LOW_LEVEL+i)==HIST_CENTER)?"  <-- CENTER":"");
    }
    log_i("HIST[ABOVE] = %u\r\n", histBuff[HIST_ABOVE_IDX]);
#else

#ifdef HIST_ACCED
    log_i("HIST: (%d accumulated runs) %umV/div, %dHz Sample Rate\r\n", lastNumCollected, cap_vadc_GetMilliVoltsPerDiv(0), capture_GetSampleRate());
#else
    log_i("HIST: %umV/div, %dHz Sample Rate\r\n", cap_vadc_GetMilliVoltsPerDiv(0), capture_GetSampleRate());
#endif
    for (ch = 0; ch < 2; ch++)
    {
      Bool foundFirst = FALSE;
      log_i("HIST: CH%d\r\n", ch);
      for (i = 1000000; i > 0; i/=10)
      {
        int j;
        uint32_t tmp;
        memset(histPrintBuff, ((i==1)?'0':' '), HIST_BUFF_SIZE);
        for (j = 0; j < HIST_BUFF_SIZE; j++)
        {
          tmp = histBuff[ch][j] / i;
          if (tmp > 0)
          {
            histPrintBuff[j] = '0' + (tmp % 10);
            foundFirst = TRUE;
          }
        }
        if (foundFirst || i==10000)
        {
          log_i("HIST: %s\r\n", histPrintBuff);
        }
      }
      memset(histPrintBuff, '-', HIST_BUFF_SIZE);
      histPrintBuff[HIST_BELOW_IDX] = 'B';
      histPrintBuff[HIST_CENTER - HIST_LOW_LEVEL] = 'C';
      histPrintBuff[HIST_ABOVE_IDX] = 'A';
      log_i("HIST: %s\r\n", histPrintBuff);
    }
    log_i("HIST: B is below %#x, C is %#x, A is above %#x\r\n", HIST_LOW_LEVEL, HIST_CENTER, HIST_HIGH_LEVEL);
#endif
  }
}
#endif //if(PRINT_ANALOG_HISTOGRAM == OPT_ENABLED)

#if (FIND_SKIPPED_SAMPLES == OPT_ENABLED)
/**************************************************************************//**
 *
 * @brief  Debugging Aid to help find skipped analog samples
 *
 * When sampling two analog channels at the same time and at a high sample
 * rate occasional "skips" are seen. As all samples have three bits identifying
 * which channel they come from it is possible to detect this by looking for
 * two consecutive samples from the same channel.
 *
 * Sample sequence without skips:
 *
 *     ch1, ch2, ch1, ch2, ch1, ch2, ...
 *
 * Sample sequence with skips:
 *
 *     ch1, ch2, ch1, ch1, ch2, ch1, ...
 *
 * It is not possible to know if only one ch2 sample is skipped above or if
 * it is one ch2 + N*(ch1 and ch2).
 *
 * This function scans the sampled data and prints the findings.
 *
 *****************************************************************************/
static void LabTool_FindSkippedSamples(void)
{
  uint16_t* pSamples;
  uint16_t  lastChannel;
  uint16_t  tmp;
  int       numSkipped = 0;
  int       numSamples;
  circbuff_t* buff = samples.cap.vadc_samples;

  if ((samples.status != CMD_STATUS_OK) || (samples.cap.vadc_samples == NULL))
  {
    // nothing to do here
    return;
  }
  if (samples.cap.vadcActiveChannels != 0x00020003)
  {
    // zero or one active channel, cannot find skipped samples
    return;
  }

  // case where the circular buffer hasn't wrapped yet
  if (buff->empty)
  {
    numSamples = buff->last/2; //last is in bytes
    pSamples = (uint16_t*)buff->data;
    lastChannel = *pSamples & 0x7000;
    while (--numSamples > 0)
    {
      pSamples++;
      tmp = *pSamples & 0x7000;
      if (tmp == lastChannel)
      {
        log_i("Skipped one or more samples at addr %#x, found 2 for channel %d in a row\r\n", (uint32_t)pSamples, tmp>>12);
        numSkipped++;
      }
      lastChannel = tmp;
    }
  }
  else
  {
    numSamples = (buff->size - buff->last)/2; //size is in bytes
    pSamples = (uint16_t*)circbuff_GetFirstAddr(buff);
    lastChannel = *pSamples & 0x7000;
    while (--numSamples > 0)
    {
      pSamples++;
      tmp = *pSamples & 0x7000;
      if (tmp == lastChannel)
      {
        log_i("Skipped one or more samples at addr %#x, found 2 for channel %d in a row\r\n", (uint32_t)pSamples, tmp>>12);
        numSkipped++;
      }
      lastChannel = tmp;
    }
    numSamples = buff->last/2; //last is in bytes
    pSamples = (uint16_t*)buff->data;
    lastChannel = *pSamples & 0x7000;
    while (--numSamples > 0)
    {
      pSamples++;
      tmp = *pSamples & 0x7000;
      if (tmp == lastChannel)
      {
        log_i("Skipped one or more samples at addr %#x, found 2 for channel %d in a row\r\n", (uint32_t)pSamples, tmp>>12);
        numSkipped++;
      }
      lastChannel = tmp;
    }
  }

  log_i("Found a total of %d skipped samples\r\n", numSkipped);
}
#endif //if(FIND_SKIPPED_SAMPLES == OPT_ENABLED)

#if (PRINT_STATISTICS == OPT_ENABLED)
typedef enum
{
  STATS_NUM,
  STATS_MIN,
  STATS_MAX,
  STATS_SUM,

  NUMBER_OF_STATS,
} stats_t;
static uint32_t stats[2][NUMBER_OF_STATS];
static void LabTool_Stats(void)
{
  uint16_t* pSamples;
  uint16_t  ch;
  uint16_t  tmp;
  int       numSamples;
  circbuff_t* buff = samples.cap.vadc_samples;

  if ((samples.status != CMD_STATUS_OK) || (samples.cap.vadc_samples == NULL))
  {
    // nothing to do here
    return;
  }
  if (samples.cap.vadcActiveChannels != 0x00020003)
  {
    // zero or one active channel, cannot find stats (for now)
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
    log_i("Stats: CH%d: Num: %5u, Min %4u (0x%03x), Max %4u (0x%03x), Avg: %4u (0x%03x)\r\n",
          ch, stats[ch][STATS_NUM],
          stats[ch][STATS_MIN], stats[ch][STATS_MIN],
          stats[ch][STATS_MAX], stats[ch][STATS_MAX],
          (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]),
          (stats[ch][STATS_SUM]/stats[ch][STATS_NUM]));
  }
}
#endif //if (PRINT_STATISTICS == OPT_ENABLED)

/******************************************************************************
 * Global Functions
 *****************************************************************************/

/**************************************************************************//**
 *
 * @brief  Event handler for the library USB Connection event.
 *
 *****************************************************************************/
void EVENT_USB_Device_Connect(void)
{
  usbConnected = TRUE;
  LED_USB_CONNECTED_ON();
}

/**************************************************************************//**
 *
 * @brief  Event handler for the library USB Disconnection event.
 *
 *****************************************************************************/
void EVENT_USB_Device_Disconnect(void)
{
  usbConnected = FALSE;
  LED_USB_CONNECTED_OFF();
}

/**************************************************************************//**
 *
 * @brief  Event handler for the library USB Configuration Changed event.
 *
 *****************************************************************************/
void EVENT_USB_Device_ConfigurationChanged(void)
{
  log_d("ENTER");

  for (uint8_t EndpointNum = 1; EndpointNum < ENDPOINT_TOTAL_ENDPOINTS; EndpointNum++)
  {
    uint16_t Size;
    uint8_t  Type;
    uint8_t  Direction;
    bool     DoubleBanked;

    if (EndpointNum == LABTOOL_IN_EPNUM)
    {
      Size         = LABTOOL_IO_EPSIZE;
      Direction    = ENDPOINT_DIR_IN;
      Type         = EP_TYPE_BULK;
      DoubleBanked = false;
    }
    else if (EndpointNum == LABTOOL_OUT_EPNUM)
    {
      Size         = LABTOOL_IO_EPSIZE;
      Direction    = ENDPOINT_DIR_OUT;
      Type         = EP_TYPE_BULK;
      DoubleBanked = false;
    }
    else
    {
      continue;
    }

    if (!(Endpoint_ConfigureEndpoint(EndpointNum, Type, Direction, Size,
                                     DoubleBanked ? ENDPOINT_BANK_DOUBLE : ENDPOINT_BANK_SINGLE)))
    {
      log_d("Failed to configure endpoint %d", EndpointNum);
      return;
    }
  }

  log_d("EXIT - Successful");
}

/**************************************************************************//**
 *
 * @brief  Event handler for the library USB Control Request reception event.
 *
 *****************************************************************************/
void EVENT_USB_Device_ControlRequest(void)
{
  /* This is just a quick example of control requests sent from the host.
     The REQ_GetPll1Speed is a read request and REQ_Ping is without data.
  */
  if (Endpoint_IsSETUPReceived() && (USB_ControlRequest.wIndex == LABTOOL_IF_NUMBER))
  {
    if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_VENDOR | REQREC_INTERFACE))
    {
      const calib_result_t* calib;
      const uint32_t* calibdata;
      int i;

      switch (USB_ControlRequest.bRequest)
      {
        case REQ_GetPll1Speed:
          log_i("Control Request: Get PLL1 Speed\r\n");
          Endpoint_ClearSETUP();
          //while (!(Endpoint_IsINReady()));
          Endpoint_Write_32_LE(SystemCoreClock);
          Endpoint_ClearIN();
          Endpoint_ClearStatusStage();
          break;

        case REQ_GetCalibData:
          log_i("Control Request: Get Calibration Data\r\n");
          calib = calibrate_GetActiveCalibrationData();
          calibdata = (uint32_t*)calib;
          Endpoint_ClearSETUP();
          //while (!(Endpoint_IsINReady()));
          Endpoint_Write_32_LE(0); // "cmd" in the client's calib_result struct
          for (i = 0; i < sizeof(calib_result_t)/4; i++)
          {
            Endpoint_Write_32_LE(calibdata[i]);
          }
          Endpoint_ClearIN();
          Endpoint_ClearStatusStage();
          break;
      }
    }
    else if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_VENDOR | REQREC_INTERFACE))
    {
      switch (USB_ControlRequest.bRequest)
      {
        case REQ_Ping:
          log_i("Control Request: Ping\r\n");
          Endpoint_ClearSETUP();
          Endpoint_ClearStatusStage();
          break;
        case REQ_StopCapture:
          log_i("Control Request: Stop Capture\r\n");
          stopCaptureRequested = TRUE;
          haveSamplesToSend = FALSE;
          Endpoint_ClearSETUP();
          Endpoint_ClearStatusStage();
          break;
        case REQ_StopGenerator:
          log_i("Control Request: Stop Generator\r\n");
          stopGeneratorRequested = TRUE;
          Endpoint_ClearSETUP();
          Endpoint_ClearStatusStage();
          break;
      }
    }
  }
}

/**************************************************************************//**
 *
 * @brief  Initialize the USB stack and save callbacks for USB commands.
 *
 * @param [in] capStop       Called the client wants to stop signal capturing
 * @param [in] capConfigure  Called when a \a CMD_CAP_CFG command is received
 * @param [in] capRun        Called when a \a CMD_CAP_RUN command is received
 * @param [in] genStop       Called the client wants to stop signal generation
 * @param [in] genConfigure  Called when a \a CMD_GEN_CFG command is received
 * @param [in] genRun        Called when a \a CMD_GEN_RUN command is received
 *
 *****************************************************************************/
void usb_handler_InitUSB(cmdFunc capStop, cmdFuncParam capConfigure, cmdFunc capRun,
                         cmdFunc genStop, cmdFuncParam genConfigure, cmdFunc genRun)
{
  SetupHardware();

  callbacks.capStop      = capStop;
  callbacks.capConfigure = capConfigure;
  callbacks.capRun       = capRun;

  callbacks.genStop      = genStop;
  callbacks.genConfigure = genConfigure;
  callbacks.genRun       = genRun;
}

/**************************************************************************//**
 *
 * @brief  Indicates that signal samples should be sent to the client
 *
 * The samples are not sent immediately, the information is saved and then
 * a flag is set to allow the \ref usb_handler_Run function to send it when
 * there is time.
 *
 * @param [in] cap      Information about the captured samples
 *
 *****************************************************************************/
void usb_handler_SendSamples(const captured_samples_t* const cap)
{
  if (haveSamplesToSend)
  {
    log_i("Error. Have not sent last batch of samples yet\r\n");
  }
  else
  {
    samples.status = CMD_STATUS_OK;
    memcpy(&samples.cap, cap, sizeof(captured_samples_t));
    haveSamplesToSend = TRUE;
  }
}

/**************************************************************************//**
 *
 * @brief  Indicates that calibration result should be sent to the client
 *
 * The calibration result is sent immediately, the information is saved and then
 * a flag is set to allow the \ref usb_handler_Run function to send it when
 * there is time.
 *
 * @param [in] parameters   The calibration result
 *
 *****************************************************************************/
void usb_handler_SendCalibrationResult(const calib_result_t* const parameters)
{
  if (haveCalibrationResultToSend)
  {
    log_i("Error. Have not sent last calibration result yet\r\n");
  }
  else
  {
    calibration.status = CMD_STATUS_OK;
    memcpy(&calibration.parameters, parameters, sizeof(calib_result_t));
    haveCalibrationResultToSend = TRUE;
  }
}

/**************************************************************************//**
 *
 * @brief  Indicates that signal sampling failed and to inform the client
 *
 * The error status is not sent immediately, the information is saved and then
 * a flag is set to allow the \ref usb_handler_Run function to send it when
 * there is time.
 *
 * @param [in] error      Information about the error that occurred
 *
 *****************************************************************************/
void usb_handler_SignalFailedSampling(cmd_status_t error)
{
  if (haveSamplesToSend)
  {
    log_i("Error. Have not sent last batch of samples yet\r\n");
  }
  else
  {
    samples.status = error;
    samples.cap.sgpio_samples = NULL;
    samples.cap.vadc_samples = NULL;
    haveSamplesToSend = TRUE;
  }
}

/**************************************************************************//**
 *
 * @brief  Indicates that calibration failed and to inform the client
 *
 * The error status is not sent immediately, the information is saved and then
 * a flag is set to allow the \ref usb_handler_Run function to send it when
 * there is time.
 *
 * @param [in] error      Information about the error that occurred
 *
 *****************************************************************************/
void usb_handler_SignalFailedCalibration(cmd_status_t error)
{
  if (haveCalibrationResultToSend)
  {
    log_i("Error. Have not sent last calibration result yet\r\n");
  }
  else
  {
    calibration.status = error;
    haveCalibrationResultToSend = TRUE;
  }
}

/**************************************************************************//**
 *
 * @brief  The infinite loop driving the USB stack and command handler
 *
 * Periodically calls the USB stack to keep it running. Sends captured samples
 * and/or error status when requested by \ref usb_handler_SendSamples or
 * \ref usb_handler_SignalFailedSampling.
 *
 * During calibration it also periodically drives the calibration sequence.
 *
 * Once called, this function will never return.
 *
 *****************************************************************************/
void usb_handler_Run(void)
{
  st_Wdt_Config wdtCfg;

  log_i("Started from %s\r\n", (WWDT_GetStatus(WWDT_TIMEOUT_FLAG) == SET)? "WDT" : "EXT");

  log_i("Setting up watchdog\r\n");

  WWDT_Init();
  wdtCfg.wdtReset = ENABLE;
  wdtCfg.wdtProtect = DISABLE;
  wdtCfg.wdtTmrConst = WDT_INTERRUPT_TIMEOUT;
  wdtCfg.wdtWarningVal = WDT_WARNING_VALUE;
  wdtCfg.wdtWindowVal = WWDT_WINDOW_MAX;
  WWDT_Configure(wdtCfg);
  WWDT_Start();

  log_i("Waiting for data requests\r\n");

  LED_USB_CONNECTED_OFF();
  sei();
  for (;;)
  {
    WWDT_Feed();
    if (stopCaptureRequested)
    {
      callbacks.capStop();
      haveSamplesToSend = FALSE;
      stopCaptureRequested = FALSE;
      log_i("-------> capture stopped\r\n");
    }
    else if (stopGeneratorRequested)
    {
      callbacks.genStop();
      stopGeneratorRequested = FALSE;
      log_i("-------> generator stopped\r\n");
    }
    else if (calibrationState != CALIB_STATE_STOPPED)
    {
      // Done with a calibration step, send the result
      if (haveCalibrationResultToSend)
      {
        LabTool_SendCalibrationResult();
      }

      // A VADC sampling has finished, calculate calibration data based on it
      else if (haveSamplesToSend)
      {
        calibrate_ProcessResult(samples.status, samples.cap.vadc_samples);
        haveSamplesToSend = FALSE;
      }

      // Waiting for something to happen
      else
      {
        calibrate_Feed();
      }
    }
    else if (haveSamplesToSend)
    {
      LED_TRIG_ON();
      LED_ARM_OFF();

#if (FIND_SKIPPED_SAMPLES == OPT_ENABLED)
      LabTool_FindSkippedSamples();
#endif
#if (PRINT_ANALOG_HISTOGRAM == OPT_ENABLED)
      LabTool_CreateHistogram();
#endif
#if (PRINT_STATISTICS == OPT_ENABLED)
      LabTool_Stats();
#endif
      LabTool_SendSamples();
      LED_TRIG_OFF();
    }
    LabTool_ProcessCommand();
    USB_USBTask();
  }
}
