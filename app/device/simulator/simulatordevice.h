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
#ifndef SIMULATORDEVICE_H
#define SIMULATORDEVICE_H

#include <QObject>

#include "device/device.h"
#include "simulatorcapturedevice.h"
#include "simulatorgeneratordevice.h"

class SimulatorDevice : public Device
{
    Q_OBJECT
public:
    explicit SimulatorDevice(QObject *parent = 0);

    QString name() const {return "Simulator";}
    bool isAvailable() const {return true;}

    bool supportsCaptureDevice() const {return (captureDevice() != NULL);}
    CaptureDevice* captureDevice() const {return mCaptureDevice;}

    bool supportsGeneratorDevice() const {return (generatorDevice() != NULL);}
    GeneratorDevice* generatorDevice() const {return mGeneratorDevice;}
    
signals:
    
public slots:

private:
    SimulatorCaptureDevice* mCaptureDevice;
    SimulatorGeneratorDevice *mGeneratorDevice;
};

#endif // SIMULATORDEVICE_H
