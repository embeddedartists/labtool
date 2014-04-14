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
#include "labtooldevice.h"

/*!
    \class LabToolDevice
    \brief A device representing the LabTool Hardware

    \ingroup Device

    The LabToolDevice class provides the interface to the functionality
    of the LabTool Hardware.
*/

/*!
    Constructs a device with the given \a parent.
*/
LabToolDevice::LabToolDevice(QObject *parent) :
    Device(parent)
{
    mConnected = false;
    mDeviceComm = NULL;

    mCaptureDevice = new LabToolCaptureDevice(this);
    mGeneratorDevice = new LabToolGeneratorDevice(this);

    mDeviceCommThread = new LabToolDeviceCommThread();
    QObject::connect(mDeviceCommThread, SIGNAL(connectionChanged(LabToolDeviceComm*)),
            this, SLOT(handleNewConnection(LabToolDeviceComm*)));
    mDeviceCommThread->start();
}

/*!
    Stops the \ref LabToolDeviceCommThread
*/
LabToolDevice::~LabToolDevice()
{
    mDeviceCommThread->stop();
    mDeviceCommThread->wait(4000);
    delete mDeviceCommThread;

    delete mDeviceComm;
}

/*!
    Returns true if a connection to the LabTool Hardware is established
*/
bool LabToolDevice::isAvailable() const
{
    return mConnected;
}

/*!
    A report that a new connection to the LabTool Hardware has been made.
    The \a newComm instance is sent to the \ref LabToolCaptureDevice
    and \ref LabToolGeneratorDevice after updating all signal connections.
*/
void LabToolDevice::handleNewConnection(LabToolDeviceComm *newComm)
{
    if (mDeviceComm != NULL) {
        mDeviceComm->disconnect();
        delete mDeviceComm;
    }
    mDeviceComm = newComm;

    /* Handle signals related to connection status */

    QObject::connect(mDeviceComm, SIGNAL(connectionStatus(bool)),
            this, SLOT(handleConnectedStatus(bool)));


    /* Handle signals related to capture */

    QObject::connect(mDeviceComm, SIGNAL(captureStopped()),
            mCaptureDevice, SLOT(handleStopped()));

    QObject::connect(mDeviceComm, SIGNAL(captureReceivedSamples(LabToolDeviceTransfer*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int)),
            mCaptureDevice, SLOT(handleReceivedSamples(LabToolDeviceTransfer*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, int)));

    QObject::connect(mDeviceComm, SIGNAL(captureConfigurationDone()),
            mCaptureDevice, SLOT(handleConfigurationDone()));

    QObject::connect(mDeviceComm, SIGNAL(captureFailed(const char*)),
            mCaptureDevice, SLOT(handleFailedCapture(const char*)));

    QObject::connect(mDeviceComm, SIGNAL(captureConfigurationFailed(const char*)),
            mCaptureDevice, SLOT(handleConfigurationFailure(const char*)));


    /* Handle signals related to generator */

    QObject::connect(mDeviceComm, SIGNAL(generatorStopped()),
            mGeneratorDevice, SLOT(handleStopped()));

    QObject::connect(mDeviceComm, SIGNAL(generatorConfigurationDone()),
            mGeneratorDevice, SLOT(handleConfigurationDone()));

    QObject::connect(mDeviceComm, SIGNAL(generatorConfigurationFailed(const char*)),
            mGeneratorDevice, SLOT(handleConfigurationFailure(const char*)));

    QObject::connect(mDeviceComm, SIGNAL(generatorRunFailed(const char*)),
            mGeneratorDevice, SLOT(handleRunningFailure(const char*)));

    QObject::connect(mDeviceComm, SIGNAL(generatorRunning()),
            mGeneratorDevice, SLOT(handleRunning()));


    mCaptureDevice->setDeviceComm(newComm);
    mGeneratorDevice->setDeviceComm(newComm);

    handleConnectedStatus(true);
}

/*!
    A report that the connection to the LabTool Hardware has changed. If the
    parameter \a connected is true then a new connection has been established
    and if false the connection has been lost.

    If the connection has been lost the \ref LabToolDeviceCommThread::reconnectToTarget
    is called to start looking for the LabTool Hardware again.

    The status is sent to \ref LabToolCaptureDevice and
    \ref LabToolGeneratorDevice and a \ref availableStatusChanged signal
    is sent to notify the application.
*/
void LabToolDevice::handleConnectedStatus(bool connected)
{
    if (mConnected != connected) {
        if (connected) {
            qDebug("Device connected");
            mConnected = true;
            availableStatusChanged(this);
        } else {
            qDebug("Device disconnected");
            mDeviceComm = NULL;
            mConnected = false;
            mCaptureDevice->setDeviceComm(NULL);
            mGeneratorDevice->setDeviceComm(NULL);
            mDeviceCommThread->reconnectToTarget();
            availableStatusChanged(this);
        }
    }
}
