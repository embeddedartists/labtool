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
#ifndef SIMULATORGENERATORDEVICE_H
#define SIMULATORGENERATORDEVICE_H

#include <QObject>
#include "device/generatordevice.h"

class SimulatorGeneratorDevice : public GeneratorDevice
{
    Q_OBJECT
public:
    explicit SimulatorGeneratorDevice(QObject *parent = 0);

    int maxNumDigitalSignals() const {return 8;}
    int maxNumAnalogSignals() const {return 2;}

    int maxNumDigitalStates() const {return 512;}

    void start(int digitalRate, bool loop);
    void stop();
    
signals:
    
public slots:
    
};

#endif // SIMULATORGENERATORDEVICE_H
