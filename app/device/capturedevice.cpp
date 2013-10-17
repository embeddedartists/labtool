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
#include "capturedevice.h"

#include <QDebug>
#include <QVector>

/*!
    \class CaptureDevice
    \brief CaptureDevice is the base class of all capture devices.

    \ingroup Device

    The CaptureDevice class provides the interface to the Capture functionality
    of the Device. Capture functionality means being able to sample
    digital and/or analog signals at a given sample rate.

*/

/*!
    Constructs a capture device with the given \a parent. This class will never
    be instantiated directly. Instead a subclass will inherit from this class.
*/
CaptureDevice::CaptureDevice(QObject *parent) :
    QObject(parent)
{
    mUsedSampleRate = 1;
}

/*!
    Deletes any signals that haven't already been deleted by the remove
    functions.
*/
CaptureDevice::~CaptureDevice()
{
    qDeleteAll(mDigitalSignalList);
    qDeleteAll(mAnalogSignalList);
}

/*!
    \fn virtual QList<int> CaptureDevice::supportedSampleRates() = 0

    Returns a list with supported sample rates for this capture device.

    A sample rate from this list may be selected by the user when starting
    a capture. Although the selected sample rate is in the returned list a
    capture can still fail, for example, because the specific configuration
    of number of digital and analog signals cannot be sampled at the given
    rate. If the capture fails this should be reported through the
    captureFinished signal.

    \sa captureFinished()
*/

/*!
    \fn virtual int CaptureDevice::maxNumDigitalSignals() = 0

    Returns the maximum number of digital signals that can be captured
    by this capture device. If digital signals isn't supported 0 will
    be returned.
*/

/*!
    \fn virtual int CaptureDevice::maxNumAnalogSignals() = 0

    Returns the maximum number of analog signals that can be captured by this
    capture device. If analog signals isn't supported 0 will be returned.
*/


/*!
    Returns a list of supported volts per division this capture device
    supports. Volts per division is a known concept from oscilloscopes
    where it controls the vertical size of the waveform. It is basically
    a scale factor defining how many volts each vertical division represents.

    For a CaptureDevice volts per division is only related to analog signals.
*/
QList<double> CaptureDevice::supportedVPerDiv()
{
    return QList<double>();
}

/*!
    \fn virtual bool CaptureDevice::supportsContinuousCapture()

    Returns true if the capture device supports continuous capture,
    that is, it is possible to immediately issue a new start request
    after a capture has finished.
*/

/*!
    \fn virtual void CaptureDevice::configureBeforeStart(QWidget* parent)

    This virtual function is called prior to calling start() and can be used
    by a subclass to ask the user for additional settings before starting
    the capture. Typically a dialog window using \a parent as UI context can
    be shown if additional settings are required.

    Reimplement this function in a CaptureDevice subclass. By default the
    function doesn't do anything.

*/

/*!
    \fn virtual void CaptureDevice::configureTrigger(QWidget* parent)

    If a capture device supports more than the default trigger settings this
    virtual function must be overriden in a subclass. Default trigger settings
    can be found in DigitalSignal and AnalogSignal and are typically; trigger
    at high-low transition or low-high transition for a specific signal.

    A dialog window can be presented to the user by using \a parent as Ui
    context.

    Reimplement this function in a CaptureDevice subclass. By default a message
    dialog is shown to indicate that there isn't any additional trigger settings
    for the device.

*/

/*!
    \fn virtual void CaptureDevice::calibrate(QWidget* parent)

    If a capture device supports calibration this
    virtual function must be overriden in a subclass.

    A dialog window can be presented to the user by using \a parent as Ui
    context.

    Reimplement this function in a CaptureDevice subclass. By default a message
    dialog is shown to indicate that there isn't any additional calibration settings
    for the device.

*/

/*!
    \fn virtual void CaptureDevice::start(int sampleRate) = 0

    Start capturing signals at the sample rate given by \a sampleRate.

    The signal captureFinished() must be emitted when a capture
    request has finished or if an error occured.

    Implement this function in a CaptureDevice subclass to start the
    capture. The capture device should only capture signals added by
    addDialogSignal or addAnalogSignal and use the configurations
    (for example trigger settings) set in those container classes
    (DigitalSignal, AnalogSignal).

    \sa DigitalSignal
    \sa AnalogSignal

*/

