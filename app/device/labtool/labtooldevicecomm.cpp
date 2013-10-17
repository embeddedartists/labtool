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
#include "labtooldevicecomm.h"


/*!
    The Vendor Identifier (VID) of the LabTool Hardware
    Used when detecting if the hardware is connected to the computer or not.
*/
#define VENDORID                0x1fc9

/*!
    The Product Identifier (PID) of the LabTool Hardware.
    Used when detecting if the hardware is connected to the computer or not.
*/
#define PRODUCTID               0x0018

/*!
    The number of the USB interface to use on the LabTool Hardware. As the hardware
    only uses one interface this value is always 0.
*/
#define INTERFACENUM             0

/*!
    Commands sent as USB Control Requests
    \private
*/
typedef enum
{
  REQ_GetPll1Speed       = 1, /*!< Request for the speed of PLL1 */
  REQ_Ping               = 2, /*!< Ping to indicate active line */
  REQ_StopCapture        = 3, /*!< Request to stop ongoing signal capture */
  REQ_StopGenerator      = 4, /*!< Request to stop ongoing signal generation */
  REQ_GetStoredCalibData = 5  /*!< Request for the ongoing calibration's data */
} control_requests_t;


/*! @brief The response sent from the LabTool Hardware as a response to the CMD_CAP_SAMPLES.
 * This is the header for the data containing the captured samples.
 *
 * This information will be saved until after the response to CMD_CAP_DATA_ONLY has been
 * received at which time it will be used to fill the \ref captureReceivedSamples signal.
 *
 * \private
 */
typedef struct
{
  uint32_t cmd;                 /*!< Start */
  uint32_t digitalBufferSize;   /*!< Number of bytes with captured digital signal data */
  uint32_t analogBufferSize;    /*!< Number of bytes with captured analog signal data */
  uint32_t triggerInfo;         /*!< Information about what caused the trigger */
  uint32_t digitalTrigSample;   /*!< Sample index when trigger occurred */
  uint32_t analogTrigSample;    /*!< Sample index when trigger occurred */
  uint32_t digitalChannelInfo;  /*!< Information about content of the digital data */
  uint32_t analogChannelInfo;   /*!< Information about content of the analog data */
} logic_samples_header;


/*!
    A callback used for the asynchronous transfers to the LabTool Hardware.
    This callback is used when the transferred command should result in only
    a status code (i.e. no data). An example is the \a CMD_CAP_RUN.

    The \a transfer parameter is checked and acts according to the result:
    - Calls \ref transferSuccess if the LabTool Hardware responded with a success code
    - Calls \ref transferSuccessErrorResponse if the LabTool Hardware responded with an error code
    - Calls \ref transferFailed if the transfer failed (e.g. was cancelled)

    This function cannot be a part of the LabToolDeviceComm class as the
    libusbx requires function pointer and that cannot (simply at least)
    be created from class instances.
*/
void LIBUSB_CALL CallbackForResponse(struct libusb_transfer* transfer)
{
    LabToolDeviceTransfer* ddt = ((LabToolDeviceTransfer*)transfer->user_data);
    if (ddt->isValidResponse()) {
        if (ddt->successful()) {
            ddt->deviceComm()->transferSuccess(ddt);
        } else {
            ddt->deviceComm()->transferSuccessErrorResponse(ddt);
        }
    } else {
        ddt->deviceComm()->transferFailed(ddt);
    }
}

/*!
    A callback used for the asynchronous transfers to the LabTool Hardware.
    This callback is only used for the \a CMD_CAP_DATA_ONLY command as the
    response to that command only contains data and no header. The header
    has been received earlier using the \a CMD_CAP_SAMPLES command.

    The \a transfer parameter is checked and acts according to the result:
    - Calls \ref transferSuccess if the transfer was completed (i.e. all data received)
    - Calls \ref transferFailed if the transfer failed (e.g. was cancelled)

    This function cannot be a part of the LabToolDeviceComm class as the
    libusbx requires function pointer and that cannot (simply at least)
    be created from class instances.
*/
void LIBUSB_CALL CallbackForData(struct libusb_transfer* transfer)
{
    LabToolDeviceTransfer* ddt = ((LabToolDeviceTransfer*)transfer->user_data);
    if ((transfer->status == LIBUSB_TRANSFER_COMPLETED) && ddt->validSequenceNumber()) {
        ddt->deviceComm()->transferSuccess(ddt);
    } else {
        ddt->deviceComm()->transferFailed(ddt);
    }
}

/*!
    A callback used for the asynchronous transfers to the LabTool Hardware.
    This callback is used when the transferred command should result in another
    action. An example is the \a CMD_CAP_CONFIGURE which (if successful) should
    result in a transfer of the configuration data.

    The \a transfer parameter is checked and acts according to the result:
    - Submitts a transfer of the command's payload if it has any
    - Submitts a transfer for the commands response if it has no payload
    - Calls \ref transferFailed if the transfer failed (e.g. was cancelled)

    This function cannot be a part of the LabToolDeviceComm class as the
    libusbx requires function pointer and that cannot (simply at least)
    be created from class instances.
*/
void LIBUSB_CALL CallbackForSend(struct libusb_transfer* transfer)
{
    LabToolDeviceTransfer* ddt = ((LabToolDeviceTransfer*)transfer->user_data);
    if (transfer->status == LIBUSB_TRANSFER_COMPLETED && ddt->validSequenceNumber()) {
        if (ddt->hasPayload()) {
            ddt->setupForSendingPayload(CallbackForSend, 2000);
        } else {
            ddt->setupForResponse(ddt->deviceComm()->inEndpoint(), CallbackForResponse, 2000);
        }
        int ret = libusb_submit_transfer(ddt->transfer());
        if (ret != LIBUSB_SUCCESS) {
            ddt->deviceComm()->transferFailed(ddt, ret);
        }
    } else {
        ddt->deviceComm()->transferFailed(ddt);
    }
}


