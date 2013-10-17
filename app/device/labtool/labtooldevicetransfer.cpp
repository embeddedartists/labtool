/*
 *  Copyright 2013 Embedded Artists AB
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#include "labtooldevicetransfer.h"

/*!
     When using debugger or Valgrind it is useful to increase the timeouts of
     all USB transfers. Default value is 1.
*/
#define TIMEOUT_MULTIPLIER  (1)


/*!
    Sequence counter to detect out-of-order transfers.
*/
int LabToolDeviceTransfer::sequenceCounter = 1000;

/*!
    Ignore all transfers with sequence numbers below this value.
*/
int LabToolDeviceTransfer::minValidSeqNr = 0;

/*!
    \class LabToolDeviceTransfer
    \brief Encapsulation of a libusbx transfer

    \ingroup Device

    When creating asynchronous transfers for libusbx the \a libusb_fill_bulk_transfer()
    function is used to fill all important fields of the transfer. The instance of
    this class is actually passed to the \a libusb_fill_bulk_transfer function so that,
    when the response is retrieved, it is possible to see which transfer the response
    is for.
*/

/*!
    Constructs a transfer for the given \a comm.
*/
LabToolDeviceTransfer::LabToolDeviceTransfer(LabToolDeviceComm *comm)
{
    mTransfer = libusb_alloc_transfer(0);
    mDeviceComm = comm;
    mHasPayload = false;
    mAnalogDataOffset = 0;
    mAnalogDataSize = 0;
    mSequenceNumber = sequenceCounter++;
    mCmd = CMD_CAL_END;
//    qDebug("[Trace] New transfer for comm %#x, mTransfer=%#x, this=%#x", (uint32_t)comm, (uint32_t)mTransfer, (uint32_t)this);
}

/*!
    Releases the resources for this transfer
*/
LabToolDeviceTransfer::~LabToolDeviceTransfer()
{
//    qDebug("[Trace] Delete transfer for comm %#x, mTransfer=%#x, this=%#x", (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    libusb_free_transfer(mTransfer);
    mTransfer = NULL;
}

/*!
    \enum LabToolDeviceTransfer::Commands

    The commands sent to the LabTool Hardware.
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_GEN_CONFIGURE
    Sent to configure the signal generation functionality
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_GEN_RUN
    Sent to start signal generation
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAP_CONFIGURE
    Sent to configure the signal capture functionality
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAP_RUN
    Sent to start signal capture
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAP_SAMPLES
    Sent to retrieve the captured signal data
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAP_DATA_ONLY
    Internal command, never sent to the LabTool Hardware, but
    used to mark the transfer
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_INIT
    Sent to initialize the calibration sequence
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT
    Sent to calibrate the analog outputs
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_ANALOG_IN
    Sent to calibrate the analog inputs
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_RESULT
    Sent to retrieve the result of the calibration operation
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_STORE
    Sent to store the calibration data in the LabTool Hardware's persistant memory
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_ERASE
    Sent to erase the calibration data from the LabTool Hardware's persistant memory
*/
/*!
    \var LabToolDeviceTransfer::Commands LabToolDeviceTransfer::CMD_CAL_END
    Sent to end the calibration sequence
*/


