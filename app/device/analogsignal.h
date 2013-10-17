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
#ifndef ANALOGSIGNAL_H
#define ANALOGSIGNAL_H

#include <QString>
#include <QMetaType>

#include "reconfigurelistener.h"

class AnalogSignal
{
public:

    enum Constants {
        InvalidAnalogId = -1
    };

    enum AnalogUsage {
        AnalogUsageCapture,
        AnalogUsageGenerate
    };

    enum AnalogTriggerState {
        AnalogTriggerNone,
        AnalogTriggerHighLow,
        AnalogTriggerLowHigh,
        AnalogTriggerNum // must be last
    };

    enum AnalogCoupling {
        CouplingDc,
        CouplingAc,
        CouplingNum // must be last
    };

    enum AnalogWaveform {
        WaveformSine,
        WaveformSquare,
        WaveformTriangle,
        WaveformNum // must be last
    };

    explicit AnalogSignal();
    explicit AnalogSignal(AnalogUsage usage, int id = 0);
    bool operator==(const AnalogSignal &other);
    bool operator!=(const AnalogSignal &other) {return !(*this == other);}
    AnalogSignal& operator=(const AnalogSignal &other);
    
    int id() const {return mId;}
    void setId(int id) {mId = id;}

    QString name() const {return mName;}
    void setName(QString name) {mName = name;}


    AnalogTriggerState triggerState() const {return mTriggerState;}
    void setTriggerState(AnalogTriggerState state);

    AnalogCoupling coupling() const {return mCoupling;}
    void setCoupling(AnalogCoupling c);

    double vPerDiv() const {return mVPerDiv;}
    void setVPerDiv(double v);

    double triggerLevel() const {return mTriggerLevel;}
    void setTriggerLevel(double l);


    AnalogWaveform waveform() const {return mWaveform;}
    void setWaveform(AnalogWaveform waveform) {mWaveform = waveform;}

    int frequency() const {return mFrequency;}
    void setFrequency (int freq) {mFrequency = freq;}

    double amplitude() const {return mAmplitude;}
    void setAmplitude(double amp) {mAmplitude = amp;}

    QString toSettingsString();
    static AnalogSignal fromSettingsString(QString& settings);

    void setReconfigureListener(ReconfigureListener* listener);


private:
    AnalogUsage mUsage;
    ReconfigureListener* mReconfigureListener;

    // ##### Common properties ########

    int mId;
    QString mName;

    // ##### Capture properties #######

    AnalogTriggerState mTriggerState;
    AnalogCoupling mCoupling;
    double mVPerDiv;
    double mTriggerLevel;

    // ##### Generator properties #####

    AnalogWaveform mWaveform;
    int mFrequency;
    double mAmplitude;

    
};

Q_DECLARE_METATYPE(AnalogSignal)
Q_DECLARE_METATYPE(AnalogSignal*)

bool analogSignalLessThan(const AnalogSignal* s1, const AnalogSignal* s2);

#endif // ANALOGSIGNAL_H