/*!
    \class LabToolDeviceComm
    \brief Encapsulates the communication with the LabTool Hardware

    \ingroup Device

    The LabToolDeviceComm class provides an interface for communication
    with the LabTool Hardware.

    The communication with LabTool Hardware is based on USB and uses
    the libusbx library (see http://libusbx.sourceforge.net/).

    This application is the USB host and the LabTool Hardware is the
    device. All communication is initiated from the host.

    The following commands are used:

    Command           | Type            | Description
    :---------------: | :-------------: | -----------
    CMD_GEN_CONFIGURE | Async Transfer  | Configuration of Generator
    CMD_GEN_RUN       | Async Transfer  | Start signal generation
    CMD_CAP_CONFIGURE | Async Transfer  | Configuration of Capture
    CMD_CAP_RUN       | Async Transfer  | Start signal capturing
    CMD_CAP_SAMPLES   | Async Transfer  | Request for sample header
    CMD_CAP_DATA_ONLY | Async Transfer  | Request for samples
    REQ_GetPll1Speed  | Control Request | Example of Control Request
    REQ_Ping          | Control Request | See if the hardware is alive
    REQ_StopCapture   | Control Request | Abort signal generation
    REQ_StopGenerator | Control Request | Stop signal generation

    The Async Transfer type is as the name suggests an asynchronous request
    meaning that it can be aborted. The reason for using the asynchronous
    request is that it may take a long time to complete and blocking the
    application during that time is not possible. The best example is the
    CMD_CAP_SAMPLES which is sent directly after starting the signal capture.
    It could take serveral minutes for the signal to trigger and during that
    time the UI would appear to be frozen.

    The Control Request type is for very short requests and runs in parallel
    with the Async transfer. The Control Request is used to stop the ongoing
    activity on the LabTool Hardware as that only requires setting a flag.
*/

/*!
    Constructs a communication instance with the given \a parent.
*/
LabToolDeviceComm::LabToolDeviceComm(QObject *parent) :
    QObject(parent)
{
    this->mContext = NULL;
    this->mDeviceHandle = NULL;
    this->mRunningTransfer = NULL;
    this->mConnected = false;
    this->mActiveCalibrationData = NULL;
}

/*!
    Disconnects from the LabTool Hardware and closes down
    the USB library.
*/
LabToolDeviceComm::~LabToolDeviceComm()
{
    disconnectFromDevice();
    if (this->mContext != NULL)
    {
        libusb_exit(this->mContext);
        this->mContext = NULL;
    }
}

/*!
    Attempts to connect to a LabTool Hardware through the libusbx
    library. The \a quiet parameter controls how much is printed in
    the log.
    Returns true if the connection was made or if alreay connected.
*/
bool LabToolDeviceComm::connectToDevice(bool quiet)
{
    if (mConnected)
    {
        return true;
    }

    if (!quiet)
    {
        const struct libusb_version* version = libusb_get_version();
        qDebug("Using libusbx v%d.%d.%d.%d", version->major, version->minor, version->micro, version->nano);
        qDebug("Initializing library...");
    }

    int r = libusb_init(&this->mContext);
    if (r != LIBUSB_SUCCESS)
    {
        qDebug("Failed to initialize libusb, got error %s", libusb_error_name(r));
        return false;
    }

    this->mDeviceHandle = libusb_open_device_with_vid_pid(NULL, VENDORID, PRODUCTID);
    if (this->mDeviceHandle == NULL) {
        if (!quiet)
        {
            qDebug("Failed to open device %04X:%04X", VENDORID, PRODUCTID);
        }
        return false;
    }

    int ret = libusb_claim_interface(this->mDeviceHandle, INTERFACENUM);
    if (ret != LIBUSB_SUCCESS) {
        if (!quiet)
        {
            qDebug("Failed to claim device %04X:%04X, got error %s", VENDORID, PRODUCTID, libusb_error_name(ret));
        }
        libusb_close(this->mDeviceHandle);
        this->mDeviceHandle = NULL;
        return false;
    }

    qDebug("Opened device %04X:%04X", VENDORID, PRODUCTID);
    mConnected = true;

    probe();

    return true;
}

/*!
    Disconnects from the LabTool Hardware by closing the USB
    connection. The libusbx remains initialized.
*/
void LabToolDeviceComm::disconnectFromDevice()
{
    if (!mConnected)
    {
        return;
    }
    mConnected = false;
    if (mDeviceHandle != NULL)
    {
        /* make sure other programs can still access this device */
        /* release the interface and close the device */
        //qDebug("Releasing interface %d...", INTERFACENUM);
        libusb_release_interface(mDeviceHandle, INTERFACENUM);
        qDebug("Closing device...");
        libusb_close(mDeviceHandle);
        mDeviceHandle = NULL;
    }
    this->mRunningTransfer = NULL;
    if (this->mActiveCalibrationData != NULL) {
        delete this->mActiveCalibrationData;
        this->mActiveCalibrationData = NULL;
    }
}

