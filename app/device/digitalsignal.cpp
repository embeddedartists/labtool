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
#include "digitalsignal.h"

#include <QDebug>
#include <QStringList>

/*!
    \class DigitalSignal
    \brief DigitalSignal is a container class for settings related to a
        digital signal.

    \ingroup Device

*/

/*!
    \enum DigitalSignal::Constants

    This enum defines constants associated with DigitalSignal

    \var DigitalSignal::Constants DigitalSignal::InvalidDigitalId
    Represents an invalid digital ID

*/

/*!
    \enum DigitalSignal::DigitalUsage

    This enum describes the possible usages for the DigitalSignal container.

    \var DigitalSignal::DigitalUsage DigitalSignal::DigitalUsageCapture
    Use when creating a digital signal for capture functionality

    \var DigitalSignal::DigitalUsage DigitalSignal::DigitalUsageGenerate
    Use when creating a digital signal for generator functionality
*/

/*!
    \enum DigitalSignal::DigitalTriggerState

    This enum describes the possible trigger states for a digital signal.

    \var DigitalSignal::DigitalTriggerState DigitalSignal::DigitalTriggerNone
    No trigger

    \var DigitalSignal::DigitalTriggerState DigitalSignal::DigitalTriggerHighLow
    Trig at high-to-low transition

    \var DigitalSignal::DigitalTriggerState DigitalSignal::DigitalTriggerLowHigh
    Trig at low-to-high transition
*/


/*!
    Constructs an empty digital signal with default usage and ID.
*/
DigitalSignal::DigitalSignal()
{
//    DigitalSignal::DigitalSignal(DigitalUsageCapture, 0);
    mUsage = DigitalUsageCapture;
    mReconfigureListener = NULL;
    mId = 0;
    mName = QString("Digital %1").arg(0);
    mNumStates = 0;
    mTriggerState = DigitalTriggerNone;
}

/*!
    Constructs a new DigitalSignal with ID set to \a id.
    The \a usage parameter indicates where this container class will be
    used; Capture or Generator. The reason to have the parameter is that
    the container class is common for both functionalities, but not all
    settings are valid in both cases.
*/
DigitalSignal::DigitalSignal(DigitalUsage usage, int id)
{
    mUsage = usage;
    mReconfigureListener = NULL;
    mId = id;
    mName = QString("Digital %1").arg(id);
    mNumStates = 0;
    mTriggerState = DigitalTriggerNone;
}

/*!
    Returns true if this digital signal and the given \a signal have the same
    contents; otherwise returns false.
*/
bool DigitalSignal::operator==(const DigitalSignal &signal) const
{
    return (mUsage == signal.mUsage &&
            mId == signal.mId &&
            mName == signal.mName &&
            mTriggerState == signal.mTriggerState &&
            mData == signal.mData &&
            mNumStates == signal.mNumStates);

}


/*!
    \fn bool DigitalSignal::operator!=(const DigitalSignal &signal)

   Returns true if this digital signal and the given \a signal have different
   contents; otherwise returns false.
*/


/*!
    Copies the content of \a other to this digital signal.
*/
DigitalSignal& DigitalSignal::operator=(const DigitalSignal &other)
{
    mUsage = other.mUsage;
    mId = other.mId;
    mName = other.mName;
    mTriggerState = other.mTriggerState;
    mData = other.mData;
    mNumStates = other.mNumStates;
    mReconfigureListener = other.mReconfigureListener;

    return *this;
}

/*!
    \fn int DigitalSignal::id() const

   Returns the signal ID.
*/

/*!
    \fn void DigitalSignal::setId(int id)

   Sets the signal ID.
*/

/*!
    \fn QString DigitalSignal::name() const

   Returns the signal name.
*/

/*!
    \fn void DigitalSignal::setName(QString name)

   Sets the signal name.
*/

/*!
    \fn QVector<bool> DigitalSignal::data() const

   Returns the signal data associated with this digital signal.

   \note
   Currently the data is only valid for the Generator functionality. Data
   that has been Captured is retrieved by CaptureDevice::digitalData().
*/


/*!
   Sets the data, that is, vector of digital states that will be associated
   with this digital signal.

   \note
   Currently the data is only valid for the Generator functionality. Data
   that has been Captured is retrieved by CaptureDevice::digitalData().
*/
void DigitalSignal::setData(QVector<bool> &data)
{

    mData = data;

    if (mData.size() < mNumStates) {
        mData.resize(mNumStates);
    }
}


/*!
    \fn DigitalTriggerState DigitalSignal::triggerState()

   Returns the trigger state set for this digital signal.
*/

/*!
    \fn void DigitalSignal::setTriggerState(DigitalTriggerState triggerState)

   Sets the trigger state for this digital signal.
*/
void DigitalSignal::setTriggerState(DigitalTriggerState triggerState)
{
    if (triggerState == mTriggerState) return;

    mTriggerState = triggerState;

    if (mReconfigureListener != NULL) {
        mReconfigureListener->reconfigure();
    }
}