/*!
    Creates a new transfer.

    The \a cmd parameter specifies type of transfer

    Command           | Endpoint | Callback            | Payload
    ----------------- | :------: | ------------------- | :-----:
    CMD_GEN_CONFIGURE |   OUT    | CallbackForSend     |   Yes
    CMD_GEN_RUN       |   OUT    | CallbackForResponse |   No
    CMD_CAP_CONFIGURE |   OUT    | CallbackForSend     |   Yes
    CMD_CAP_RUN       |   OUT    | CallbackForSend     |   No

    The \a deviceHandle parameter is needed by libusbx, \a timeout specifies in milliseconds
    when a transfer should be aborted.

    The transferred data will be 4 bytes formatted like this:

   \dot
    digraph structs {
        node [shape=record];
        start [label="0xEA | Command | {Payload Size | { LSB | MSB } }"];
    }
    \enddot

    Note that only the size of the payload is sent in this first transfer, not the actual payload.
*/
void LabToolDeviceTransfer::setupForCommand(Commands cmd, unsigned char endpoint, libusb_device_handle *deviceHandle, libusb_transfer_cb_fn callback, unsigned int timeout, int payloadSize, const unsigned char *payload)
{
//    qDebug("[Trace] Setup for command %d: comm %#x, mTransfer=%#x, this=%#x", cmd, (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    mData.resize(4 + payloadSize);

    mData[0] = (payloadSize >> 0) & 0xff;
    mData[1] = (payloadSize >> 8) & 0xff;
    mData[2] = cmd;
    mData[3] = 0xea;

    for (int i = 0; i < payloadSize; i++) {
        mData[4+i] = payload[i];
    }

    if (payloadSize > 0) {
        mHasPayload = true;
    }

    mCmd = cmd;

    libusb_fill_bulk_transfer(mTransfer,
                              deviceHandle,
                              endpoint,
                              mData.data(),
                              4,
                              callback,
                              this,
                              timeout * TIMEOUT_MULTIPLIER);
}

/*!
    Modifies this transfer so that it's payload can be sent. This is only valid for
    a transfer that has previously been setup by \ref setupForCommand.

    The \a callback parameter should always be the CallbackForSend function.

    The \a timeout parameter specifies in milliseconds when a transfer should be aborted.

    The transferred data will be:

   \dot
    digraph structs {
        node [shape=record];
        start [label="Data byte 0 | Data Byte 1 | .. | Data Byte (size-1)"];
    }
    \enddot

    Note that only the size of the payload was sent in this first transfer, this is
    the actual payload data.
*/
void LabToolDeviceTransfer::setupForSendingPayload(libusb_transfer_cb_fn callback, unsigned int timeout)
{
//    qDebug("[Trace] Setup for payload: comm %#x, mTransfer=%#x, this=%#x", (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    if (!mHasPayload) {
        qCritical("Invalid function call");
    }

    mHasPayload = false;

    // remove the header which has already been sent
    mData.remove(0, 4);

    mTransfer->length = mData.size();
    mTransfer->buffer = mData.data();
    mTransfer->callback = callback;
    mTransfer->timeout = timeout * TIMEOUT_MULTIPLIER;
}

/*!
    Modifies this transfer so that it's payload can be sent. This is only valid for
    a transfer that has previously been setup by \ref setupForCommand.

    The \a endpoint parameter should be the OUT endpoint to use.

    The \a callback parameter should always be the CallbackForResponse function.

    The \a timeout parameter specifies in milliseconds when a transfer should be aborted.

    The received data will be 4 bytes formatted like this:

   \dot
    digraph structs {
        node [shape=record];
        start [label="0xEA | Command to respond to | 0x00 | Error Code"];
    }
    \enddot

    The error code can be translated into readable form by the
    \ref statusErrorString function.
*/
void LabToolDeviceTransfer::setupForResponse(unsigned char endpoint, libusb_transfer_cb_fn callback, unsigned int timeout)
{
//    qDebug("[Trace] Setup for response: comm %#x, mTransfer=%#x, this=%#x", (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    mData.clear();
    mData.resize(4);

    mTransfer->length = mData.size();
    mTransfer->buffer = mData.data();
    mTransfer->endpoint = endpoint;
    mTransfer->callback = callback;
    mTransfer->timeout = timeout * TIMEOUT_MULTIPLIER;
}