/*!
    Sends a request to the LabTool Hardware to prepare it for the calibration process.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrateInit()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_INIT)" ];
        usb -> comm [ label="3. CallbackForResponse()" ];
        comm -> dev [ label="4. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrateInit()
{
    if (!mConnected)
    {
        emit calibrationFailed("No hardware connected");
        return;
    }

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_INIT, mEndpointOut, mDeviceHandle, CallbackForResponse, 2000);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}

/*!
    Sends a request to the LabTool Hardware to set the output level to \a level
    on both analog outputs.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrateAnalogOut()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_ANALOG_OUT)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrateAnalogOut(quint32 level)
{
    if (!mConnected)
    {
        emit calibrationFailed("No hardware connected");
        return;
    }

    quint32 data[1] = {
        level,
    };

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000,
                         sizeof(data),
                         (unsigned char*)data);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}

/*!
    Sends a request to the LabTool Hardware to measure the input levels for both analog
    channels. The \a a0 and \a a1 arrays contains the user's measured values for each
    of the \a levels during calibration of the Analog Outputs.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrateAnalogIn()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_ANALOG_IN)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrateAnalogIn(double a0[], double a1[], int levels[])
{
    if (!mConnected)
    {
        emit calibrationFailed("No hardware connected");
        return;
    }

    int data[9] = {
        levels[0], levels[1], levels[2],
        (int)(1000 * a0[0]), (int)(1000 * a0[1]), (int)(1000 * a0[2]),
        (int)(1000 * a1[0]), (int)(1000 * a1[1]), (int)(1000 * a1[2]),
    };

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_ANALOG_IN,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000,
                         sizeof(data),
                         (unsigned char*)data);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}

/*!
    Sends a request to the LabTool Hardware to store the gathered calibration
    \a data in persistent memory and then end the calibration process.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrationSaveData()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_STORE)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrationSaveData(LabToolCalibrationData *data)
{
    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_STORE,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000,
                         LabToolCalibrationData::rawDataByteSize(),
                         data->rawCalibrationData());

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}

/*!
    Sends a request to the LabTool Hardware to erase the persistently stored
    calibration data (if any) and then end the calibration process.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrationRestoreDefaults()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_ERASE)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrationRestoreDefaults()
{
    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_ERASE,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}

/*!
    Sends a request to the LabTool Hardware to end the calibration process without
    modifying the persistently stored calibration data.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. calibrationEnd()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAL_END)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
void LabToolDeviceComm::calibrationEnd()
{
    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAL_END,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }
}


/*!
    Sends a request to read the calibration data from the LabTool Hardware's persistant
    storage.

    The request is a Control Transfer and is synchronous.

    If the data has already been loaded and the \a forceReload flag is not set, then
    the local copy is returned instead (without any communication with the hardware).
*/
LabToolCalibrationData *LabToolDeviceComm::storedCalibrationData(bool forceReload)
{
    if (forceReload || mActiveCalibrationData == NULL)
    {
        // Load calibration information
        int size = LabToolCalibrationData::rawDataByteSize();
        unsigned char buff[size];
        int r = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
                REQ_GetStoredCalibData, 0, INTERFACENUM, buff, size, 1000);
        if (r == size) {
            if (this->mActiveCalibrationData != NULL) {
                delete this->mActiveCalibrationData;
            }
            this->mActiveCalibrationData = new LabToolCalibrationData(buff);
            this->mActiveCalibrationData->printCalibrationInfo();
        } else {
            qDebug("[Probe] Failed to get calibration data, error %s (%d)", libusb_error_name(r), r);
        }
    }
    return mActiveCalibrationData;
}

/*!
    \fn libusb_context* LabToolDeviceComm::usbContext()

    Returns the current libusbx context.
*/

/*!
    \fn quint8 LabToolDeviceComm::inEndpoint()

    Returns the IN endpoint of the USB connection.
*/

/*!
    \fn quint8 LabToolDeviceComm::outEndpoint()

    Returns the OUT endpoint of the USB connection.
*/

/*!
    \fn void LabToolDeviceComm::connectionStatus(bool connected)

    Sent to notify that a connection has been established (\a connected is true)
    of lost (\a connected is false).
*/

/*!
    \fn void LabToolDeviceComm::captureStopped()

    Sent to notify that the signal capturing functionality has been successfully stopped.
*/

/*!
    \fn void LabToolDeviceComm::captureConfigurationDone()

    Sent to notify that the signal capturing functionality was successfully configured.
*/

/*!
    \fn void LabToolDeviceComm::captureReceivedSamples(LabToolDeviceTransfer* transfer, unsigned int size, unsigned int trigger, unsigned int digitalTrigSample, unsigned int analogTrigSample, unsigned int activeDigital, unsigned int activeAnalog)

    Sent to notify that captured signal data has been received.
    The \a transfer parameter holds the data, \a size is the size of the data (in bytes),
    \a trigger is information about what caused the trigger, \a digitalTrigSample is the
    digital sample at the time of the trigger, \a analogTrigSample is the analog sample
    at the time of the trigger, \a activeDigital is
    information about what the digital signal data contains, \a activeAnalog is
    information about what the analog signal data contains.
*/

/*!
    \fn void LabToolDeviceComm::captureFailed(const char* msg)

    Sent to notify that the signal capturing failed. The \a msg parameter contains an
    error description.
*/

/*!
    \fn void LabToolDeviceComm::captureConfigurationFailed(const char* msg)

    Sent to notify that the signal capturing functionality could not be configured.
    The \a msg parameter contains an error description.
*/

/*!
    \fn void LabToolDeviceComm::generatorStopped()

    Sent to notify that the signal generating functionality has been successfully stopped.
*/

/*!
    \fn void LabToolDeviceComm::generatorConfigurationDone()

    Sent to notify that the signal generating functionality was successfully configured.
*/

