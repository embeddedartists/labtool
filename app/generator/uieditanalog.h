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
#ifndef UIEDITANALOG_H
#define UIEDITANALOG_H

#include <QWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QDoubleSpinBox>

#include "device/analogsignal.h"

#include "uianalogshape.h"

class UiEditAnalog : public QWidget
{
    Q_OBJECT
public:
    explicit UiEditAnalog(AnalogSignal* signal, QWidget *parent = 0);

    AnalogSignal* signal() {return mSignal;}
    void invalidateSignal();
    
signals:
    
public slots:

private:
    AnalogSignal* mSignal;
    QLineEdit* mNameEdit;
    QLineEdit* mRate;
    QString mLastRateText;
    QComboBox* mWaveBox;
    QDoubleSpinBox* mAmpBox;
    UiAnalogShape* mShape;

    QComboBox* createWaveformBox(AnalogSignal::AnalogWaveform selected = AnalogSignal::WaveformSine);
    QLineEdit* createFrequencyBox();
    QDoubleSpinBox *createAmplitudeBox();

private slots:
    void handleNameEdited();
    void updateRate();
    void changeWaveform(int selectedIdx);
    void amplitudeChanged(double v);
    
};

#endif // UIEDITANALOG_H