/*!
    Creates a new transfer.

    The \a cmd parameter specifies type of transfer

    Command           | Endpoint | Callback            | Payload
    ----------------- | :------: | ------------------- | :-----:
    CMD_CAP_SAMPLES   |   IN     | CallbackForResponse |   Yes

    The \a deviceHandle parameter is needed by libusbx, \a timeout specifies in milliseconds
    when a transfer should be aborted.

    The received data will be formatted like this:

    \dot
     digraph structs {
         node [shape=record];
         message [label="START | Digital Size | Analog Size | Trigger | Active Digital Channels | Active Analog Channels"];
     }
    \enddot

    Where each part is 32 bits and \a START is divided into four bytes like this:

    \dot
     digraph structs {
         node [shape=record];
         start [label="0xEA | CMD_CAP_SAMPLES | 0x00 | Error Code"];
     }
    \enddot
*/
void LabToolDeviceTransfer::setupForIncomingCommand(Commands cmd, unsigned char endpoint, libusb_device_handle *deviceHandle, libusb_transfer_cb_fn callback, unsigned int timeout, int payloadSize)
{
//    qDebug("[Trace] Setup for incomming cmd %d: comm %#x, mTransfer=%#x, this=%#x", cmd, (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    mData.clear();
    mData.resize(payloadSize);

    mCmd = cmd;

    libusb_fill_bulk_transfer(mTransfer,
                              deviceHandle,
                              endpoint,
                              mData.data(),
                              mData.size(),
                              callback,
                              this,
                              timeout * TIMEOUT_MULTIPLIER);
}

/*!
    Modifies this transfer so that it can receive payload.

    The command will be set to CMD_CAP_DATA_ONLY.

    The \a endpoint parameter should be the IN endpoint to use.

    The \a deviceHandle parameter is needed by libusbx, \a timeout specifies in milliseconds
    when a transfer should be aborted.

    The \a callback parameter should always be the CallbackForData function.

    The \a digitalPayloadSize parameter specifies how many bytes of digital samples to receive.
    The \a analogPayloadSize parameter specifies how many bytes of analog samples to receive.

    The received data will be \a digitalPayloadSize + \a analogPayloadSize bytes formatted like this:

   \dot
    digraph structs {
        node [shape=record];
        start [label="Digital Byte 0 | Digital Byte 1 | ... | Digital Byte (digitalPayloadSize - 1) | Analog Byte 0 | Analog Byte 1 | ... | Analog Byte (analogPayloadSize - 1)"];
    }
    \enddot
*/
void LabToolDeviceTransfer::setupForIncomingData(unsigned char endpoint, libusb_device_handle *deviceHandle, libusb_transfer_cb_fn callback, unsigned int timeout, int digitalPayloadSize, int analogPayloadSize)
{
//    qDebug("[Trace] Setup for incoming data: comm %#x, mTransfer=%#x, this=%#x", (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    mData.clear();
    mData.resize(digitalPayloadSize + analogPayloadSize);
    mAnalogDataOffset = digitalPayloadSize;
    mAnalogDataSize = analogPayloadSize;

    mCmd = CMD_CAP_DATA_ONLY;

    libusb_fill_bulk_transfer(mTransfer,
                              deviceHandle,
                              endpoint,
                              mData.data(),
                              mData.size(),
                              callback,
                              this,
                              timeout * TIMEOUT_MULTIPLIER);
}

/*!
    Verifies that the first received byte is 0xEA and that the Command byte corresponds
    to the Command that this transfer is configured for.
*/
bool LabToolDeviceTransfer::isValidResponse()
{
//    qDebug("[Trace] IsValidResponse() return %s: comm %#x, mTransfer=%#x, this=%#x", (((mData[3] == 0xea) && (mData[2] == mCmd))?"TRUE":"FALSE"), (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    if (mSequenceNumber < minValidSeqNr) {
        //qDebug("isValidResponse: Found out-of-order transfer");
        return false;
    }
    return (mData[3] == 0xea) && (mData[2] == mCmd);
}

/*!
    Verifies the same as \ref isValidResponse() plus the error code must indicate success.
*/
bool LabToolDeviceTransfer::successful()
{
//    qDebug("[Trace] Successful() return %s: comm %#x, mTransfer=%#x, this=%#x", (((mData[3] == 0xea) && (mData[2] == mCmd) && (mData[0] == 0))?"TRUE":"FALSE"), (uint32_t)mDeviceComm, (uint32_t)mTransfer, (uint32_t)this);
    if (isValidResponse()) {
        if (mData[0] == 0) {
            return true;
        }
//        qDebug("%s returned status %d", CommandString(), mData[0]);
//    } else if (mTransfer->status == LIBUSB_TRANSFER_COMPLETED) {
//        qDebug("%s completed, but got invalid response {%#x, %#x, %#x, %#x}", CommandString(), mData[3], mData[2], mData[1], mData[0]);
//    } else {
//        qDebug("%s got transfer error: %s", CommandString(), TransferErrorString());
    }
    return false;
}

