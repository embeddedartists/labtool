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
#include "simulatorgeneratordevice.h"
#include <QDebug>


/*!
    \class SimulatorGeneratorDevice
    \brief Allows the user to test the Generator functionality of this application.

    \ingroup Device

*/

/*!
    Constructs a generator device with the given \a parent.
*/
SimulatorGeneratorDevice::SimulatorGeneratorDevice(QObject *parent) :
    GeneratorDevice(parent)
{
}

void SimulatorGeneratorDevice::start(int digitalRate, bool loop)
{

    qDebug() << "start: loop=" << loop;
    qDebug() << " digital enabled: " << isDigitalGeneratorEnabled();
    qDebug() << " digital signals: " << digitalSignals().size();
    qDebug() << " digital rate:    " << digitalRate;
    qDebug();
    qDebug() << " analog enabled:  " << isAnalogGeneratorEnabled();
    qDebug() << " analog signals:  " << analogSignals().size();
    qDebug() ;
    foreach(AnalogSignal* s, mAnalogSignalList) {
        qDebug() << "   " << s->id() << ": wave=" << s->waveform()
                 << " freq="<<s->frequency()
                 << " amp="<<s->amplitude();
    }

    if (!loop) emit generateFinished(true, "");

}

void SimulatorGeneratorDevice::stop()
{
    qDebug() << "stop";
}
