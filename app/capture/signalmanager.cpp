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
#include "signalmanager.h"

#include <QDebug>

#include <QBitArray>
#include <QDataStream>
#include <QByteArray>

#include "uidigitalsignal.h"
#include "analyzer/analyzermanager.h"
#include "device/devicemanager.h"


/*!
    \class SignalManager
    \brief The SignalManager class is responsible for creating, deleting,
    and maintaining the UI wdigets related to signals and analyzers.

    \ingroup Capture


*/

/*!
    Constructs the SignalManager with the given \a parent.
*/
SignalManager::SignalManager(QObject *parent) :
    QObject(parent)
{
    mAnalogSignalWidget = NULL;
}

/*!
    \fn QList<UiAbstractSignal*>& SignalManager::signalList()

    Returns the list of added signal widgets.
*/

/*!
    Save signal settings and signal data to persistent storage. The
    settings are stored in \a settings and data are written to \a out.
*/
void SignalManager::saveSignalSettings(QSettings &settings, QDataStream &out)
{
    int idx = 0;

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    settings.beginWriteArray("signals");


    // the file with signal data must start with a Magic number
    out << SignalDataMagic;

    foreach(UiAbstractSignal* s, SignalManager::mSignalList) {

        UiDigitalSignal* ds = qobject_cast<UiDigitalSignal*>(s);
        if (ds != NULL) {
            DigitalSignal* signal = ds->signal();

            settings.setArrayIndex(idx++);
            settings.setValue("meta", signal->toSettingsString());

            QVector<int>* data = device->digitalData(signal->id());
            if (data != NULL) {
                QBitArray binData = digitalSignalDataToBitArray(data);
                out << SignalStartMagic;
                out << SignalDigital;
                out << signal->id();
                out << binData.size();
                out << binData;
            }

            continue;
        }

        UiAnalogSignal* as = qobject_cast<UiAnalogSignal*>(s);
        if (as != NULL) {
            QList<AnalogSignal*> list = as->addedSignals();

            foreach(AnalogSignal* signal, list) {
                settings.setArrayIndex(idx++);
                settings.setValue("meta", signal->toSettingsString());

                QVector<double>* data = device->analogData(signal->id());
                if (data != NULL) {
                    out << SignalStartMagic;
                    out << SignalAnalog;
                    out << signal->id();
                    out << data->size();
                    out << *data;
                }
            }

            continue;
        }

        // not digital or analog, must be an analyzer
        QString metaStr = AnalyzerManager::analyzerToString(qobject_cast<UiAnalyzer*>(s));
        if (!metaStr.isNull()) {
            settings.setArrayIndex(idx++);
            settings.setValue("meta", metaStr);
        }

    }
    settings.endArray();

}

/*!
    Load signal settings and signal data from persistent storage. The
    settings are loaded from \a settings and data are read from \a in.
*/
void SignalManager::loadSignalsFromSettings(QSettings &settings, QDataStream &in)
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    closeAllSignals(true);

    int size = settings.beginReadArray("signals");
    for (int i = 0; i < size; i++) {
        settings.setArrayIndex(i);
        QString meta = settings.value("meta").toString();

        if (meta.contains("Digital;")) {
            DigitalSignal tmp = DigitalSignal::fromSettingsString(meta);

            do {

                DigitalSignal* signal = device->addDigitalSignal(tmp.id());
                if (signal == NULL) break;

                // copy loaded settings
                *signal = tmp;

                // when loading digital signal from settings file the
                // reconfigure listener will be NULL. Copying tmp to signal
                // will then also set the listener to NULL in signal.
                // Set the listener explicitly below.
                signal->setReconfigureListener(device);
                addDigitalSignal(signal);

            } while(false);

        }
        else if (meta.contains("Analog;")) {

            AnalogSignal tmp = AnalogSignal::fromSettingsString(meta);

            do {

                AnalogSignal* signal = device->addAnalogSignal(tmp.id());
                if (signal == NULL) break;

                // copy loaded settings
                *signal = tmp;

                // when loading digital signal from settings file the
                // reconfigure listener will be NULL. Copying tmp to signal
                // will then also set the listener to NULL in signal.
                // Set the listener explicitly below.
                signal->setReconfigureListener(device);
                addAnalogSignal(signal);

            } while(false);

        }
        else {
            UiAnalyzer* analyzer = AnalyzerManager::analyzerFromString(meta);
            if (analyzer != NULL) {
                addAnalyzer(analyzer);
            }
        }

    }
    settings.endArray();

    // load signal data
    uint fileMagic;
    int startMagic;
    int type;
    int id;
    int sz;
    QBitArray digitalData;
    QVector<double> analogData;

    in >> fileMagic;
    if (fileMagic == SignalDataMagic) {
        do {
            in >> startMagic;
            if (startMagic != SignalStartMagic) break;

            in >> type;
            if (type != SignalDigital && type != SignalAnalog) break;

            in >> id;
            in >> sz;

            if (type == SignalDigital) {
                in >> digitalData;
                if (sz != digitalData.size()) break;

                device->setDigitalData(id, bitArrayToDigitalSignal(digitalData));
                digitalData.clear();
            }
            else {
                in >> analogData;
                if (sz != analogData.size()) break;

                device->setAnalogData(id, analogData);
                analogData.clear();
            }



        } while (!in.atEnd());
    }

}

