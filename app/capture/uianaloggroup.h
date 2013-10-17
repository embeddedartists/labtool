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
#ifndef UIANALOGGROUP_H
#define UIANALOGGROUP_H

#include <QGroupBox>
#include <QLabel>

#include "uianalogsignal.h"

class UiAnalogGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit UiAnalogGroup(QWidget *parent = 0);
    void setNumSignals(int numSignals);
    
signals:
    
public slots:
    void setMeasurementData(QList<double>level, QList<double>pk, bool active);

protected:
    void showEvent(QShowEvent* event);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;


private:

    enum PrivConstants {
        MarginTop = 5,
        MarginRight = 5,
        MarginBottom = 10,
        MarginLeft = 5,
        VertDistBetweenRelated = 0,
        VertDistBetweenUnrelated = 7,
        HoriDistBetweenRelated = 5
    };


    enum MeasureIndexes {
        MeasureV0 = 0,
        MeasureV1,
        MeasureV0V1,
        Measure0PkPk,
        Measure1PkPk,
        NumMeasurements // Must be last
    };

    QLabel mMeasureLbl[NumMeasurements];
    QLabel mMeasure[NumMeasurements];

    QLabel* mMeasureLevelLbl[UiAnalogSignal::MaxNumSignals];
    QLabel* mMeasureLevel[UiAnalogSignal::MaxNumSignals];

    QLabel* mMeasureLevelDiffLbl[UiAnalogSignal::MaxNumSignals/2];
    QLabel* mMeasureLevelDiff[UiAnalogSignal::MaxNumSignals/2];

    QLabel* mMeasurePkLbl[UiAnalogSignal::MaxNumSignals];
    QLabel* mMeasurePk[UiAnalogSignal::MaxNumSignals];

    QSize mMinSize;

    int mNumSignals;

    void setupLabels();
    void doLayout();

};

    


#endif // UIANALOGGROUP_H
