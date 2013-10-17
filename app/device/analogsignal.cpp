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
#include "analogsignal.h"

#include <QStringList>

/*!
    \class AnalogSignal
    \brief AnalogSignal is a container class for settings related to a
        analog signal.

    \ingroup Device

*/

/*!
    \enum AnalogSignal::Constants

    This enum defines constants associated with AnalogSignal

    \var AnalogSignal::Constants AnalogSignal::InvalidAnalogId
    Represents an invalid analog ID

*/

/*!
    \enum AnalogSignal::AnalogUsage

    This enum describes the possible usages for the AnalogSignal container.

    \var AnalogSignal::AnalogUsage AnalogSignal::AnalogUsageCapture
    Use when creating an analog signal for capture functionality

    \var AnalogSignal::AnalogUsage AnalogSignal::AnalogUsageGenerate
    Use when creating an analog signal for generator functionality
*/

/*!
    \enum AnalogSignal::AnalogTriggerState

    This enum describes the possible trigger states for an analog signal.

    \var AnalogSignal::AnalogTriggerState AnalogSignal::AnalogTriggerNone
    No trigger

    \var AnalogSignal::AnalogTriggerState AnalogSignal::AnalogTriggerHighLow
    Trig at high-to-low transition

    \var AnalogSignal::AnalogTriggerState AnalogSignal::AnalogTriggerLowHigh
    Trig at low-to-high transition
*/

/*!
    \enum AnalogSignal::AnalogCoupling

    This enum describes the possible coupling settings for an analog signal.

    \var AnalogSignal::AnalogCoupling AnalogSignal::CouplingDc
    Use DC coupling - both the DC and AC components of the signal are measured

    \var AnalogSignal::AnalogCoupling AnalogSignal::CouplingAc
    Use AC coupling - the DC component of the signal is filtered out.
*/

/*!
    \enum AnalogSignal::AnalogWaveform

    This enum describes the possible waveforms that can be generated for an
    analog signal.

    \var AnalogSignal::AnalogWaveform AnalogSignal::WaveformSine
    Generate a sine waveform.

    \var AnalogSignal::AnalogWaveform AnalogSignal::WaveformSquare
    Generate a square waveform.

    \var AnalogSignal::AnalogWaveform AnalogSignal::WaveformTriangle
    Generate a triangle waveform.
*/


/*!
    Constructs an empty analog signal with default usage and ID.
*/
AnalogSignal::AnalogSignal()
{
//    AnalogSignal::AnalogSignal(AnalogUsageCapture, 0);
    mUsage     = AnalogUsageCapture;
    mReconfigureListener = NULL;
    mId        = 0;
    mName      = QString("Analog %1").arg(0);

    mTriggerState = AnalogTriggerNone;
    mCoupling     = CouplingDc;
    mVPerDiv      = 2;
    mTriggerLevel = 0;

    mFrequency = 10000;
    mWaveform  = AnalogSignal::WaveformSine;
    mAmplitude = 3;
}

/*!
    Constructs a new AnalogSignal with ID set to \a id.
    The \a usage parameter indicates where this container class will be
    used; Capture or Generator. The reason to have the parameter is that
    the container class is common for both functionalities, but not all
    settings are valid in both cases.
*/
AnalogSignal::AnalogSignal(AnalogUsage usage, int id)
{
    mUsage     = usage;
    mReconfigureListener = NULL;
    mId        = id;
    mName      = QString("Analog %1").arg(id);

    mTriggerState = AnalogTriggerNone;
    mCoupling     = CouplingDc;
    mVPerDiv      = 2;
    mTriggerLevel = 0;

    mFrequency = 10000;
    mWaveform  = AnalogSignal::WaveformSine;
    mAmplitude = 3;
}

/*!
    Returns true if this analog signal and the given \a signal have the same
    contents; otherwise returns false.
*/
bool AnalogSignal::operator==(const AnalogSignal &other)
{
    return (mId == other.mId &&
            mName == other.mName &&
            mTriggerState == other.mTriggerState &&
            mCoupling == other.mCoupling &&
            mVPerDiv == other.mVPerDiv &&
            mTriggerLevel == other.mTriggerLevel &&
            mFrequency == other.mFrequency &&
            mWaveform == other.mWaveform &&
            mAmplitude == other.mAmplitude);

}