/*!
    \fn void LabToolDeviceComm::generatorConfigurationFailed(const char* msg)

    Sent to notify that the signal generating functionality could not be configured.
    The \a msg parameter contains an error description.
*/

/*!
    \fn void LabToolDeviceComm::generatorRunning()

    Sent to notify that the signal generating has been successfully started.
*/

/*!
    \fn void LabToolDeviceComm::generatorRunFailed(const char* msg)

    Sent to notify that the signal generating functionality could not be started.
    The \a msg parameter contains an error description.
*/

/*!
    \fn void LabToolDeviceComm::calibrationFailed(const char* msg)

    Sent to notify that the calibration operation could not be completed.
    The \a msg parameter contains an error description.
*/

/*!
    \fn void LabToolDeviceComm::calibrationSuccess(LabToolCalibrationData* data)

    Sent to notify that the calibration operation was been successfully completed.
    Depending on the type of the operation the \a data parameter could be NULL
    or contain the updated calibration data.
*/


/*!
    Retrieves various pieces of information from the connected LabTool Hardware
    and writes it to the log.

    This purpose of this function is to show some ways of getting device information.
    It can easily be extended to gather more informaion in the future. Perhaps
    LabTool Hardware version, supported sample rates, supported optional features...
*/
void LabToolDeviceComm::probe()
{
    static bool alreadyProbed = false; // prevents printing everyting everytime
    libusb_device *dev;
    struct libusb_config_descriptor *conf_desc;
    const struct libusb_endpoint_descriptor *endpoint;
    const struct libusb_interface_descriptor *altsetting;
    int i, j, k, r;
    int nb_ifaces;
    uint8_t string_index[3];         // indexes of the string descriptors
    mEndpointIn = mEndpointOut = 0;  // default IN and OUT endpoints

    if (!mConnected) {
        return;
    }

    dev = libusb_get_device(this->mDeviceHandle);
    if (!alreadyProbed) {
        uint8_t bus, port_path[8];
        struct libusb_device_descriptor dev_desc;
        const char* speed_name[5] = { "Unknown", "1.5 Mbit/s (USB LowSpeed)", "12 Mbit/s (USB FullSpeed)",
            "480 Mbit/s (USB HighSpeed)", "5000 Mbit/s (USB SuperSpeed)"};

        bus = libusb_get_bus_number(dev);
        r = libusb_get_port_path(NULL, dev, port_path, sizeof(port_path));
        if (r > 0) {
            qDebug("[Probe] bus: %d, port path from HCD: %d", bus, port_path[0]);
            for (i=1; i<r; i++) {
                qDebug("->%d", port_path[i]);
            }
        }
        r = libusb_get_device_speed(dev);
        if ((r<0) || (r>4)) r=0;
        qDebug("[Probe] speed: %s", speed_name[r]);

        qDebug("\n[Probe] Reading device descriptor:");
        r = libusb_get_device_descriptor(dev, &dev_desc);
        if (r != LIBUSB_SUCCESS) {
            qCritical("Failed to get device descriptor, got error %s", libusb_error_name(r));
            return;
        }
        qDebug("[Probe]             length: %d", dev_desc.bLength);
        qDebug("[Probe]       device class: %d", dev_desc.bDeviceClass);
        qDebug("[Probe]                S/N: %d", dev_desc.iSerialNumber);
        qDebug("[Probe]            VID:PID: %04X:%04X", dev_desc.idVendor, dev_desc.idProduct);
        qDebug("[Probe]          bcdDevice: %04X", dev_desc.bcdDevice);
        qDebug("[Probe]    iMan:iProd:iSer: %d:%d:%d", dev_desc.iManufacturer, dev_desc.iProduct, dev_desc.iSerialNumber);
        qDebug("[Probe]           nb confs: %d", dev_desc.bNumConfigurations);

        // Copy the string descriptors for easier parsing
        string_index[0] = dev_desc.iManufacturer;
        string_index[1] = dev_desc.iProduct;
        string_index[2] = dev_desc.iSerialNumber;

        qDebug("\n[Probe] Reading configuration descriptors:");
    }

    r = libusb_get_config_descriptor(dev, 0, &conf_desc);
    if (r != LIBUSB_SUCCESS) {
        qCritical("[Probe] Failed to get device descriptor, got error %s", libusb_error_name(r));
        return;
    }
    nb_ifaces = conf_desc->bNumInterfaces;
    if (!alreadyProbed) {
        qDebug("[Probe]              nb interfaces: %d", nb_ifaces);
    }
    for (i=0; i<nb_ifaces; i++) {
        if (!alreadyProbed) {
            qDebug("[Probe]               interface[%d]: id = %d", i,
                conf_desc->interface[i].altsetting[0].bInterfaceNumber);
        }
        for (j=0; j<conf_desc->interface[i].num_altsetting; j++) {
            altsetting = &conf_desc->interface[i].altsetting[j];
            if (!alreadyProbed) {
                qDebug("[Probe] interface[%d].altsetting[%d]: num endpoints = %d",
                    i, j, altsetting->bNumEndpoints);
                qDebug("[Probe]    Class.SubClass.Protocol: %02X.%02X.%02X",
                    altsetting->bInterfaceClass,
                    altsetting->bInterfaceSubClass,
                    altsetting->bInterfaceProtocol);
            }
            for (k=0; k<altsetting->bNumEndpoints; k++) {
                endpoint = &altsetting->endpoint[k];
                if (!alreadyProbed) {
                    qDebug("[Probe]        endpoint[%d].address: %02X", k, endpoint->bEndpointAddress);
                }

                // Use the first interrupt or bulk IN/OUT endpoints as default for testing
                if ((endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) & (LIBUSB_TRANSFER_TYPE_BULK | LIBUSB_TRANSFER_TYPE_INTERRUPT)) {
                    if (endpoint->bEndpointAddress & LIBUSB_ENDPOINT_IN) {
                        if (!mEndpointIn) {
                            mEndpointIn = endpoint->bEndpointAddress;
                        }
                    } else {
                        if (!mEndpointOut) {
                            mEndpointOut = endpoint->bEndpointAddress;
                        }
                    }
                }
                if (!alreadyProbed) {
                    qDebug("[Probe]            max packet size: %04X", endpoint->wMaxPacketSize);
                    qDebug("[Probe]           polling interval: %02X", endpoint->bInterval);
                }
            }
        }
    }
    libusb_free_config_descriptor(conf_desc);

    if (!alreadyProbed) {
        char string[128];
        qDebug("\n[Probe] Reading string descriptors:");
        for (i=0; i<3; i++) {
            if (string_index[i] == 0) {
                continue;
            }
            if (libusb_get_string_descriptor_ascii(this->mDeviceHandle, string_index[i], (unsigned char*)string, 128) >= 0) {
                qDebug("[Probe]    String (0x%02X): \"%s\"", string_index[i], string);
            }
        }
        // Read the OS String Descriptor
        if (libusb_get_string_descriptor_ascii(this->mDeviceHandle, 0xEE, (unsigned char*)string, 128) >= 0) {
            qDebug("[Probe]    String (0x%02X): \"%s\"", 0xEE, string);
        }


        // Get some info from target. This is just an example
        quint32 speed = 0;
        r = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
                REQ_GetPll1Speed, 0, INTERFACENUM, (unsigned char*)&speed, sizeof(speed), 100);
        if (r == sizeof(speed)) {
            qDebug("[Probe] MCU PLL is running at %u MHz", speed/1000000);
        } else {
            qDebug("[Probe] Failed to get speed, error %s (%d)", libusb_error_name(r), r);
        }

        alreadyProbed = true;
    }

    // Load calibration information
    int size = LabToolCalibrationData::rawDataByteSize();
    unsigned char buff[size];
    r = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_IN|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
            REQ_GetStoredCalibData, 0, INTERFACENUM, buff, size, 1000);
    if (r == size) {
        if (this->mActiveCalibrationData != NULL) {
            delete this->mActiveCalibrationData;
        }
        this->mActiveCalibrationData = new LabToolCalibrationData(buff);
        this->mActiveCalibrationData->printCalibrationInfo();
    } else {
        qDebug("[Probe] Failed to get calibration data, error %s (%d)", libusb_error_name(r), r);
    }
}