/*!
    Create and add a digital signal with the unique id \a id.
*/
void SignalManager::addDigitalSignal(int id)
{

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    do {

        if (device == NULL) break;

        DigitalSignal* s = device->addDigitalSignal(id);
        if (s == NULL) break;

        addDigitalSignal(s);

    } while(false);

}

/*!
    Create and add an analog signal with the unique id \a id.
*/
void SignalManager::addAnalogSignal(int id)
{

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    do {
        if (device == NULL) break;

        AnalogSignal* s = device->addAnalogSignal(id);
        if (s == NULL) break;

        addAnalogSignal(s);

    } while (false);


}

/*!
    Add the analyzer given by \a analyzer to the list of signal
    widgets.
*/
void SignalManager::addAnalyzer(UiAnalyzer* analyzer)
{
    if (analyzer == NULL) return;

    connect(analyzer, SIGNAL(closed(UiAbstractSignal*)),
            this, SLOT(closeSignal(UiAbstractSignal*)));

    mSignalList.append(analyzer);
    emit signalsAdded();
}

/*!
    Closes all signal widgets and removes the signal containers if
    \a removeDeviceSignals is true.
*/
void SignalManager::closeAllSignals(bool removeDeviceSignals)
{
    foreach(UiAbstractSignal* s, mSignalList) {
        closeSignal(s, removeDeviceSignals);
    }
}

/*!
    Reload and create UI widgets for the signals available in the
    active device.
*/
void SignalManager::reloadSignalsFromDevice()
{
    // close any existing signal widgets (not the device signal since then
    // there wouldn't be anything to reload)
    closeAllSignals(false);

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device == NULL) return;

    foreach(DigitalSignal* ds, device->digitalSignals()) {
        addDigitalSignal(ds);
    }

    foreach(AnalogSignal* as, device->analogSignals()) {
        addAnalogSignal(as);
    }

}

/*!
    Find the closest digital signal transition to the given time \a startTime.
    If there is an active signal (user holds mouse pointer over it) this
    signal will be used; otherwise the order of the signals in the signal
    list will be used when searching for a transition.
*/
double SignalManager::closestDigitalTransition(double startTime)
{

    if (startTime < 0) {
        return -1;
    }

    int signalId = activeDigitalSignalId();
    if (signalId != -1) {
        return getClosestDigitalTransitionForSignal(startTime, signalId);
    }

    double closestTime = -1;
    double lastDiff =    99999999;


    foreach(UiAbstractSignal* s, mSignalList) {
        UiDigitalSignal* ds = qobject_cast<UiDigitalSignal*>(s);
        if (ds != NULL) {
            double t = getClosestDigitalTransitionForSignal(startTime,
                                                            ds->signal()->id());
            double diff = qAbs(startTime-t);
            if (diff < lastDiff) {
                lastDiff = diff;
                closestTime = t;
            }

        }

    }

    return closestTime;
}

/*!
    \fn void SignalManager::signalsAdded()

    This signal is emitted when a signal has been added.
*/

/*!
    \fn void SignalManager::signalsRemoved()

    This signal is emitted when a signal has been removed.
*/

/*!
    \fn void SignalManager::digitalMeasurmentChanged(double start, double mid, double end, bool highLow, bool mActive)

    This signal is emitted when a measurement related to a digital signal has
    occured. The measurement is related to one period closest to where the user
    holds the mouse cursor.
*/

/*!
    \fn void SignalManager::analogMeasurmentChanged(QList<double>level, QList<double>pk, bool active)

    This signal is emitted when a measurement related to an analog signal has
    occured. The measurement is related to the analog signal value for each
    analog signalwhere the user holds the mouse cursor.

    The parameter \a level contains the analog level for each analog signal.
    The parameter \a pk contains peak-to-peak calculations for each
    analog signal. The paramter \a active is true if the measurement is active.
*/


/*!
    Converts the digital signal \a data to a bit array
*/
QBitArray SignalManager::digitalSignalDataToBitArray(QVector<int>* data)
{
    QBitArray a(data->size());
    for (int i = 0; i < data->size(); i++) {
        if (data->at(i) == 1) {
            a.setBit(i, true);
        }
    }

    return a;
}

/*!
    Converts the bit array \a data to a vector with digital states.
*/
QVector<int> SignalManager::bitArrayToDigitalSignal(QBitArray data)
{
    QVector<int> v;
    for (int i = 0; i < data.size(); i++) {
        v.append( data.at(i) ? 1 :0 );
    }

    return v;
}

