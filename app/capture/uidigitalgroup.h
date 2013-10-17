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
#ifndef UIDIGITALGROUP_H
#define UIDIGITALGROUP_H

#include <QGroupBox>
#include <QLabel>

class UiDigitalGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit UiDigitalGroup(QWidget *parent = 0);
    
signals:
    
public slots:
    void setCycleData(double start, double mid, double end, bool highLow, bool active);

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
        HoriDistBetweenRelated = 5
    };

    enum MeasureIndexes {
        MeasurePeriod = 0,
        MeasureFrequency,
        MeasureWidth,
        MeasureDutyCycle,
        NumMeasurements // Must be last
    };

    QLabel mMeasureLbl[NumMeasurements];
    QLabel mMeasure[NumMeasurements];

    QSize mMinSize;

    void setupLabels();
    void doLayout();
    
};

#endif // UIDIGITALGROUP_H