/*!
    Sends a request to stop/abort the ongoing signal capture to the LabTool Hardware.

    The request is a Control Transfer and is synchronous. Any ongoing USB transfers
    (typically the CMD_CAP_SAMPLES if a capture is ongoing) are cancelled.

    A \ref captureStopped signal will be sent to indicate that the capture has stopped.
*/
int LabToolDeviceComm::stopCapture()
{
    LabToolDeviceTransfer::invalidateOldTransfers();

    if (!mConnected)
    {
        mRunningTransfer = NULL;
        return -1;
    }

    // Synchronous request to make sure HW will not send more data
    int ret = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
            REQ_StopCapture, 0, INTERFACENUM, NULL, 0, 1000);
//    if (ret != LIBUSB_SUCCESS) {
//        return -1;//emit connectionStatus(false);
//    }

    if (mRunningTransfer != NULL)
    {
        if (libusb_cancel_transfer(mRunningTransfer->transfer()) != LIBUSB_SUCCESS)
        {
            // a successful transfer cancellation will always get a callback which will delete it
            mRunningTransfer = NULL;
        }
    }


//    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
//    ddt->SetupForCommand(LabToolDeviceTransfer::CMD_STOP, m_EndpointOut, m_DeviceHandle, CallbackForSend, 2000);

//    int ret = libusb_submit_transfer(ddt->transfer());
//    if (ret != LIBUSB_SUCCESS) {
//        TransferFailed(ddt, ret);
//    }

    emit captureStopped();

    return ret;
}

