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
#include "generatordevice.h"

/*!
    \class GeneratorDevice
    \brief GeneratorDevice is the base class of all generator devices.

    \ingroup Device

    The GeneratorDevice class provides the interface to the Generator
    functionality of the Device. Generator functionality means being able to
    generate digital and/or analog output signals.

*/

/*!
    Constructs a generator device with the given \a parent. This class will
    never be instantiated directly. Instead a subclass will inherit from this
    class.
*/
GeneratorDevice::GeneratorDevice(QObject *parent) :
    QObject(parent)
{
    mDigitalEnabled = false;
    mAnalogEnabled = false;
}

/*!
    Deletes any signals that haven't already been deleted by the remove
    functions.
*/
GeneratorDevice::~GeneratorDevice()
{
    removeAllDigitalSignals();
    removeAllAnalogSignals();
}

/*!
    \fn virtual int GeneratorDevice::maxNumDigitalSignals() const

    Rreturns the maximum number of digital signals that can be used by
    the generator. If digital signals isn't supported 0 will be returned.
*/

/*!
    \fn virtual int GeneratorDevice::maxNumAnalogSignals() const

    Returns the maximum number of analog signals that can be used by the
    generator. If analog signals isn't supported 0 will be returned.
*/

/*!
    \fn virtual int GeneratorDevice::maxNumDigitalStates() const

    Returns the maximum number of states supported by a digital signal.
    If, for example, the device supports 32 states this means that a
    total of 32 signal changes can occur at the rate/frequency specified
    when calling the start() function.

    Example: Generating a clock signal with 50% duty cycle requires only two
    states; one high and one low.
*/

/*!
    \fn virtual int GeneratorDevice::maxDigitalRate() const

    Returns the maximal supported rate/frequency in Hz
    at which digitals signals can be generated.
*/

/*!
    \fn virtual int GeneratorDevice::minDigitalRate() const

    Returns the minimal supported rate/frequency in Hz
    at which digitals signals can be generated.
*/

/*!
    \fn virtual int GeneratorDevice::maxAnalogRate() const

    Returns the maximal supported rate/frequency in Hz
    at which analog signals can be generated.
*/

/*!
    \fn virtual int GeneratorDevice::minAnalogRate() const

    Returns the minimal supported rate/frequency in Hz
    at which analog signals can be generated.
*/

/*!
    \fn virtual double GeneratorDevice::maxAnalogAmplitude() const

    Returns the maximal amplitude in Volts that can
    be generated for an analog signal.
*/


/*!
    Returns a list of supported analog waveforms that can be generated
    by this device.

    Reimplement this function in a GeneratorDevice subclass. By default
    all waveforms supported by the application are returned.

*/
QList<AnalogSignal::AnalogWaveform> GeneratorDevice::supportedAnalogWaveforms()
{
    return QList<AnalogSignal::AnalogWaveform>()
            << AnalogSignal::WaveformSine
            << AnalogSignal::WaveformSquare
            << AnalogSignal::WaveformTriangle;
}

/*!
    Enable or disable digital signal generation as specified by \a enable.
*/
void GeneratorDevice::enableDigitalGenerator(bool enable)
{
    if (maxNumDigitalSignals() > 0) {
        mDigitalEnabled = enable;
    }
}

/*!
    Returns true if digital signal generation is enabled.
*/
bool GeneratorDevice::isDigitalGeneratorEnabled()
{
    return mDigitalEnabled && (maxNumDigitalSignals() > 0);
}

/*!
    Enable or disable analog signal generation as specified by \a enable.
*/
void GeneratorDevice::enableAnalogGenerator(bool enable)
{
    if (maxNumAnalogSignals() > 0) {
        mAnalogEnabled = enable;
    }
}

/*!
    Returns true if analog signal generation is enabled.
*/
bool GeneratorDevice::isAnalogGeneratorEnabled()
{
    return mAnalogEnabled && (maxNumAnalogSignals() > 0);
}

/*!
    Create and add a digital signal with \a id to the list of digital signals
    that should be generated.

    This function returns an instance of DigitalSignal if the signal could be
    added; otherwise it returns NULL.

    The \a id is unique for a digital signal and is used by the device to
    identify, for example, a physical output pin where the signal will be
    generated. The IDs range from 0 up to maxNumDigitalSignals-1
*/
DigitalSignal* GeneratorDevice::addDigitalSignal(int id)
{
    DigitalSignal* signal = NULL;

    do {
        // make sure the ID is valid
        if (id < 0 || id >= maxNumDigitalSignals()) break;
        QList<int> unused = unusedDigitalIds();
        if (!unused.contains(id)) break;

        // Deallocation:
        //   DigitalSignal will be deallocated by removeDigitalSignal,
        //   removeAllDigitalSignals or by the destructor.
        signal = new DigitalSignal(DigitalSignal::DigitalUsageGenerate, id);

        mDigitalSignalList.append(signal);
        qSort(mDigitalSignalList.begin(), mDigitalSignalList.end(),
              digitalSignalLessThan);

    } while(false);


    return signal;
}

/*!
    Remove the digital signal \a s from the list of digital signals
    that should be including in the signal generation.
*/
void GeneratorDevice::removeDigitalSignal(DigitalSignal* s)
{
    if (mDigitalSignalList.contains(s)) {
        mDigitalSignalList.removeOne(s);
        delete s;
    }
}