/*!
    Translates this transfer's status code (one from libusbx) into a printable string.
*/
const char *LabToolDeviceTransfer::transferErrorString()
{
    switch (mTransfer->status)
    {
    case LIBUSB_TRANSFER_COMPLETED: return "LIBUSB_TRANSFER_COMPLETED";
    case LIBUSB_TRANSFER_ERROR:     return "LIBUSB_TRANSFER_ERROR";

    case LIBUSB_TRANSFER_STALL:
    case LIBUSB_TRANSFER_TIMED_OUT: return "The USB communication with the LabTool hardware timed out!\n\n" \
                "This could be because the number of signals to capture in combination with the sample rate " \
                "is too high (i.e. the hardware does not have time to process it all).\n\n" \
                "Continuous attempts will be made to reestablish the connection. If the " \
                "status hasn't changed in ca 10 seconds, unplug the USB cable " \
                "from the LabTool hardware and then insert it again.";

    case LIBUSB_TRANSFER_CANCELLED: return "LIBUSB_TRANSFER_CANCELLED";
    case LIBUSB_TRANSFER_NO_DEVICE: return "LIBUSB_TRANSFER_NO_DEVICE";
    case LIBUSB_TRANSFER_OVERFLOW:  return "LIBUSB_TRANSFER_OVERFLOW";
    default:                        return "Unknown error code";
    }
}

/*!
    Translates this command's status code (received from the LabTool Hardware)
    into a printable string.
*/
const char *LabToolDeviceTransfer::statusErrorString()
{
    if (isValidResponse()) {
        switch (mData[0])
        {
        case  0: return "CMD_STATUS_OK";
        case  1: return "CMD_STATUS_ERR";

        /* Related to Signal Capture */
        case  2: return "Unsupported sample rate! The selected combination of signals and\n" \
                    "sample rate is invalid. The hard limits are:\n\n" \
                    " * 60 MHz when capturing either A0 or A1\n" \
                    " * 30 MHz when capturing both A0 and A1";
        case  3: return "CMD_STATUS_ERR_INVALID_POSTFILLPERCENT";
        case  4: return "CMD_STATUS_ERR_INVALID_VDIV";
        case  5: return "CMD_STATUS_ERR_FAILED_TO_SET_VDIV";
        case  6: return "CMD_STATUS_ERR_FAILED_TO_SET_ACDC_COUPLING";
        case  7: return "CMD_STATUS_ERR_NO_DIGITAL_SIGNALS_ENABLED";
        case  8: return "CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_LOW";
        case  9: return "CMD_STATUS_ERR_TRIGGER_LEVEL_TOO_HIGH";
        case 10: return "CMD_STATUS_ERR_NOISE_REDUCTION_LEVEL_TOO_HIGH";
        case 11: return "Cannot start capture without at least one channel enabled!\n\n"\
                    "Use the \"Add Channel\" button in the toolbar to add one or more channels.";
        case 12: return "You have hit one of the current limitations of this version of the software!\n\n" \
                    "The limitation when sampling both analog and digital signals is:\n" \
                    " * Sample rate cannot be higher than 20MHz.\n\n" \
                    "The limitations when sampling only digital signals without triggers are:\n" \
                    " * Max 50MHz sample rate when capturing D0 to D7.\n" \
                    " * Max 20MHz sample rate when capturing D0 to D10.\n\n" \
                    "The limitations when sampling only digital signals with triggers are:\n" \
                    " * Max 80MHz sample rate when capturing D0 to D3.\n" \
                    " * Max 40MHz sample rate when capturing D0 to D7.\n" \
                    " * Max 20MHz sample rate when capturing D0 to D10.";

        /* Related to Signal Generation */
        case 25: return "CMD_STATUS_ERR_NOTHING_TO_GENERATE";
        case 26: return "CMD_STATUS_ERR_GEN_INVALID_WAVEFORM";
        case 27: return "CMD_STATUS_ERR_GEN_INVALID_FREQUENCY";
        case 28: return "CMD_STATUS_ERR_GEN_INVALID_RUN_COUNTER";
        case 29: return "CMD_STATUS_ERR_GEN_INVALID_NUMBER_OF_STATES";
        case 30: return "CMD_STATUS_ERR_GEN_INVALID_AMPLITUDE";

        /* Related to I2C monitoring */
        case 40: return "CMD_STATUS_ERR_MON_I2C_PCA95555_FAILED";
        case 41: return "CMD_STATUS_ERR_MON_I2C_INVALID_RATE";
        case 42: return "CMD_STATUS_ERR_MON_I2C_NOT_CONFIGURED";

        /* Related to calibration of analog signals */
        case 50: return "CMD_STATUS_ERR_CAL_AOUT_INVALID_PARAMS";
        case 51: return "CMD_STATUS_ERR_CAL_AIN_INVALID_PARAMS";
        case 52: return "Readback of stored data returned different result.";

        /* Internal state machine errors */
        case 99: return "CMD_STATUS_ERR_NO_SUCH_STATE";
        }
    }
    return "Unknown status error code";
}