/*!
    \fn virtual void CaptureDevice::stop() = 0

    Stop an ongoing capture.

    Implement this function in a CaptureDevice subclass to stop the
    capture. The signal captureFinished() must be emitted when the stop
    request has finished.

    \sa start()
*/

/*!
    \fn virtual int CaptureDevice::usedSampleRate()

    Returns the sample rate used during the last capture.
*/

/*!
    \fn virtual void CaptureDevice::setUsedSampleRate(int sampleRate)

    Sets the used sample rate to \a sampleRate.
*/

/*!
    \fn virtual int CaptureDevice::lastSampleIndex() = 0

    Returns the last valid index for the latest capture
    request. If a 1000 samples were performed this function should return 999.
*/

/*!
    Create and add a digital signal with \a id to the list of digital signals
    that should be including in the next capture.

    This function returns an instance of DigitalSignal if the signal could be
    added; otherwise it returns NULL.

    The \a id is unique for a digital signal and is used by the device to
    identify, for example, a physical input connected to the signal that
    will be captured. The IDs range from 0 up to maxNumDigitalSignals-1
*/
DigitalSignal* CaptureDevice::addDigitalSignal(int id)
{
    DigitalSignal* signal = NULL;

    do {
        // make sure the ID is valid
        if (id < 0 || id >= maxNumDigitalSignals()) break;
        QList<int> unused = unusedDigitalIds();
        if (!unused.contains(id)) break;

        // Deallocation:
        //   DigitalSignal will be deallocated by removeDigitalSignal or
        //   by the destructor.
        signal = new DigitalSignal(DigitalSignal::DigitalUsageCapture, id);
        signal->setReconfigureListener(this);

        mDigitalSignalList.append(signal);
        qSort(mDigitalSignalList.begin(), mDigitalSignalList.end(),
              digitalSignalLessThan);

        // adding a signal might require a reconfiguration
        reconfigure();

    } while(false);


    return signal;
}

/*!
    Remove the digital signal \a s from the list of digital signals
    that should be including in the next capture.
*/
void CaptureDevice::removeDigitalSignal(DigitalSignal* s)
{
    if (mDigitalSignalList.contains(s)) {
        mDigitalSignalList.removeOne(s);
        delete s;

        // removing a signal might require a reconfiguration
        reconfigure();
    }
}

/*!
    Returns a list with digital signal IDs that are unused, that is,
    not already added to the list of signals that will be included
    in the next capture.

    \sa addDigitalSignal
*/
QList<int> CaptureDevice::unusedDigitalIds()
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
    Returns the readable name associated with a specific digital signal
    identified with \a id.
*/
QString CaptureDevice::digitalSignalName(int id)
{
    QString name = QString("Digital %1").arg(id);;

    foreach(DigitalSignal* s, mDigitalSignalList) {
        if (s->id() == id) {
            name = s->name();
            break;
        }
    }

    return name;
}

/*!
    \fn QList<DigitalSignal*> CaptureDevice::digitalSignals()

    Returns a list with the digital signals that have been added to this
    capture device.
*/

/*!
    \fn virtual QVector<int>* CaptureDevice::digitalData(int signalId) = 0

    Returns a vector with the latest captured digital signal data for the
    given \a signalId. NULL is returned if there isn't any data for the given
    ID.
*/

/*!
    \fn virtual void CaptureDevice::setDigitalData(int signalId, QVector<int> data) = 0

    Set digital signal data \a data for the digital signal with ID \a signalID.
*/


/*!
    Create and add an analog signal with \a id to the list of analog signals
    that should be including in the next capture.

    This function returns an instance of AnalogSignal if the signal could be
    added; otherwise it returns NULL.

    The \a id is unique for an analog signal and is used by the device to
    identify, for example, a physical input connected to the signal that
    will be captured. The IDs range from 0 up to maxNumAnalogSignals-1
*/
AnalogSignal* CaptureDevice::addAnalogSignal(int id)
{
    AnalogSignal* signal = NULL;

    do {
        // make sure the ID is valid
        if (id < 0 || id >= maxNumAnalogSignals()) break;
        QList<int> unused = unusedAnalogIds();
        if (!unused.contains(id)) break;

        // Deallocation:
        //   AnalogSignal will be deallocated by removeAnalogSignal or
        //   by the destructor.
        signal = new AnalogSignal(AnalogSignal::AnalogUsageCapture, id);
        signal->setReconfigureListener(this);

        mAnalogSignalList.append(signal);
        qSort(mAnalogSignalList.begin(), mAnalogSignalList.end(),
              analogSignalLessThan);

        // adding a signal might require a reconfiguration
        reconfigure();

    } while(false);


    return signal;
}