/*!
    Remove all digital signals.
*/
void GeneratorDevice::removeAllDigitalSignals()
{
    qDeleteAll(mDigitalSignalList);
    mDigitalSignalList.clear();
}

/*!
    Returns a list with digital signal IDs that are unused, that is,
    not already added to the list of signals that will be included
    in the next generation.

    \sa addDigitalSignal
*/
QList<int> GeneratorDevice::unusedDigitalIds()
{
    QList<int> l;

    for (int i = 0; i < maxNumDigitalSignals(); i++) {
        bool used = false;

        foreach(DigitalSignal* s, mDigitalSignalList) {

            if (s->id() == i) {
                used = true;
                break;
            }

            // mDigitalSignalList is sorted in ascending order
            // if we find a larger ID than what we are looking for it
            // isn't used
            if (s->id() > i) break;
        }

        if (!used) {
            l.append(i);
        }
    }

    return l;
}


/*!
    \fn QList<DigitalSignal*> GeneratorDevice::digitalSignals()

    Returns a list with the digital signals that have been added to this
    generator device.
*/


/*!
    Create and add an analog signal with \a id to the list of analog signals
    that should be generated.

    This function returns an instance of AnalogSignal if the signal could be
    added; otherwise it returns NULL.

    The \a id is unique for an analog signal and is used by the device to
    identify, for example, a physical output pin where the signal will be
    generated. The IDs range from 0 up to maxNumAnalogSignals-1
*/
AnalogSignal* GeneratorDevice::addAnalogSignal(int id)
{
    AnalogSignal* signal = NULL;

    do {
        // make sure the ID is valid
        if (id < 0 || id >= maxNumAnalogSignals()) break;
        QList<int> unused = unusedAnalogIds();
        if (!unused.contains(id)) break;

        // Deallocation:
        //   AnalogSignal will be deallocated by removeAnalogSignal,
        //   removeAllAnalogSignals or by the destructor.
        signal = new AnalogSignal(AnalogSignal::AnalogUsageGenerate, id);

        mAnalogSignalList.append(signal);
        qSort(mAnalogSignalList.begin(), mAnalogSignalList.end(),
              analogSignalLessThan);

    } while(false);


    return signal;
}

/*!
    Remove the analog signal \a s from the list of analog signals
    that should be including in the signal generation.
*/
void GeneratorDevice::removeAnalogSignal(AnalogSignal* s)
{
    if (mAnalogSignalList.contains(s)) {
        mAnalogSignalList.removeOne(s);
        delete s;
    }
}

/*!
    Remove all analog signals.
*/
void GeneratorDevice::removeAllAnalogSignals()
{
    qDeleteAll(mAnalogSignalList);
    mAnalogSignalList.clear();
}

/*!
    Returns a list with analog signal IDs that are unused, that is,
    not already added to the list of signals that will be included
    in the next generation.

    \sa addAnalogSignal
*/
QList<int> GeneratorDevice::unusedAnalogIds()
{
    QList<int> l;

    for (int i = 0; i < maxNumAnalogSignals(); i++) {
        bool used = false;

        foreach(AnalogSignal* s, mAnalogSignalList) {

            if (s->id() == i) {
                used = true;
                break;
            }

            // AnalogSignal is sorted in ascending order
            // if we find a larger ID than what we are looking for it
            // isn't used
            if (s->id() > i) break;
        }

        if (!used) {
            l.append(i);
        }
    }

    return l;
}

/*!
    \fn QList<AnalogSignal*> GeneratorDevice::analogSignals()

    Returns a list with the analog signals that have been added to this
    generator device.
*/

/*!
    \fn virtual void GeneratorDevice::start(int digitalRate, bool loop) = 0

    This function is called to start generation of signals.
    The digital rate/frequency \a digitalRate is common for all digital
    signals, while the frequency for analog signals can be set on an
    individual basis and is therefore available in the class AnalogSignal.

    The parameter \a loop is set to true of the generation should continue
    to loop until stop is called.

    The signal generateFinished() must be emitted when generation has finished
    or if an error occurs starting or during generation.

    Implement this function in a GeneratorDevice subclass to start the
    generation. The generator device should only generate signals added by
    addDialogSignal or addAnalogSignal and use the configurations
    set in those container classes (DigitalSignal, AnalogSignal).

    \sa DigitalSignal
    \sa AnalogSignal
*/

/*!
    \fn virtual void GeneratorDevice::stop() = 0

    This function is called to stop an ongoing generation.

    \sa start()
*/

/*!
    \fn void GeneratorDevice::generateFinished(bool successful, QString msg)

    This signal is emitted when a generation of signals has finished.
    The \a succcessful parameter indicates if the generation finished
    successfully or not. If it wasn't successful the parameter \a msg
    contains the reason why it wasn't successful.
*/

/*!
    \fn QList<DigitalSignal*> GeneratorDevice::mDigitalSignalList

    List of digital signals that will be used during generation.
*/

/*!
    \fn QList<AnalogSignal*> GeneratorDevice::mAnalogSignalList

    List of analog signals that will be used during generation.
*/