/*!
    \fn int DigitalSignal::numStates() const

    Returns the number of states for this digital signal.

    \note
    This function must be used to determine how many of the states in the data
    vector that are valid. The data vector may have a larger size than what is
    reported by this function.
*/


/*!
   Sets the number of states to use for this digital signal.
*/
void DigitalSignal::setNumStates(int numStates)
{
    if (numStates <= 0) return;

    // the vector with states is never decreased, only increased
    if (mData.size() < numStates) {
        mData.resize(numStates);
    }

    mNumStates = numStates;
}

/*!
   Set the state at \a index to the value given by \a high. The parameter
   \a high is true for a logical 1 and false for a logical 0.
*/
void DigitalSignal::setState(int index, bool high)
{
    if (index < 0 || index >= mData.size()) return;

    mData[index] = high;
}

/*!
    Toggle the state at index \a index.
*/
void DigitalSignal::toogleState(int index)
{
    if (index < 0 || index >= mData.size()) return;

    mData[index] = !mData[index];
}

/*!
    Returns a string representation of this digital signal. This is typically
    used when saving settings to persistent storage.

    \sa fromSettingsString()
*/
QString DigitalSignal::toSettingsString()
{
    // -- common fields
    // type;usage;id;name;

    // -- capture fields
    // trigger

    // -- generate fields
    // states;data(base64 coded)

    QString str;
    str.append("Digital;");
    str.append(QString("%1;").arg(mUsage));
    str.append(QString("%1;").arg(mId));
    str.append(mName);str.append(";");

    if (mUsage == DigitalUsageCapture) {
        str.append(QString("%1").arg(mTriggerState));
    }
    else {
        str.append(QString("%1;").arg(mNumStates));

        // convert data to a byte array and store the base64 represenation
        QByteArray out;
        out.resize(mNumStates/8+1);
        out.fill(0);
        for (int i = 0; i < mNumStates; i++) {
            out[i/8] = ( out.at(i/8) | (mData[i]?1:0)<<(i%8)) ;
        }
        str.append(out.toBase64());
    }

    return str;
}

/*!
    Create a digital signal from a string representation. The string must have
    been created by DigitalSignal::toSettingsString().

    If the parsing of the string fails a DigitalSignal with ID
    DigitalSignal::InvalidDigitalId will be returned

    \sa toSettingsString()
*/
DigitalSignal DigitalSignal::fromSettingsString(QString &s)
{
    bool ok = false;
    DigitalSignal tmp;
    tmp.mId = DigitalSignal::InvalidDigitalId;

    do {

        // -- common fields
        // type;usage;id;name;

        // -- capture fields
        // trigger

        // -- generate fields
        // states;data(base64 coded)


        QStringList list = s.split(';');
        if (list.size() < 4) break;

        // --- type
        if (list.at(0) != "Digital") break;

        // --- usage
        int u = list.at(1).toInt(&ok);
        if (!ok) break;
        if (u < 0 || u > DigitalUsageGenerate) break;
        DigitalUsage usage = (DigitalUsage)u;

        // --- ID
        int id = list.at(2).toInt(&ok);
        if (!ok) break;

        // --- name
        QString name = list.at(3);
        if (name.isNull() || name.isEmpty()) break;


        if (usage == DigitalUsageCapture) {

            // --- trigger state
            int t = list.at(4).toInt(&ok);
            if (!ok) break;
            if (t < 0 || t >= DigitalSignal::DigitalTriggerNum) break;

            DigitalSignal::DigitalTriggerState trigger
                    = (DigitalSignal::DigitalTriggerState)t;

            tmp.mId = id;
            tmp.mUsage = usage;
            tmp.mName = name;
            tmp.mTriggerState = trigger;

        }
        else {

            if (list.size() != 6) break;

            // --- num states
            int numStates = list.at(4).toInt(&ok);
            if (!ok) break;

            // --- data
            QString dataBase64 = list.at(5);
            if (dataBase64.isNull() || dataBase64.isEmpty()) break;

            QByteArray arrData = QByteArray::fromBase64(dataBase64.toLatin1());
            if (arrData.size() * 8 < numStates) break;
            QVector<bool> data(numStates);
            for (int i = 0; i < numStates; i++) {
                data[i] = ((arrData[i/8] & (1 << (i%8))) != 0);
            }

            tmp.mId = id;
            tmp.mUsage = usage;
            tmp.mName = name;
            tmp.mNumStates = numStates;
            tmp.mData = data;
        }

    } while (false);


    return tmp;
}

/*!
    Set the reconfigure listener to \a listener. The reconfigure listener will
    be called whenever a state is changed that might need a reconfiguration of
    the device.
*/
void DigitalSignal::setReconfigureListener(ReconfigureListener* listener)
{
    /*
        Normally signals and slots would be used instead of a listener.
        The DigitalSignal class, however, is used in a way where it must
        be copied (stored in QList for example). Classes which implement
        QObject cannot be copied and we would have had to implement QObject
        in order to use signals and slots.
    */
    mReconfigureListener = listener;
}

bool digitalSignalLessThan(const DigitalSignal* s1, const DigitalSignal* s2) {
    return s1->id() < s2->id();
}


