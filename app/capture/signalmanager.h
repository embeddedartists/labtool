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
#ifndef UISIGNALMANAGER_H
#define UISIGNALMANAGER_H

#include <QWidget>
#include <QVector>
#include <QMap>
#include <QList>
#include <QSettings>

#include "uiabstractsignal.h"
#include "uidigitalsignal.h"
#include "uianalogsignal.h"

#include "analyzer/uianalyzer.h"

class SignalManager : public QObject
{
    Q_OBJECT
public:    
    explicit SignalManager(QObject *parent = 0);

    QList<UiAbstractSignal*>& signalList() {return mSignalList;}

    void saveSignalSettings(QSettings &settings, QDataStream &out);
    void loadSignalsFromSettings(QSettings &settings, QDataStream &in);

    void addDigitalSignal(int id);
    void addAnalogSignal(int id);
    void addAnalyzer(UiAnalyzer* analyzer);

    void closeAllSignals(bool removeDeviceSignals);
    void reloadSignalsFromDevice();

    double closestDigitalTransition(double startTime);
    
signals:
    void signalsAdded();
    void signalsRemoved();
    void digitalMeasurmentChanged(double start, double mid, double end,
                                bool highLow, bool mActive);
    void analogMeasurmentChanged(QList<double>level, QList<double>pk, bool active);
    
public slots:



private:

    enum {
        SignalDigital    = 1,
        SignalAnalog     = 2,
        SignalDataMagic  = 0xEA0102AE,
        SignalStartMagic = 0x000000EB
    };

    QList<UiAbstractSignal*> mSignalList;

    UiAnalogSignal* mAnalogSignalWidget;

    QBitArray digitalSignalDataToBitArray(QVector<int>* data);
    QVector<int> bitArrayToDigitalSignal(QBitArray data);

    double getClosestDigitalTransitionForSignal(double t, int signalId);
    int activeDigitalSignalId();

    void addDigitalSignal(DigitalSignal* s);
    void addAnalogSignal(AnalogSignal* s);

private slots:
    void closeSignal(UiAbstractSignal* s, bool removeDeviceSignal=true);
    void handleDigitalTriggerSet();
    void handleAnalogTriggerSet();

};


#endif // UISIGNALMANAGER_H