/*!
    Translates this transfer's Command into a printable string.
*/
const char *LabToolDeviceTransfer::commandString()
{
    switch (mCmd)
    {
    case CMD_GEN_CONFIGURE: return "CMD_GEN_CONFIGURE";
    case CMD_GEN_RUN:       return "CMD_GEN_RUN";
    case CMD_CAP_CONFIGURE: return "CMD_CAP_CONFIGURE";
    case CMD_CAP_RUN:       return "CMD_CAP_RUN";
    case CMD_CAP_SAMPLES:   return "CMD_CAP_SAMPLES";
    case CMD_CAP_DATA_ONLY: return "CMD_CAP_DATA_ONLY";
    default:                return "Unknown command";
    }
}

/*!
    \fn const quint8* LabToolDeviceTransfer::data()

    Returns the received data or the data to send, depending on what kind of
    transfer this is. The data is only valid as long as this object is not
    deallocated.
*/
/*!
    \fn QVector<quint8> LabToolDeviceTransfer::copyData()

    Creates a copy of the received data. The data will remain even after this
    object is deallocated and the recipient is responsible for deallocation.
*/
/*!
    \fn int LabToolDeviceTransfer::payloadSize()

    Returns the number of bytes in the payload which is the total number
    of bytes minus the header size (4 bytes).
*/
/*!
    \fn bool LabToolDeviceTransfer::hasPayload()

    Returns true if this transfer has a payload.
*/
/*!
    \fn int LabToolDeviceTransfer::analogDataOffset()

    Both analog and digital data is stored in the same data array (retrievable
    with \ref data() and \ref copyData(). The digital data is always stored at
    offset 0, but the analog data can be placed anywhere after that.
    This function returns the offset in that array to where the analog
    sample data is stored. The returned value is only valid if
    \ref analogDataSize() returns a non-zero value.
*/
/*!
    \fn int LabToolDeviceTransfer::analogDataSize()

    Returns the number of bytes of analog sample data stored in this transfer.
*/
/*!
    \fn struct libusb_transfer* LabToolDeviceTransfer::transfer()

    Returns libusbx transfer structure.
*/
/*!
    \fn LabToolDeviceComm* LabToolDeviceTransfer::deviceComm()

    Returns the \ref LabToolDeviceComm instance.
*/
/*!
    \fn Commands LabToolDeviceTransfer::command()

    Returns the command that this transfer is used for.
*/
/*!
    \fn void LabToolDeviceTransfer::invalidateOldTransfers()

    Rises the minimum required sequence number so that all (if any) current
    transfers will fail validation.
*/
/*!
    \fn bool LabToolDeviceTransfer::validSequenceNumber()

    Tests if this transfer's sequence number is still valid.
*/
/*!
    \fn int LabToolDeviceTransfer::sequenceNumber()

    Returns this transfer's sequence number.
*/
