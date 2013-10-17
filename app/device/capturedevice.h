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
#ifndef CAPTUREDEVICE_H
#define CAPTUREDEVICE_H

#include <QDebug>
#include <QObject>
#include <QList>
#include <QMessageBox>

#include "digitalsignal.h"
#include "analogsignal.h"
#include "reconfigurelistener.h"

class CaptureDevice : public QObject, public ReconfigureListener
{
    Q_OBJECT
public:

    explicit CaptureDevice(QObject *parent = 0);
    ~CaptureDevice();

    virtual QList<int> supportedSampleRates() = 0;
    virtual int maxNumDigitalSignals() = 0;
    virtual int maxNumAnalogSignals() = 0;
    virtual QList<double> supportedVPerDiv();
    virtual bool supportsContinuousCapture() {return false;}

    virtual void configureBeforeStart(QWidget* parent) {(void)parent;/* do nothing by default */}
    virtual void configureTrigger(QWidget* parent)
    {
        QMessageBox::warning(
                    parent,
                    tr("No settings"),
                    tr("No trigger settings for this device"));
    }
    virtual void calibrate(QWidget* parent)
    {
        QMessageBox::warning(
                    parent,
                    tr("No settings"),
                    tr("No calibration settings for this device"));
    }

    virtual void start(int sampleRate) = 0;
    virtual void stop() = 0;

    virtual int usedSampleRate() {return mUsedSampleRate;}
    virtual void setUsedSampleRate(int sampleRate) {mUsedSampleRate = sampleRate;}
    virtual int lastSampleIndex() = 0;

    DigitalSignal* addDigitalSignal(int id);
    void removeDigitalSignal(DigitalSignal* s);
    QList<int> unusedDigitalIds();
    QString digitalSignalName(int id);
    QList<DigitalSignal*> digitalSignals() {return mDigitalSignalList;}

    virtual QVector<int>* digitalData(int signalId) = 0;
    virtual void setDigitalData(int signalId, QVector<int> data) = 0;

    AnalogSignal* addAnalogSignal(int id);
    void removeAnalogSignal(AnalogSignal* s);
    QList<int> unusedAnalogIds();
    QList<AnalogSignal*> analogSignals() {return mAnalogSignalList;}

    virtual QVector<double>* analogData(int signalId) = 0;
    virtual void setAnalogData(int signalId, QVector<double> data) = 0;

    virtual void clearSignalData() = 0;

    virtual int digitalTriggerIndex() = 0;
    virtual void setDigitalTriggerIndex(int idx) = 0;

    virtual void digitalTransitions(int signalId, QList<int> &list);


signals:
    void captureFinished(bool successful, QString msg);
    
public slots:


protected:
    int mUsedSampleRate;
    QList<DigitalSignal*> mDigitalSignalList;
    QList<AnalogSignal*> mAnalogSignalList;


    
};

#endif // CAPTUREDEVICE_H
