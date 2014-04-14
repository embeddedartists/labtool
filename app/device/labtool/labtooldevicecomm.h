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
#ifndef LABTOOLDEVICECOMM_H
#define LABTOOLDEVICECOMM_H

#include <QObject>
#include "labtooldevicecommthread.h"
#include "labtooldevicetransfer.h"
#include "labtoolcalibrationdata.h"

#include "libusbx/include/libusbx-1.0/libusb.h"

class LabToolDeviceTransfer;

class LabToolDeviceComm : public QObject
{
    Q_OBJECT
private:
    libusb_context*          mContext;
    libusb_device_handle*    mDeviceHandle;
    LabToolDeviceTransfer*  mRunningTransfer;
    bool                     mConnected;
    quint8                   mEndpointIn;
    quint8                   mEndpointOut;
    LabToolCalibrationData* mActiveCalibrationData;

public:
    explicit LabToolDeviceComm(QObject *parent = 0);
    ~LabToolDeviceComm();

    void probe();

    int stopCapture();
    int configureCapture(int cfgSize, quint8 * cfgData);
    int runCapture();

    int stopGenerator();
    int configureGenerator(int cfgSize, quint8* cfgData);
    int runGenerator();

    int ping();

    void transferSuccess(LabToolDeviceTransfer* transfer);
    void transferSuccessErrorResponse(LabToolDeviceTransfer* transfer);
    void transferFailed(LabToolDeviceTransfer* transfer, int libusb_error=LIBUSB_SUCCESS);

    bool connectToDevice(bool quiet=true);
    void disconnectFromDevice();

    libusb_context* usbContext() { return mContext; }
    quint8          inEndpoint() { return mEndpointIn; }
    quint8          outEndpoint() { return mEndpointOut; }

    void calibrateInit();
    void calibrateAnalogOut(quint32 level);
    void calibrateAnalogIn(double a0[3], double a1[3], int levels[3]);
    void calibrationSaveData(LabToolCalibrationData*);
    void calibrationRestoreDefaults();
    void calibrationEnd();
    LabToolCalibrationData* storedCalibrationData(bool forceReload=false);

signals:
    void connectionStatus(bool connected);

    void captureStopped();
    void captureConfigurationDone();
    void captureReceivedSamples(LabToolDeviceTransfer* transfer, unsigned int size, unsigned int trigger, unsigned int digitalTrigSample, unsigned int analogTrigSample, unsigned int activeDigital, unsigned int activeAnalog, int signalTrim);
    void captureFailed(const char* msg);
    void captureConfigurationFailed(const char* msg);

    void generatorStopped();
    void generatorConfigurationDone();
    void generatorConfigurationFailed(const char* msg);
    void generatorRunning();
    void generatorRunFailed(const char* msg);

    void calibrationFailed(const char* msg);
    void calibrationSuccess(LabToolCalibrationData* data);

public slots:

private slots:
};

#endif // LABTOOLDEVICECOMM_H