/*!
    \fn bool AnalogSignal::operator!=(const AnalogSignal &signal)

   Returns true if this analog signal and the given \a signal have different
   contents; otherwise returns false.
*/


/*!
    Copies the content of \a other to this analog signal.
*/
AnalogSignal& AnalogSignal::operator=(const AnalogSignal &other)
{
    mId = other.mId;
    mName = other.mName;
    mTriggerState = other.mTriggerState;
    mCoupling = other.mCoupling;
    mVPerDiv = other.mVPerDiv;
    mTriggerLevel = other.mTriggerLevel;
    mFrequency = other.mFrequency;
    mWaveform = other.mWaveform;
    mAmplitude = other.mAmplitude;
    mUsage = other.mUsage;
    mReconfigureListener = other.mReconfigureListener;

    return *this;
}

/*!
    \fn int AnalogSignal::id() const

   Returns the signal ID.
*/

/*!
    \fn void AnalogSignal::setId(int id)

   Sets the signal ID.
*/

/*!
    \fn QString AnalogSignal::name() const

   Returns the signal name.
*/

/*!
    \fn void AnalogSignal::setName(QString name)

   Sets the signal name.
*/

/*!
    \fn AnalogTriggerState AnalogSignal::triggerState() const

   Returns the trigger state for this signal.
*/

/*!
    \fn void AnalogSignal::setTriggerState(AnalogTriggerState state)

   Sets the trigger state for this signal to \a state.
*/
void AnalogSignal::setTriggerState(AnalogTriggerState state)
{
    if (state == mTriggerState) return;

    mTriggerState = state;

    if (mReconfigureListener != NULL) {
        mReconfigureListener->reconfigure();
    }
}

/*!
    \fn AnalogCoupling AnalogSignal::coupling() const

   Returns the coupling for this signal.
*/

/*!
    \fn void AnalogSignal::setCoupling(AnalogCoupling c)

   Sets the coupling for this signal to \a c.
*/
void AnalogSignal::setCoupling(AnalogCoupling c)
{
    if (c == mCoupling) return;

    mCoupling = c;

    if (mReconfigureListener != NULL) {
        mReconfigureListener->reconfigure();
    }
}

/*!
    \fn double AnalogSignal::vPerDiv() const

   Returns the Volts/Div this signal.
*/

/*!
    \fn void AnalogSignal::setVPerDiv(double v)

   Sets the Volts/Div for this signal to \a v.
*/
void AnalogSignal::setVPerDiv(double v)
{
    if (v == mVPerDiv) return;

    mVPerDiv = v;

    if (mReconfigureListener != NULL) {
        mReconfigureListener->reconfigure();
    }
}

/*!
    \fn double AnalogSignal::triggerLevel() const

   Returns the trigger level for this signal.
*/

/*!
    \fn void AnalogSignal::setTriggerLevel(double l)

   Sets the trigger level for this signal to \a l.
*/
void AnalogSignal::setTriggerLevel(double l)
{
    if (l == mTriggerLevel) return;

    mTriggerLevel = l;

    if (mReconfigureListener != NULL) {
        mReconfigureListener->reconfigure();
    }
}

/*!
    \fn AnalogWaveform AnalogSignal::waveform() const

   Returns the waveform that will be generated for this signal.
*/

/*!
    \fn void AnalogSignal::setWaveform(AnalogWaveform waveform)

   Sets the waveform to generate for this signal to \a waveform.
*/

/*!
    \fn int AnalogSignal::frequency() const

   Returns the frequency for this signal.
*/

/*!
    \fn void AnalogSignal::setFrequency (int freq)

   Sets the frequency for this signal to \a freq.
*/

/*!
    \fn double AnalogSignal::amplitude() const

   Returns the amplitude for this signal.
*/

/*!
    \fn void AnalogSignal::setAmplitude(double amp)

   Sets the amplitude for this signal to \a amp.
*/


/*!
    Returns a string representation of this analog signal. This is typically
    used when saving settings to persistent storage.

    \sa fromSettingsString()
*/
QString AnalogSignal::toSettingsString()
{
    // -- common fields
    // type;usage;id;name;

    // -- capture fields
    // vPerDiv;triggerState;triggerLevel;coupling

    // -- generate fields
    // waveform;frequency;amplitude


    QString str;
    str.append("Analog;");
    str.append(QString("%1;").arg(mUsage));
    str.append(QString("%1;").arg(mId));
    str.append(mName).append(";");

    if (mUsage == AnalogUsageCapture) {
        str.append(QString("%1;").arg(mVPerDiv));
        str.append(QString("%1;").arg(mTriggerState));
        str.append(QString("%1;").arg(mTriggerLevel));
        str.append(QString("%1").arg(mCoupling));
    }
    else {
        str.append(QString("%1;").arg(mWaveform));
        str.append(QString("%1;").arg(mFrequency));
        str.append(QString("%1").arg(mAmplitude));
    }


    return str;
}

