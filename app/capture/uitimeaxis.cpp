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
#include "uitimeaxis.h"

#include <QPainter>
#include <QDebug>

#include <qmath.h>

#include "common/stringutil.h"
#include "device/devicemanager.h"

/*!
    \class UiTimeAxis
    \brief UI widget that paints the time axis

    \ingroup Capture

    This class is also responsible for handling "time to pixel" and
    "pixel to time" conversions needed when painting signals.

*/



/*!
    \enum UiTimeAxis::Constants

    This enum describes integer constants used by this widget.

    \var UiTimeAxis::Constants UiTimeAxis::MajorStepPixelWidth
    Number of pixels between major steps

    \var UiTimeAxis::Constants UiTimeAxis::NumberOfMinorSteps
    Number of minor steps between major steps

    \var UiTimeAxis::Constants UiTimeAxis::ReferenceMajorStep
    Reference time starts at this major step

    \var UiTimeAxis::Constants UiTimeAxis::MinStepAsPowOf10
    Minimum step time as power of 10

    \var UiTimeAxis::Constants UiTimeAxis::MaxStepAsPowOf10
    Maximum step time as power of 10

    \var UiTimeAxis::Constants UiTimeAxis::MinRefTimeAsPowOf10
    Minimum Reference time as power of 10
*/


/*!
    Constructs an UiTimeAxis with the given \a parent.
*/
UiTimeAxis::UiTimeAxis(QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    setMinimumHeight(30);

    // the default reference time is 0
    mRefTime = 0.0;
    // 1 ms is the default time between major steps
    mMajorStepTime = 0.001;

    mRangeLower = 0;
    mRangeUpper = 1;


    // we don't want the background to be transparent since signals should
    // be put behind the time axis during vertical scroll
    setAutoFillBackground(true);
}

/*!
    Returns the upper time value for the axis range.
*/
double UiTimeAxis::rangeUpper()
{
    return mRangeUpper;
}

/*!
    Returns the lower time value for the axis range.
*/
double UiTimeAxis::rangeLower()
{
    return mRangeLower;
}

/*!
    Returns the reference time. The time axis has one position as reference
    and other values are normally calulated relative to this reference.
*/
double UiTimeAxis::reference()
{
    return mRefTime;
}

/*!
    Sets the reference time/position.
*/
void  UiTimeAxis::setReference(double value)
{
    if (value < 0 && -value < qPow(10, MinRefTimeAsPowOf10)) {
        value = 0;
    }
    else if (value > 0 && value < qPow(10, MinRefTimeAsPowOf10)) {
        value = 0;
    }

    mRefTime = value;
    updateRange();
    update();
}

/*!
    Returns the pixel position for given time \a value.
*/
double UiTimeAxis::timeToPixel(double value)
{
    return value*MajorStepPixelWidth/mMajorStepTime;
}

/*!
    Returns the time for at the given pixel postition \a value
*/
double UiTimeAxis::pixelToTime(double value)
{
    return value*mMajorStepTime/MajorStepPixelWidth;
}

/*!
    Returns the pixel position relative to the reference position for
    given time \a value
*/
double UiTimeAxis::timeToPixelRelativeRef(double value)
{
    return timeToPixel(value-mRangeLower) + infoWidth();
}

/*!
    Returns the time relative to the reference time for
    given pixel position (x-coordinate) \a xcoord
*/
double UiTimeAxis::pixelToTimeRelativeRef(double xcoord)
{
    // make xcoord relative to the plot area only
    xcoord -= infoWidth();

    return (xcoord*mMajorStepTime) / (MajorStepPixelWidth) + mRangeLower;
}