/*!
    Find the transition closest to time \a t for the signal with given
    \a signalId.
*/
double SignalManager::getClosestDigitalTransitionForSignal(double t, int signalId)
{
    double time = -1;


    CaptureDevice* device = DeviceManager::instance().activeDevice()->captureDevice();

    QList<int> data;
    device->digitalTransitions(signalId, data);


    double period = (double)1/device->usedSampleRate();

    if (data.size() > 0) {

        int startIdx = (int)(t/period);
        int beforeIdx = startIdx;
        int afterIdx = startIdx;

        for (int i = 1; i < data.size()-1; i++) {
            if (startIdx > data.at(i)) {
                beforeIdx = data.at(i);
            }
            if (startIdx < data.at(i)) {

                afterIdx = data.at(i);

                break;
            }
        }

        if (startIdx - beforeIdx < afterIdx - startIdx) {
            time = (beforeIdx+1)*period;
        }
        else {
            time = afterIdx*period;
        }

    }

    return time;
}

/*!
    Get the ID of the active digital signal. Returns -1 if there isn't any
    active signal. A signal is considered active when a user holds to mouse
    cursor ontop of the signal.
*/
int SignalManager::activeDigitalSignalId()
{
    int id = -1;

    foreach(UiAbstractSignal* s, mSignalList) {
        UiDigitalSignal* ds = qobject_cast<UiDigitalSignal*>(s);
        if (ds != NULL && ds->isActive()) {
            id = ds->signal()->id();
            break;
        }
    }

    return id;
}

/*!
    Add the digital signal \a s to the list of signals and setup the
    associated UI widget.
*/
void SignalManager::addDigitalSignal(DigitalSignal* s)
{
    // Deallocation:
    //   Deleted by closeSignal. The signal will also be re-parented
    //   when added to the UiPlot which means that it will be deleted
    //   when UiPlot is deleted.
    UiDigitalSignal* signal = new UiDigitalSignal(s);

    if (signal == NULL) return;

    connect(signal, SIGNAL(closed(UiAbstractSignal*)),
            this, SLOT(closeSignal(UiAbstractSignal*)));
    connect(signal, SIGNAL(triggerSet()), this, SLOT(handleDigitalTriggerSet()));

    connect(signal, SIGNAL(cycleMeasurmentChanged(double,double,double,bool,bool)),
            this, SIGNAL(digitalMeasurmentChanged(double,double,double,bool,bool)));

    mSignalList.append(signal);
    emit signalsAdded();
}

/*!
    Add the analog signal \a s to the list of signals and setup the
    associated UI widget.
*/
void SignalManager::addAnalogSignal(AnalogSignal* s)
{
    if (mAnalogSignalWidget == NULL) {
        // Deallocation:
        //   Deleted by closeSignal. The signal will also be re-parented
        //   when added to the UiPlot which means that it will be deleted
        //   when UiPlot is deleted.
        mAnalogSignalWidget = new UiAnalogSignal();


        connect(mAnalogSignalWidget, SIGNAL(closed(UiAbstractSignal*)),
                this, SLOT(closeSignal(UiAbstractSignal*)));

        connect(mAnalogSignalWidget,
                SIGNAL(measurmentChanged(QList<double>,QList<double>,bool)),
                this,
                SIGNAL(analogMeasurmentChanged(QList<double>,QList<double>,bool)));

        connect(mAnalogSignalWidget, SIGNAL(triggerSet()), this, SLOT(handleAnalogTriggerSet()));

        mSignalList.append(mAnalogSignalWidget);
    }


    mAnalogSignalWidget->addSignal(s);
    signalsAdded();
}

/*!
    Close the given UI signal widget \a s. If \a removeDeviceSignal is true
    the associated signal container will be removed.
*/
void SignalManager::closeSignal(UiAbstractSignal* s, bool removeDeviceSignal)
{
    if (s == NULL) return;

    CaptureDevice* capDevice = DeviceManager::instance().activeDevice()
            ->captureDevice();

    mSignalList.removeOne(s);

    if (removeDeviceSignal) {
        UiDigitalSignal* ds = qobject_cast<UiDigitalSignal*>(s);
        if (ds != NULL && capDevice != NULL) {
            capDevice->removeDigitalSignal(ds->signal());
        }

        if (s == mAnalogSignalWidget && capDevice != NULL) {
            foreach(AnalogSignal* as, mAnalogSignalWidget->addedSignals()) {
                capDevice->removeAnalogSignal(as);
            }
        }
    }

    if (mAnalogSignalWidget == s) {
        mAnalogSignalWidget = NULL;
    }

    s->close();    
    delete s;

    emit signalsRemoved();
}

/*!
    Must be called when a trigger state is modified on a digital signal
*/
void SignalManager::handleDigitalTriggerSet()
{
    // disable triggers for analog signals
    if (mAnalogSignalWidget != NULL) {
        mAnalogSignalWidget->clearTriggers();
    }
}

/*!
    Must be called when a trigger state is modified on an analog signal
*/
void SignalManager::handleAnalogTriggerSet()
{
    // disable triggers for digital signals

    foreach(UiAbstractSignal* s,  mSignalList) {
        UiDigitalSignal* ds = qobject_cast<UiDigitalSignal*>(s);
        if (ds != NULL) {
            ds->setTriggerState(DigitalSignal::DigitalTriggerNone);
        }
    }
}