/*!
    Called when a transfer has successfully completed and the status code from the
    LabTool Hardware indicates success.

    Completed Command  | Description
    :----------------: | ------------
    CMD_GEN_CONFIGURE  | Done, success reported with generatorConfigurationDone signal
    CMD_GEN_RUN        | Done, success reported with generatorRunning signal
    CMD_CAP_CONFIGURE  | Done, success reported with captureConfigurationDone signal
    CMD_CAP_RUN        | Now running, send CMD_CAP_SAMPLES to wait for captured data header
    CMD_CAP_SAMPLES    | Got header, send CMD_CAP_DATA_ONLY to get for captured data
    CMD_CAP_DATA_ONLY  | Done, success reported with captureReceivedSamples signal
    CMD_CAL_INIT       | Done, success reported with calibrationSuccess signal
    CMD_CAL_ANALOG_OUT | Done, success reported with calibrationSuccess signal
    CMD_CAL_ANALOG_IN  | Calibration running, send CMD_CAL_RESULT to get result
    CMD_CAL_RESULT     | Done, success reported with calibrationSuccess signal
    CMD_CAL_STORE      | Done, success reported with calibrationSuccess signal
    CMD_CAL_ERASE      | Done, success reported with calibrationSuccess signal
    CMD_CAL_END        | Done, success reported with calibrationSuccess signal
*/
void LabToolDeviceComm::transferSuccess(LabToolDeviceTransfer *transfer)
{
    // This is to keep information while retrieving the samples
    static logic_samples_header sampleHeader;

    int ret;

//    qDebug("%s: Success", transfer->CommandString());
    switch (transfer->command()) {
    case LabToolDeviceTransfer::CMD_GEN_CONFIGURE:
        emit generatorConfigurationDone();
        break;

    case LabToolDeviceTransfer::CMD_GEN_RUN:
        emit generatorRunning();
        break;

    case LabToolDeviceTransfer::CMD_CAP_CONFIGURE:
        emit captureConfigurationDone();
        break;

    case LabToolDeviceTransfer::CMD_CAP_RUN:
        // target is now running, time to wait for samples
        transfer->setupForIncomingCommand(LabToolDeviceTransfer::CMD_CAP_SAMPLES, mEndpointIn, mDeviceHandle, CallbackForResponse, 0xffffffff, sizeof(logic_samples_header));
        ret = libusb_submit_transfer(transfer->transfer());
        if (ret == LIBUSB_SUCCESS) {
            // must return to avoid the deletion of this transfer
            return;
        }
        transferFailed(transfer, ret);
        break;

    case LabToolDeviceTransfer::CMD_CAP_SAMPLES:
        // target has sent the header for the samples, investigate and get actual samples
        memcpy(&sampleHeader, transfer->data(), sizeof(logic_samples_header));
//        qDebug("Got samples. Headers: %#x, %#x, %#x, %#x", sampleHeader.cmd, sampleHeader.bufferSize, sampleHeader.triggerInfo, sampleHeader.channelInfo);
        transfer->setupForIncomingData(mEndpointIn, mDeviceHandle, CallbackForData, 2000, sampleHeader.digitalBufferSize, sampleHeader.analogBufferSize);
        ret = libusb_submit_transfer(transfer->transfer());
        if (ret == LIBUSB_SUCCESS) {
            // must return to avoid the deletion of this transfer
            return;
        }
        transferFailed(transfer, ret);
        break;

    case LabToolDeviceTransfer::CMD_CAP_DATA_ONLY:
        // actual sample data
        // give sampleHeader and transfer) to LabToolDevice to forward to UI
        emit captureReceivedSamples(transfer, sampleHeader.digitalBufferSize + sampleHeader.analogBufferSize, sampleHeader.triggerInfo, sampleHeader.digitalTrigSample, sampleHeader.analogTrigSample, sampleHeader.digitalChannelInfo, sampleHeader.analogChannelInfo);
        if (mRunningTransfer == transfer)
        {
            mRunningTransfer = NULL;
        }
        // must return to avoid the deletion of this transfer
        return;

    case LabToolDeviceTransfer::CMD_CAL_INIT:
        emit calibrationSuccess(NULL);
        break;

    case LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT:
        // target is now calibrating
        emit calibrationSuccess(NULL);
        break;

    case LabToolDeviceTransfer::CMD_CAL_ANALOG_IN:
        // target is now calibrating, time to wait up to 10 seconds for the result
        transfer->setupForIncomingCommand(LabToolDeviceTransfer::CMD_CAL_RESULT, mEndpointIn, mDeviceHandle, CallbackForResponse, 10000, LabToolCalibrationData::rawDataByteSize());
        ret = libusb_submit_transfer(transfer->transfer());
        if (ret == LIBUSB_SUCCESS) {
            // must return to avoid the deletion of this transfer
            return;
        }
        transferFailed(transfer, ret);
        break;

    case LabToolDeviceTransfer::CMD_CAL_RESULT:
        // target has completed the calibration and sent the result
        //memcpy(&calibResult, transfer->data(), sizeof(calib_result));

        emit calibrationSuccess(new LabToolCalibrationData(transfer->data()));
        break;

    case LabToolDeviceTransfer::CMD_CAL_STORE:
    case LabToolDeviceTransfer::CMD_CAL_ERASE:
    case LabToolDeviceTransfer::CMD_CAL_END:
        emit calibrationSuccess(NULL);
        break;
    }

    if (mRunningTransfer == transfer)
    {
        mRunningTransfer = NULL;
    }
    delete transfer;
}

