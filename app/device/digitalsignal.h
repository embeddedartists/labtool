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
#ifndef DIGITALSIGNAL_H
#define DIGITALSIGNAL_H

#include <QMetaType>
#include <QVector>
#include <QString>

#include "reconfigurelistener.h"

class DigitalSignal
{
public:

    enum Constants {
        InvalidDigitalId = -1
    };

    enum DigitalUsage {
        DigitalUsageCapture,
        DigitalUsageGenerate
    };

    enum DigitalTriggerState {
        DigitalTriggerNone,
        DigitalTriggerHighLow,
        DigitalTriggerLowHigh,
#if 0 // disabling high level and low level as trigger levels
        DigitalTriggerHigh,
        DigitalTriggerLow,
#endif
        DigitalTriggerNum // must be last
    };

    explicit DigitalSignal();
    explicit DigitalSignal(DigitalUsage usage, int id = 0);
    bool operator==(const DigitalSignal &signal) const;
    bool operator!=(const DigitalSignal &signal) const
        {return !(*this == signal);}
    DigitalSignal& operator=(const DigitalSignal &other);

    int id() const {return mId;}
    void setId(int id) {mId = id;}

    QString name() const {return mName;}
    void setName(QString name) {mName = name;}

    QVector<bool> data() const {return mData;}
    void setData(QVector<bool> &data);

    DigitalTriggerState triggerState() {return mTriggerState;}
    void setTriggerState(DigitalTriggerState triggerState);

    // numStates() must be used to determine how many of the states
    // in the data vector that are valid. The vector may be larger than
    // numStates
    int numStates() const {return mNumStates;}
    void setNumStates(int states);

    void setState(int index, bool state);
    void toogleState(int index);

    QString toSettingsString();
    static DigitalSignal fromSettingsString(QString &settings);

    void setReconfigureListener(ReconfigureListener* listener);

private:

    DigitalUsage mUsage;
    ReconfigureListener* mReconfigureListener;

    // ##### Common properties ########

    int mId;
    QString mName;
    QVector<bool> mData;

    // ##### Capture properties #######

    DigitalTriggerState mTriggerState;

    // ##### Generator properties #####

    int mNumStates;
    
};

Q_DECLARE_METATYPE(DigitalSignal)
Q_DECLARE_METATYPE(DigitalSignal*)

bool digitalSignalLessThan(const DigitalSignal* s1, const DigitalSignal* s2);

#endif // DIGITALSIGNAL_H
