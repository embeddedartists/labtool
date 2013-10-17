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
#ifndef UIDIGITALSIGNAL_H
#define UIDIGITALSIGNAL_H

#include <QWidget>
#include <QMouseEvent>
#include <QPoint>


#include "uisimpleabstractsignal.h"
#include "uidigitaltrigger.h"

#include "device/digitalsignal.h"

class UiDigitalSignal : public UiSimpleAbstractSignal
{
    Q_OBJECT
public:
    explicit UiDigitalSignal(DigitalSignal* s, QWidget *parent = 0);

    void setSignalName(QString signalName);
    DigitalSignal* signal() {return mSignal;}
    void setTriggerState(DigitalSignal::DigitalTriggerState state);
    bool isActive() {return mActive;}

signals:
    void cycleMeasurmentChanged(double start, double mid, double end,
                                bool highLow, bool mActive);
    void triggerSet();
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void leaveEvent(QEvent* event);
    void showEvent(QShowEvent* event);


private:

    DigitalSignal* mSignal;
    bool mActive;
    UiDigitalTrigger* mTrigger;

    double mTransitionTimes[3];
    double mMouseOverValid;


    enum Constants {
        SignalIdMarginRight = 10
    };

    void paintSignal(QPainter* painter, QList<int>* data, int sampleRate);
    void paintArrows(QPainter* painter);

    void infoWidthChanged();
    int calcMinimumWidth();
    void doLayout();

private slots:
    void handleTriggerChanged();
    
};

#endif // UIDIGITALSIGNAL_H