/*!
    Called when a transfer has successfully completed but the status code from the
    LabTool Hardware indicated an error.

    Completed Command  | Description
    :----------------: | ------------
    CMD_GEN_CONFIGURE  | Report failure with generatorConfigurationFailed signal
    CMD_GEN_RUN        | Report failure with generatorRunFailed signal
    CMD_CAP_CONFIGURE  | Report failure with captureConfigurationFailed signal
    CMD_CAP_RUN        | Report failure with captureFailed signal
    CMD_CAP_SAMPLES    | Report failure with captureFailed signal
    CMD_CAP_DATA_ONLY  | Report failure with captureFailed signal
    CMD_CAL_INIT       | Report failure with calibrationFailed signal
    CMD_CAL_ANALOG_OUT | Report failure with calibrationFailed signal
    CMD_CAL_ANALOG_IN  | Report failure with calibrationFailed signal
    CMD_CAL_RESULT     | Report failure with calibrationFailed signal
    CMD_CAL_STORE      | Report failure with calibrationFailed signal
    CMD_CAL_ERASE      | Report failure with calibrationFailed signal
    CMD_CAL_END        | Report failure with calibrationFailed signal
*/
void LabToolDeviceComm::transferSuccessErrorResponse(LabToolDeviceTransfer *transfer)
{
    qDebug("%s: Got error status (%s) from target", transfer->commandString(), transfer->statusErrorString());
    switch (transfer->command()) {
    case LabToolDeviceTransfer::CMD_GEN_CONFIGURE:
        emit generatorConfigurationFailed(transfer->statusErrorString());
        break;

    case LabToolDeviceTransfer::CMD_GEN_RUN:
        emit generatorRunFailed(transfer->statusErrorString());
        break;

    case LabToolDeviceTransfer::CMD_CAP_CONFIGURE:
        emit captureConfigurationFailed(transfer->statusErrorString());
        break;

    case LabToolDeviceTransfer::CMD_CAP_RUN:
    case LabToolDeviceTransfer::CMD_CAP_SAMPLES:
    case LabToolDeviceTransfer::CMD_CAP_DATA_ONLY:
        emit captureFailed(transfer->statusErrorString());
        break;

    case LabToolDeviceTransfer::CMD_CAL_INIT:
    case LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT:
    case LabToolDeviceTransfer::CMD_CAL_ANALOG_IN:
    case LabToolDeviceTransfer::CMD_CAL_RESULT:
    case LabToolDeviceTransfer::CMD_CAL_STORE:
    case LabToolDeviceTransfer::CMD_CAL_ERASE:
    case LabToolDeviceTransfer::CMD_CAL_END:
        emit captureFailed(transfer->statusErrorString());
        break;
    }

    if (mRunningTransfer == transfer)
    {
        mRunningTransfer = NULL;
    }
    delete transfer;
}

/*!
    Called when a transfer has failed with an error code from the libusbx instead of
    from the LabTool Hardware.

    The \a transfer parameter contains all information about the transfer, including
    the libusbx error code.

    The \a libusb_error parameter is optional and is passed when calling this function
    to report that a transfer could not be submitted. E.g attempting to send a CMD_GEN_RUN
    command to a disconnected LabTool Hardware.

    All transfer errors except for LIBUSB_TRANSFER_CANCELLED will cause the
    \ref connectionStatus signal to be sent, causing a reconnect to the LabTool Hardware.
*/
void LabToolDeviceComm::transferFailed(LabToolDeviceTransfer *transfer, int libusb_error)
{
    if (!transfer->validSequenceNumber()) {
        //qDebug("Discarding out-of-order transfer");
        if (mRunningTransfer == transfer)
        {
            mRunningTransfer = NULL;
        }
        delete transfer;
        return;
    }
    if (libusb_error == LIBUSB_SUCCESS) {
        // transfer error
        qDebug("%s: Got transfer error: %s", transfer->commandString(), transfer->transferErrorString());

        // Don't display an error message on cancelled transfers (which occur when pressing STOP during a capture)
        if (transfer->transfer()->status != LIBUSB_TRANSFER_CANCELLED) {

            switch (transfer->command()) {
            case LabToolDeviceTransfer::CMD_GEN_CONFIGURE:
                emit generatorConfigurationFailed(transfer->transferErrorString());
                break;

            case LabToolDeviceTransfer::CMD_GEN_RUN:
                emit generatorRunFailed(transfer->transferErrorString());
                break;

            case LabToolDeviceTransfer::CMD_CAP_CONFIGURE:
                emit captureConfigurationFailed(transfer->transferErrorString());
                break;

            case LabToolDeviceTransfer::CMD_CAP_RUN:
            case LabToolDeviceTransfer::CMD_CAP_SAMPLES:
            case LabToolDeviceTransfer::CMD_CAP_DATA_ONLY:
                emit captureFailed(transfer->transferErrorString());
                break;

            case LabToolDeviceTransfer::CMD_CAL_INIT:
            case LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT:
            case LabToolDeviceTransfer::CMD_CAL_ANALOG_IN:
            case LabToolDeviceTransfer::CMD_CAL_RESULT:
            case LabToolDeviceTransfer::CMD_CAL_STORE:
            case LabToolDeviceTransfer::CMD_CAL_ERASE:
            case LabToolDeviceTransfer::CMD_CAL_END:
                emit calibrationFailed(transfer->transferErrorString());
                break;
            }
        }
    } else {
        qDebug("libusb_submit_transfer returned %s (%d)", libusb_error_name(libusb_error), libusb_error);
        const char* errMsg = "The USB communication with the LabTool hardware timed out!\n\n" \
                "This could be because the number of signals to capture in combination with the sample rate " \
                "is too high (i.e. the hardware does not have time to process it all).\n\n" \
                "Continuous attempts will be made to reestablish the connection. If the " \
                "status hasn't changed in ca 10 seconds, unplug the USB cable " \
                "from the LabTool hardware and then insert it again.";

        switch (transfer->command()) {
        case LabToolDeviceTransfer::CMD_GEN_CONFIGURE:
            emit generatorConfigurationFailed(errMsg);
            break;

        case LabToolDeviceTransfer::CMD_GEN_RUN:
            emit generatorRunFailed(errMsg);
            break;

        case LabToolDeviceTransfer::CMD_CAP_CONFIGURE:
            emit captureConfigurationFailed(errMsg);
            break;

        case LabToolDeviceTransfer::CMD_CAP_RUN:
        case LabToolDeviceTransfer::CMD_CAP_SAMPLES:
        case LabToolDeviceTransfer::CMD_CAP_DATA_ONLY:
            emit captureFailed(errMsg);
            break;

        case LabToolDeviceTransfer::CMD_CAL_INIT:
        case LabToolDeviceTransfer::CMD_CAL_ANALOG_OUT:
        case LabToolDeviceTransfer::CMD_CAL_ANALOG_IN:
        case LabToolDeviceTransfer::CMD_CAL_RESULT:
        case LabToolDeviceTransfer::CMD_CAL_STORE:
        case LabToolDeviceTransfer::CMD_CAL_ERASE:
        case LabToolDeviceTransfer::CMD_CAL_END:
            emit calibrationFailed(errMsg);
            break;
        }
    }

    if (mRunningTransfer == transfer)
    {
        mRunningTransfer = NULL;
    }

    // Don't reconnect on cancelled transfers (which occur when pressing STOP during a capture)
    if (transfer->transfer()->status != LIBUSB_TRANSFER_CANCELLED) {
        emit connectionStatus(false);
    }

    delete transfer;
}