/*!
    Zoom by specified number of \a steps centered around the given
    x coordinate \a xCenter.
*/
void UiTimeAxis::zoom(int steps, double xCenter)
{
    double center = pixelToTimeRelativeRef(xCenter);
    double newValue = 0;

    double factor = 0.5;
    int majorUnitValue = closestUnitDigit(mMajorStepTime);
    if (steps < 0) {
        steps = -steps;
        factor = 2.0;
        if (majorUnitValue == 2)
            factor = 2.5;
    }
    else {
        factor = 0.5;
        if (majorUnitValue == 5)
            factor = 0.4;
    }
    factor = factor*steps;

    newValue = mMajorStepTime * factor;

    // lower and upper limit on the major step
    if (factor < 1 && newValue < qPow(10, MinStepAsPowOf10)) return;
    if (factor > 1 && newValue > qPow(10, MaxStepAsPowOf10)) return;

    // zoom around the center point
    mMajorStepTime = newValue;
    //mRefTime = center - (center-mRefTime)*factor;
    setReference(center - (center-mRefTime)*factor);

    updateRange();
}

/*!
    Zoom the plot until \a lowerTime and \a upperTime will be visible
    in the range.
*/
void UiTimeAxis::zoomAll(double lowerTime, double upperTime)
{
    double interval = upperTime - lowerTime;

    if (interval <= 0) return;

    setReference(mMajorStepTime);
    updateRange();
    while (upperTime < mRangeUpper) {
        zoom(1, 0);
        setReference(mMajorStepTime);
        updateRange();
    }

    while (upperTime > mRangeUpper) {
        zoom(-1, 0);
        setReference(mMajorStepTime);
        updateRange();
    }


    setReference(mMajorStepTime-(mRangeUpper-upperTime)/2);
    updateRange();

}

/*!
    Move the time axis \a differenceInPixels number of pixels
*/
void UiTimeAxis::moveAxis(int differenceInPixels)
{

    setReference(mRefTime +
               (differenceInPixels*(mMajorStepTime/MajorStepPixelWidth)));


    updateRange();
    update();
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiTimeAxis::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);

    int plotWidth = width()-infoWidth();

    int numMinorSteps = plotWidth
            / (MajorStepPixelWidth/NumberOfMinorSteps) + 1;

    painter.save();
    painter.translate(infoWidth(), 0);

    for (int i = 0; i < numMinorSteps; i++) {
        int stepHeight = 3;

        if (/*i > 0 &&*/ (i % NumberOfMinorSteps) == 0) {
            stepHeight += 9;

            QString stepText = getTimeLabelForStep(i/NumberOfMinorSteps);

            // draw text centered over a major step
            int textWidth = painter.fontMetrics().width(stepText);
            painter.drawText((MajorStepPixelWidth/NumberOfMinorSteps)*i
                             - textWidth/2, 10, stepText);

        }

        // draw minor/major step on the time axis
        painter.drawLine((MajorStepPixelWidth/NumberOfMinorSteps)*i,
                         height()-stepHeight,
                         (MajorStepPixelWidth/NumberOfMinorSteps)*i,
                         height());
    }

    painter.restore();
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiTimeAxis::resizeEvent(QResizeEvent* event)
{
    (void)event;
    updateRange();
}

/*!
    Called when the info width has changed.
*/
void UiTimeAxis::infoWidthChanged()
{
    updateRange();
}

/*!
    Update the range based on widget width.
*/
void UiTimeAxis::updateRange()
{
    int plotWidth = width() - mInfoWidth;

    mRangeLower = mRefTime - ReferenceMajorStep*mMajorStepTime;
    mRangeUpper = mRangeLower
            + mMajorStepTime*plotWidth/MajorStepPixelWidth;
}

/*!
    Get time label for given step \a majorStep.
*/
QString UiTimeAxis::getTimeLabelForStep(int majorStep)
{
    double t = mMajorStepTime*(majorStep-ReferenceMajorStep);

    // get time relative to trigger
    CaptureDevice * device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    double triggerTime = (double)device->digitalTriggerIndex()
            / device->usedSampleRate();
    t -= (triggerTime-mRefTime);

    QString result = StringUtil::timeInSecToString(t);

    if (t > 0) {
        result.prepend("+");
    }

    return result;
}

/*!
    Returns closest unit value for given double \a value.
    Example: 0.0021 -> 2, 30.076 -> 3
*/
int UiTimeAxis::closestUnitDigit(double value)
{

    while(value < 1) {
        value *= 10;
    }

    while(value > 10) {
        value /= 10;
    }

    return (int) value;
}


