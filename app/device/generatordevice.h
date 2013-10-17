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
#ifndef GENERATORDEVICE_H
#define GENERATORDEVICE_H

#include <QObject>

#include "digitalsignal.h"
#include "analogsignal.h"

class GeneratorDevice : public QObject
{
    Q_OBJECT
public:
    explicit GeneratorDevice(QObject *parent = 0);
    ~GeneratorDevice();

    virtual int maxNumDigitalSignals() const {return 0;}
    virtual int maxNumAnalogSignals() const {return 0;}

    // maximum number of digital states per supported signal
    virtual int maxNumDigitalStates() const {return 32;}

    virtual int maxDigitalRate() const {return 100000000;}
    virtual int minDigitalRate() const {return 1;}

    virtual int maxAnalogRate() const {return 1000000;}
    virtual int minAnalogRate() const {return 1;}

    virtual double maxAnalogAmplitude() const {return 5;}
    virtual QList<AnalogSignal::AnalogWaveform> supportedAnalogWaveforms();

    void enableDigitalGenerator(bool enable);
    bool isDigitalGeneratorEnabled();

    void enableAnalogGenerator(bool enable);
    bool isAnalogGeneratorEnabled();

    DigitalSignal* addDigitalSignal(int id);
    void removeDigitalSignal(DigitalSignal* s);
    void removeAllDigitalSignals();
    QList<int> unusedDigitalIds();
    QList<DigitalSignal*> digitalSignals() {return mDigitalSignalList;}

    AnalogSignal* addAnalogSignal(int id);
    void removeAnalogSignal(AnalogSignal* s);
    void removeAllAnalogSignals();
    QList<int> unusedAnalogIds();
    QList<AnalogSignal*> analogSignals() {return mAnalogSignalList;}

    virtual void start(int digitalRate, bool loop) = 0;
    virtual void stop() = 0;


signals:
    void generateFinished(bool successful, QString msg);
    
public slots:

protected:
    QList<DigitalSignal*> mDigitalSignalList;
    QList<AnalogSignal*> mAnalogSignalList;

private:
    bool mDigitalEnabled;
    bool mAnalogEnabled;

};

#endif // GENERATORDEVICE_H
