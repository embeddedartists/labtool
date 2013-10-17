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
#ifndef UITIMEAXIS_H
#define UITIMEAXIS_H

#include <QWidget>

#include "uiabstractplotitem.h"

class UiTimeAxis : public UiAbstractPlotItem
{
    Q_OBJECT
public:
    explicit UiTimeAxis(QWidget *parent = 0);

    double timeToPixelRelativeRef(double value);
    double pixelToTimeRelativeRef(double xcoord);
    double timeToPixel(double value);
    double pixelToTime(double value);

    double rangeUpper();
    double rangeLower();
    double reference();
    void setReference(double time);

    void zoom(int steps, double xCenter);
    void zoomAll(double lowerTime, double upperTime);
    void moveAxis(int differenceInPixels);

    enum Constants {
        // number of pixels between major steps
        MajorStepPixelWidth = 100,
        // number of minor steps between major
        NumberOfMinorSteps = 5,
        // Reference time starts at this major step
        ReferenceMajorStep = 1,

        // minimum step time as power of 10
        MinStepAsPowOf10 = -9,
        // maximum step time as power of 10
        MaxStepAsPowOf10 = 3,
        // minimum reference time as power of 10
        MinRefTimeAsPowOf10 = -12
    };

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);

    void infoWidthChanged();


private:
    double mRefTime;
    double mMajorStepTime;
    double mRangeLower;
    double mRangeUpper;

    void updateRange();
    QString getTimeLabelForStep(int majorStep);
    int closestUnitDigit(double value);
    
};

#endif // UITIMEAXIS_H
