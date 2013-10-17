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
#ifndef LABTOOLGENERATORDEVICE_H
#define LABTOOLGENERATORDEVICE_H

#include <QObject>
#include "device/generatordevice.h"
#include "labtooldevicecomm.h"

class LabToolGeneratorDevice : public GeneratorDevice
{
    Q_OBJECT
public:
    explicit LabToolGeneratorDevice(QObject *parent = 0);
    ~LabToolGeneratorDevice();

    int maxNumDigitalSignals() const {return 11;}
    int maxNumAnalogSignals() const {return 2;}

    // maximum number of digital states per supported signal
    int maxNumDigitalStates() const {return 256;}

    int maxDigitalRate() const {return 100000000;} // limit to 100MHz for now
    int minDigitalRate() const {return 20;}

    void start(int digitalRate, bool loop);
    void stop();

    void setDeviceComm(LabToolDeviceComm* comm);

signals:

public slots:

private slots:
    void handleStopped();
    void handleConfigurationDone();
    void handleConfigurationFailure(const char *msg);
    void handleRunning();
    void handleRunningFailure(const char *msg);

private:


    unsigned int configSize();
    quint8* configData(int digitalRate);
    void updateDigitalConfigData(int digitalRate);
    void updateAnalogConfigData();

    bool hasConfigChanged();
    void saveConfig();


    LabToolDeviceComm*  mDeviceComm;
    quint8* mData;
    bool mContinuousRun;
    int mDigitalRate;

    bool mConfigMustBeUpdated;
    QList<DigitalSignal> mLastUsedDigtalSignals;
    QList<AnalogSignal> mLastUsedAnalogSignals;
    int mLastUsedDigitalRate;
    bool mLastUsedContinuousRun;
};

#endif // LABTOOLGENERATORDEVICE_H