/*!
    Remove the analog signal \a s from the list of analog signals
    that should be including in the next capture.
*/
void CaptureDevice::removeAnalogSignal(AnalogSignal* s)
{
    if (mAnalogSignalList.contains(s)) {
        mAnalogSignalList.removeOne(s);
        delete s;

        // removing a signal might require a reconfiguration
        reconfigure();
    }
}

/*!
    Returns a list with analog signal IDs that are unused, that is,
    not already added to the list of signals that will be included
    in the next capture.

    \sa addAnalogSignal
*/
QList<int> CaptureDevice::unusedAnalogIds()
{
    QList<int> l;

    for (int i = 0; i < maxNumAnalogSignals(); i++) {
        bool used = false;

        foreach(AnalogSignal* s, mAnalogSignalList) {

            if (s->id() == i) {
                used = true;
                break;
            }

            // mAnalogSignalList is sorted in ascending order
            // if we find a larger ID than what we are looking for it
            // means that the ID we are looking for isn't used
            if (s->id() > i) break;
        }

        if (!used) {
            l.append(i);
        }
    }

    return l;
}

/*!
    \fn QList<DigitalSignal*> CaptureDevice::analogSignals()

    Returns a list with the analog signals that have been added to this
    capture device.
*/

/*!
    \fn virtual QVector<double>* CaptureDevice::analogData(int signalId) = 0

    Returns a vector with the latest captured analog signal data for the
    given \a signalId. NULL is returned if there isn't any data for the given
    ID.
*/

/*!
    \fn virtual void CaptureDevice::setAnalogData(int signalId, QVector<double> data) = 0

    Set analog signal data \a data for the analog signal with ID \a signalID.
*/

/*!
    \fn virtual void CaptureDevice::clearSignalData() = 0

    Clears any captured signal data.
*/

/*!
    \fn virtual int CaptureDevice::digitalTriggerIndex() = 0

    Returns the sample index where the trigger occured.
*/

/*!
    \fn virtual void CaptureDevice::setDigitalTriggerIndex(int idx) = 0

    Sets the sample index where the trigger occured to \a idx.
*/

/*!
    Get a list with digital transitions for the digital signal with ID
    \a signalId. The first position in the list \a list will contain the
    logical level of the signal at sample index 0. The remaining positions
    (except the last) in the list will contain the sample indexes where
    a transition (high-to-low or low-to-high) occured.

    The last position contains the last sample index of the data even if there
    wasn't a transition at that index.
*/
void CaptureDevice::digitalTransitions(int signalId, QList<int> &list)
{

    QVector<int>* data = digitalData(signalId);
    if (data != NULL && data->size() > 0) {

        int val = data->at(0);

        //
        //  Index 0 always contains the logic level of the data at first
        //  position. The remaining position contains transition index
        //
        list.append(val);


        for (int i = 1; i < data->size(); i++) {
            if (data->at(i) != val) {
                list.append(i);
                val = data->at(i);
            }
        }

        //
        //  The last index of the transition list always contains the "size",
        //  i.e., the last sample time of the data list
        //
        list.append(data->size()-1);

    }
}

/*!
    \fn void CaptureDevice::captureFinished(bool successful, QString msg)

    This signal is emitted when a capture has finished. The \a succcessful
    parameter indicates if the capture finished successfully or not. If the
    capture wasn't successful the parameter \a msg contains the reason why
    it wasn't successful.
*/

/*!
    \fn int CaptureDevice::mUsedSampleRate

    The sample rate used during latest capture.
*/

/*!
    \fn QList<DigitalSignal*> CaptureDevice::mDigitalSignalList

    List of digital signals that will be used during capture.
*/

/*!
    \fn QList<AnalogSignal*> CaptureDevice::mAnalogSignalList

    List of analog signals that will be used during capture.
*/

