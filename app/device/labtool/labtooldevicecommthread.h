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
#ifndef LABTOOLDEVICECOMMTHREAD_H
#define LABTOOLDEVICECOMMTHREAD_H

#include <QProcess>
#include <QThread>
#include "labtooldevicecomm.h"
#include "libusbx/include/libusbx-1.0/libusb.h"

class LabToolDeviceComm;

class LabToolDeviceCommThread : public QThread
{
    Q_OBJECT
public:
    explicit LabToolDeviceCommThread(QObject *parent = 0);

    void run();
    void stop();
    void reconnectToTarget();

signals:
    void connectionChanged(LabToolDeviceComm* newComm);

private:
    void prepareDfuImage();
    void runDFU();
    bool connectToDevice();

    libusb_context*     mContext;
    bool                mRun;
    bool                mReconnect;
    bool                mConnected;
    bool                mFirstConnectAttempt;
    QString             mPreparedImage;
    LabToolDeviceComm* mDeviceComm;
};

#endif // LABTOOLDEVICECOMMTHREAD_H