/*!
    Create an analog signal from a string representation. The string must have
    been created by AnalogSignal::toSettingsString().

    If the parsing of the string fails an AnalogSignal with ID
    AnalogSignal::InvalidAnalogId will be returned

    \sa toSettingsString()
*/
AnalogSignal AnalogSignal::fromSettingsString(QString& s)
{
    bool ok = false;
    AnalogSignal tmp;
    tmp.mId = AnalogSignal::InvalidAnalogId;

    do {
        // -- common fields
        // type;usage;id;name;

        // -- capture fields
        // vPerDiv;triggerState;triggerLevel;coupling

        // -- generate fields
        // waveform;frequency;amplitude

        QStringList list = s.split(';');
        if (list.size() < 7) break;

        // --- type
        if (list.at(0) != "Analog") break;

        // --- usage
        int u = list.at(1).toInt(&ok);
        if (!ok) break;
        if (u < 0 || u > AnalogUsageGenerate) break;
        AnalogUsage usage = (AnalogUsage)u;

        // --- ID
        int id = list.at(2).toInt(&ok);
        if (!ok) break;

        // --- name
        QString name = list.at(3);
        if (name.isNull() || name.isEmpty()) break;


        if (usage == AnalogUsageCapture) {

            if (list.size() < 8) break;

            // --- voltperdiv
            double vperdiv = list.at(4).toDouble(&ok);
            if (!ok) break;
            if (vperdiv < 0 || vperdiv > 5) break;

            // --- trigger state
            int t = list.at(5).toInt(&ok);
            if (!ok) break;
            if (t < 0 || t >= AnalogSignal::AnalogTriggerNum) break;

            AnalogSignal::AnalogTriggerState trigger
                    = (AnalogSignal::AnalogTriggerState)t;

            // --- trigger level
            double level = list.at(6).toDouble(&ok);
            if (!ok) break;

            // --- AC/DC coupling
            int c = list.at(7).toInt(&ok);
            if (!ok) break;
            if (c < 0 || c >= AnalogSignal::CouplingNum) break;

            AnalogSignal::AnalogCoupling coupling
                    = (AnalogSignal::AnalogCoupling)c;


            tmp.mUsage = usage;
            tmp.mId = id;
            tmp.mName = name;
            tmp.mVPerDiv = vperdiv;
            tmp.mTriggerState = trigger;
            tmp.mTriggerLevel = level;
            tmp.mCoupling = coupling;

        }
        else {

            // --- waveform
            int w = list.at(4).toInt(&ok);
            if (!ok) break;
            if (w < 0 || w >= AnalogSignal::WaveformNum) break;
            AnalogSignal::AnalogWaveform waveform
                    = static_cast<AnalogSignal::AnalogWaveform>(w);

            // --- frequency
            int freq = list.at(5).toInt(&ok);
            if (!ok) break;

            // --- amplitude
            double amp = list.at(6).toDouble(&ok);
            if (!ok) break;

            tmp.mUsage = usage;
            tmp.mId = id;
            tmp.mName = name;
            tmp.mWaveform = waveform;
            tmp.mFrequency = freq;
            tmp.mAmplitude = amp;
        }



    } while (false);

    return tmp;
}

/*!
    Set the reconfigure listener to \a listener. The reconfigure listener will
    be called whenever a state is changed that might need a reconfiguration of
    the device.
*/
void AnalogSignal::setReconfigureListener(ReconfigureListener* listener)
{
    /*
        Normally signals and slots would be used instead of a listener.
        The AnalogSignal class, however, is used in a way where it must
        be copied (stored in QList for example). Classes which implement
        QObject cannot be copied and we would have had to implement QObject
        in order to use signals and slots.
    */

    mReconfigureListener = listener;
}

bool analogSignalLessThan(const AnalogSignal* s1, const AnalogSignal* s2) {
    return s1->id() < s2->id();
}
