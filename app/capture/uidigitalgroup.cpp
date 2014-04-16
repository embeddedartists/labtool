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
#include "uidigitalgroup.h"

#include <QDebug>

#include "common/stringutil.h"

/*!
    \class UiDigitalGroup
    \brief UI widget that show digital signal measurements.

    \ingroup Capture

*/


/*!
    Constructs an UiDigitalGroup with the given \a parent.
*/
UiDigitalGroup::UiDigitalGroup(QWidget *parent) :
    QGroupBox(parent),
    mMinSize(0, 0)
{
    setTitle("Digital Measurements");
    setupLabels();
}

/*!
    Sets the measurement data.
*/
void UiDigitalGroup::setCycleData(double start, double mid, double end,
                                  bool highLow, bool active)
{
    if (active) {
        double period = end-start;

        if (period == 0) return;

        double freq = 1/period;
        double sWidth = mid-start;
        double dutyCycle = (sWidth/period)*100;
        if (!highLow) {
            dutyCycle = ((period-sWidth)/period)*100;
        }

        mMeasure[MeasurePeriod].setText(StringUtil::timeInSecToString(period));
        mMeasure[MeasureFrequency].setText(StringUtil::frequencyToString(freq));
        mMeasure[MeasureWidth].setText(StringUtil::timeInSecToString(sWidth));
        mMeasure[MeasureDutyCycle].setText(QString("%1 %").arg(dutyCycle));
    }
    else {
        for (int i = 0; i < NumMeasurements; i++) {
            mMeasure[i].setText("");
        }
    }

    doLayout();
    repaint();
}

/*!
    This event handler is called when this widget is made visible.
*/
void UiDigitalGroup::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
}

/*!
    Returns the minimum size of this widget.
*/
QSize UiDigitalGroup::minimumSizeHint() const
{
    return mMinSize;
}

/*!
    Returns the recommended size of this widget.
*/
QSize UiDigitalGroup::sizeHint() const
{
    return minimumSizeHint();
}

/*!
    Create and setup labels.
*/
void UiDigitalGroup::setupLabels()
{

    for (int i = 0; i < NumMeasurements; i++) {
        mMeasureLbl[i].setParent(this);
        mMeasure[i].setParent(this);

        switch(i) {
        case MeasurePeriod:
            mMeasureLbl[i].setText("Period:");
            break;
        case MeasureFrequency:
            mMeasureLbl[i].setText("Frequency:");
            break;
        case MeasureWidth:
            mMeasureLbl[i].setText("Width:");
            break;
        case MeasureDutyCycle:
            mMeasureLbl[i].setText("Duty Cycle:");
            break;
        default:
            break;
        }
    }
}

/*!
    Position child widgets.
*/
void UiDigitalGroup::doLayout()
{
    int maxLblWidth = 0;
    int minWidth = 0;
    QMargins boxMargins = contentsMargins();

    //
    //    make sure all labels are resized to their minimum size
    //

    for (int i = 0; i < NumMeasurements; i++) {

        mMeasureLbl[i].resize(mMeasureLbl[i].minimumSizeHint());
        mMeasure[i].resize(mMeasure[i].minimumSizeHint());

        if (mMeasureLbl[i].minimumSizeHint().width() > maxLblWidth) {
            maxLblWidth = mMeasureLbl[i].minimumSizeHint().width();
        }

    }

    //
    //    position the labels
    //

    int yPos = MarginTop + boxMargins.top();
    int xPos = MarginLeft + boxMargins.left();
    int xPosRight = xPos + maxLblWidth + HoriDistBetweenRelated;

    for (int i = 0; i < NumMeasurements; i++) {
        mMeasureLbl[i].move(xPos, yPos);
        mMeasure[i].move(xPosRight, yPos);

        yPos += mMeasureLbl[i].height()+VertDistBetweenRelated;

        if (mMeasure[i].x()+mMeasure[i].width() > minWidth) {
            minWidth = mMeasure[i].x()+mMeasure[i].width();
        }
    }

    //
    //    size constraints
    //

    mMinSize.setHeight(mMeasure[NumMeasurements-1].y()
                       +mMeasure[NumMeasurements-1].height()
                       +MarginBottom+boxMargins.bottom());

    // check if QGroupBox has a larger width (because of the box title).
    if (QGroupBox::minimumSizeHint().width()+5 > minWidth) {
        minWidth = QGroupBox::minimumSizeHint().width()+5;
    }
    mMinSize.setWidth(minWidth);

}