/*!
    Sends a request to the LabTool Hardware to configure the signal capturing functionality.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. configureCapture()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAP_CONFIG)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit captureConfigurationDone()" ];
    }
    \enddot
*/
int LabToolDeviceComm::configureCapture(int cfgSize, quint8 * cfgData)
{
    if (!mConnected)
    {
        return -1;
    }

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAP_CONFIGURE,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000,
                         cfgSize,
                         cfgData);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }

    return ret;
}

/*!
    Sends a request to the LabTool Hardware to start the signal capturing functionality.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. runCapture()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_CAP_RUN)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="5. CallbackForResponse()" ];
        comm -> usb [ label="6. libusb_submit_transfer(CMD_CAP_SAMPLES)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> usb [ label="8. libusb_submit_transfer(CMD_CAP_DATA_ONLY)" ];
        usb -> comm [ label="9. CallbackForData()" ];
        comm -> dev [ label="10. emit captureReceivedSamples()" ];
    }
    \enddot
*/
int LabToolDeviceComm::runCapture()
{
    if (!mConnected)
    {
        return -1;
    }

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_CAP_RUN, mEndpointOut, mDeviceHandle, CallbackForSend, 2000);
    mRunningTransfer = ddt;

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }

    return ret;
}

/*!
    Sends a request to the LabTool Hardware to stop/abort the ongoing signal generation.

    The request is a Control Transfer and is synchronous.

    A \ref generatorStopped signal will be sent to indicate that the generation has stopped.
*/
int LabToolDeviceComm::stopGenerator()
{
    if (!mConnected)
    {
        return -1;
    }

    // Synchronous request
    int ret = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
            REQ_StopGenerator, 0, INTERFACENUM, NULL, 0, 1000);

    emit generatorStopped();

    return ret;
}

/*!
    Sends a request to the LabTool Hardware to configure the signal generation functionality.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. configureGenerator()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_GEN_CONFIG)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(configuration data)" ];
        usb -> comm [ label="5. CallbackForSend()" ];
        comm -> usb [ label="6. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="7. CallbackForResponse()" ];
        comm -> dev [ label="8. emit generatorConfigurationDone()" ];
    }
    \enddot
*/
int LabToolDeviceComm::configureGenerator(int cfgSize, quint8* cfgData)
{
    if (!mConnected)
    {
        return -1;
    }

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_GEN_CONFIGURE,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000,
                         cfgSize,
                         cfgData);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }

    return ret;
}

/*!
    Sends a request to the LabTool Hardware to start the signal generation functionality.

    The request is an asynchronous transfer. See \ref LabToolDeviceTransfer for the
    actual format of the command.

    This function will trigger this sequence of events:

    \dot
    digraph example {
        rankdir=LR
        node [shape=box, fontname=Helvetica, fontsize=10];
        edge [arrowhead="open", style="solid", fontname=Helvetica, fontsize=10];
        dev [ label="LabToolDevice" ];
        comm [ label="LabToolDeviceComm" ];
        usb [ label="libUSBx" ];
        dev -> comm [ label="1. runGenerator()" ];
        comm -> usb [ label="2. libusb_submit_transfer(CMD_GEN_RUN)" ];
        usb -> comm [ label="3. CallbackForSend()" ];
        comm -> usb [ label="4. libusb_submit_transfer(get response)" ];
        usb -> comm [ label="5. CallbackForResponse()" ];
        comm -> dev [ label="6. emit generatorRunning()" ];
    }
    \enddot
*/
int LabToolDeviceComm::runGenerator()
{
    if (!mConnected)
    {
        return -1;
    }

    LabToolDeviceTransfer* ddt = new LabToolDeviceTransfer(this);
    ddt->setupForCommand(LabToolDeviceTransfer::CMD_GEN_RUN,
                         mEndpointOut,
                         mDeviceHandle,
                         CallbackForSend,
                         2000);

    int ret = libusb_submit_transfer(ddt->transfer());
    if (ret != LIBUSB_SUCCESS) {
        transferFailed(ddt, ret);
    }

    return ret;
}

/*!
    Sends a request to the LabTool Hardware to check if it is still running.

    The request is a Control Transfer and is synchronous.

    If the request could not be completed a \ref connectionStatus signal will be sent
    to indicate that the communication has been lost.
*/
int LabToolDeviceComm::ping()
{
    if (!mConnected)
    {
        return -1;
    }

    int ret = libusb_control_transfer(this->mDeviceHandle, LIBUSB_ENDPOINT_OUT|LIBUSB_REQUEST_TYPE_VENDOR|LIBUSB_RECIPIENT_INTERFACE,
            REQ_Ping, 0, INTERFACENUM, NULL, 0, 100);
    if (ret != LIBUSB_SUCCESS) {
        emit connectionStatus(false);
    }

    return ret;
}

