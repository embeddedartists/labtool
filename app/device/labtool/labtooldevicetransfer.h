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
#ifndef LABTOOLDEVICETRANSFER_H
#define LABTOOLDEVICETRANSFER_H

#include "QVector"
#include "labtooldevicecomm.h"

#include "libusbx/include/libusbx-1.0/libusb.h"

class LabToolDeviceComm;

class LabToolDeviceTransfer
{
public:
    LabToolDeviceTransfer(LabToolDeviceComm* comm);
    ~LabToolDeviceTransfer();

    enum Commands {
        CMD_GEN_CONFIGURE = 1,
        CMD_GEN_RUN       = 2,
        CMD_CAP_CONFIGURE = 3,
        CMD_CAP_RUN       = 4,
        CMD_CAP_SAMPLES   = 5,
        CMD_CAP_DATA_ONLY = 6,

        CMD_CAL_INIT       = 7,
        CMD_CAL_ANALOG_OUT = 8,
        CMD_CAL_ANALOG_IN  = 9,
        CMD_CAL_RESULT     = 10,
        CMD_CAL_STORE      = 11,
        CMD_CAL_ERASE      = 12,
        CMD_CAL_END        = 13
    };

    void setupForCommand(Commands cmd,
                         unsigned char endpoint,
                         libusb_device_handle* deviceHandle,
                         libusb_transfer_cb_fn callback,
                         unsigned int timeout,
                         int payloadSize=0,
                         const unsigned char* payload=NULL);

    void setupForSendingPayload(libusb_transfer_cb_fn callback,
                                unsigned int timeout);

    void setupForResponse(unsigned char endpoint,
                          libusb_transfer_cb_fn callback,
                          unsigned int timeout);

    void setupForIncomingCommand(Commands cmd,
                                 unsigned char endpoint,
                                 libusb_device_handle* deviceHandle,
                                 libusb_transfer_cb_fn callback,
                                 unsigned int timeout,
                                 int payloadSize);
    void setupForIncomingData(unsigned char endpoint,
                              libusb_device_handle* deviceHandle,
                              libusb_transfer_cb_fn callback,
                              unsigned int timeout,
                              int digitalPayloadSize,
                              int analogPayloadSize);

    bool isValidResponse();
    bool successful();


    const char* transferErrorString();
    const char* statusErrorString();
    const char* commandString();

    const quint8* data()  { return mData.constData(); }
    QVector<quint8> copyData() { return QVector<quint8>(mData); }
    int payloadSize() { return mData.size() - 4; }
    bool hasPayload() { return mHasPayload; }
    int analogDataOffset() { return mAnalogDataOffset; }
    int analogDataSize() { return mAnalogDataSize; }

    struct libusb_transfer* transfer() { return mTransfer; }
    LabToolDeviceComm* deviceComm() { return mDeviceComm; }
    Commands command() { return mCmd; }

    static void invalidateOldTransfers() { minValidSeqNr = sequenceCounter; }
    bool validSequenceNumber() { return mSequenceNumber >= minValidSeqNr; }
    int sequenceNumber() { return mSequenceNumber; }


private:
    QVector<quint8> mData;
    int mAnalogDataOffset;
    int mAnalogDataSize;
    bool mHasPayload;

    struct libusb_transfer* mTransfer;

    static int sequenceCounter;
    static int minValidSeqNr;
    int mSequenceNumber;

    LabToolDeviceComm* mDeviceComm;
    Commands mCmd;
};

#endif // LABTOOLDEVICETRANSFER_H
