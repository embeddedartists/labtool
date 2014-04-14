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
#include "uidigitalsignal.h"

#include <QDebug>
#include <QPainter>

#include <QApplication>
#include <QDrag>
#include <QMimeData>

#include "uidigitaltrigger.h"
#include "common/configuration.h"
#include "device/devicemanager.h"


/*!
    \class UiDigitalSignal
    \brief UI widget that represents a digital signal.

    \ingroup Capture

    This widget is responsible for visualizing and controlling one
    digital signal.

*/


/*!
    Constructs an UiDigitalSignal with the given digital signal \a signal
    and \a parent.
*/
UiDigitalSignal::UiDigitalSignal(DigitalSignal* s, QWidget *parent) :
    UiSimpleAbstractSignal(parent)
{

    mSignal = s;
    mActive = false;

    mMouseOverValid = false;
    mTransitionTimes[0] = 0;
    mTransitionTimes[1] = 0;
    mTransitionTimes[2] = 0;

    mIdLbl->setText(QString("D%1").arg(s->id()));
    mNameLbl->setText(s->name());

    mColorLbl->setText("    ");
    QString color = Configuration::instance().digitalCableColor(s->id()).name();
    mColorLbl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mTrigger = new UiDigitalTrigger(this);
    mTrigger->setState(s->triggerState());
    mTrigger->show();
    connect(mTrigger, SIGNAL(triggerSet()), this, SLOT(handleTriggerChanged()));

    setFixedHeight(40);

    setMouseTracking(true);
}

/*!
    Set the name of the signal to \a signalName.
*/
void UiDigitalSignal::setSignalName(QString signalName)
{
    UiSimpleAbstractSignal::setSignalName(signalName);
    mSignal->setName(signalName);
}


/*!
    \fn DigitalSignal* UiDigitalSignal::signal()

    Returns the digital signal associated with this widget.
*/


/*!
    Set the trigger state to \a state.
*/
void UiDigitalSignal::setTriggerState(DigitalSignal::DigitalTriggerState state)
{
    mSignal->setTriggerState(state);
    mTrigger->setState(state);
}

/*!
    \fn bool UiDigitalSignal::isActive()

    Returns true if this signal widget is considered to be active, that is,
    the user holds the mouse cursor above the widget.
*/

/*!
    \fn void UiDigitalSignal::cycleMeasurmentChanged(double start, double mid, double end,
                            bool highLow, bool mActive)

    This signal is emitted when a measurement related to this digital signal
    has changed.
*/

/*!
    \fn void UiDigitalSignal::triggerSet()

    This signal is emitted when the trigger state has changed.
*/


/*!
    Paint event handler responsible for painting this widget.
*/
void UiDigitalSignal::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);


    // -----------------
    // draw background
    // -----------------
    paintBackground(&painter);

    if (mTimeAxis == NULL) return;

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    QVector<int>* data = device->digitalData(mSignal->id());

    if (data == NULL) return;

    // -----------------
    // draw signal
    // -----------------
    QList<int> trans;

    device->digitalTransitions(mSignal->id(), trans);

    QPen pen = painter.pen();
    pen.setColor(Configuration::instance().digitalSignalColor(mSignal->id()));
    painter.setPen(pen);

    paintSignal(&painter, &trans, device->usedSampleRate());

    if (mMouseOverValid) {
        paintArrows(&painter);
    }


}

/*!
    Mouse move event handler called when the mouse has moved above this widget.
*/
void UiDigitalSignal::mouseMoveEvent(QMouseEvent *event)
{
    if (!mActive) {
        mActive = true;
    }

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    QVector<int>* data = device->digitalData(mSignal->id());
    QList<int> trans;

    device->digitalTransitions(mSignal->id(), trans);

    if (data != NULL && event->pos().x() >= plotX()) {
        double xTime = mTimeAxis->pixelToTimeRelativeRef(
                    event->pos().x());

        // find first sample
        int idx = (int)(xTime*device->usedSampleRate());

        do {

            // assuming that t=0 is start for all samples
            if (idx < 0) break;
            // outside of sample data
            if (idx >= trans.at(trans.size()-1)) break;

            int level = trans.at(0);

            /*
                Need to find one transition to the left of where the mouse
                points. Also need to find two transitions to the right.

                x = where mouse points

                  +---+      +----
                  | x |      |
                --+   +------+

                  ^   ^      ^
                  |   |      |
                1:st  2:nd   3:rd
            */

            // find first transition left of index
            int leftTransitionIdx = -1;
            for (int i = 1; i < trans.size(); i++) {
                if (trans.at(i) > idx && i > 1) {
                    leftTransitionIdx = trans.at(i-1);
                    idx = i-1;
                    break;
                }
            }
            // didn't find a transition left of point
            if (leftTransitionIdx == -1) break;


            idx++;
            // no transition
            if (idx > trans.size()-1) break;

            int right1TransitionIdx = trans.at(idx);
            // record logic level at first transition right of point
            if ((idx % 2) != 0) {
                level = ((level + 1) % 2);
            }

            idx++;
            // no transition
            if (idx > trans.size()-1) break;

            int right2TransitionIdx = trans.at(idx);


            bool highLow = true;
            if (level == 1) {
                highLow = false;
            }

            int rate = device->usedSampleRate();
            double t1 = (double)(leftTransitionIdx)/rate;
            double t2 = (double)(right1TransitionIdx)/rate;
            double t3 = (double)(right2TransitionIdx)/rate;

            // check if this is a new measurement
            if (t1 != mTransitionTimes[0] ||
                    t2 != mTransitionTimes[1] ||
                    t3 != mTransitionTimes[2] ||
                    !mMouseOverValid)
            {
                mTransitionTimes[0] = t1;
                mTransitionTimes[1] = t2;
                mTransitionTimes[2] = t3;

                mMouseOverValid = true;

                emit cycleMeasurmentChanged(t1, t2, t3, highLow, true);
            }


        } while(false);

        update();
    }


    QWidget::mouseMoveEvent(event);
}

/*!
    Leave event handler called when the mouse cursor leaves this widget.
*/
void UiDigitalSignal::leaveEvent(QEvent* event)
{
    UiSimpleAbstractSignal::leaveEvent(event);

    if (mActive) {
        mActive = false;
    }

    mMouseOverValid = false;
    update();

    emit cycleMeasurmentChanged(0, 0, 0, false, false);
}

/*!
    Show event handler called when this widget is set to visible.
*/
void UiDigitalSignal::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Paint the signal data.
*/
void UiDigitalSignal::paintSignal(QPainter* painter, QList<int> *data,
                                  int sampleRate)
{

    int yFactor = height()/2;

    int fromIdx = (int)(mTimeAxis->rangeLower()*sampleRate);

    if (fromIdx < 0) fromIdx = 0;

    int toIdx = 0;
    int level = data->at(0);

    double from = 0;
    double to = 0;
//    double lastTransitionPx = -1;

    painter->save();
    painter->setClipRect(infoWidth(), 0, plotWidth(), height());

    // vertical: position signal at center
    painter->translate(0, height()-(height()-yFactor)/2);

    int start = 1;
    for (start = 1; start < data->size(); start++) {
        if (data->at(start) > fromIdx) break;
    }

    if ((start % 2) == 0) {
        level = ((level + 1) % 2);
    }

    for (int i = start; i < data->size(); i++) {

        toIdx = data->at(i);

        from = mTimeAxis->timeToPixelRelativeRef((double)fromIdx/sampleRate);
        to = mTimeAxis->timeToPixelRelativeRef((double)toIdx/sampleRate);

        // no need to draw when signal is out of plot area
        if (from > width()) {
            break;
        }

        // optimize: don't draw anything if we have already
        // drawn transition at this position
        double diff = to - from;
        if (diff < 0) diff = -diff;
        if (diff < 1) {
            level = ((level + 1) % 2);
            continue;
        }

        painter->drawLine(from, -level*yFactor,
                          to, -level*yFactor);


        // the last index of the transition list contains the
        // last index of the signal data independent of any
        // transition
        if (i < data->size()-1) {
            // transition: draw vertical line
            painter->drawLine(to, -level*yFactor,
                              to, -((level + 1)%2)*yFactor);
//            lastTransitionPx = to;
        }


        level = ((level + 1) % 2);
        fromIdx = toIdx;

    }

    painter->restore();
}

/*!
    Paint arrows for period and signal width at mouse cursor position
*/
void UiDigitalSignal::paintArrows(QPainter* painter)
{
    int yForWidth = height()/2;
    int yForPeriod = 4;
    double x1 = mTimeAxis->timeToPixelRelativeRef(mTransitionTimes[0]);
    double x2 = mTimeAxis->timeToPixelRelativeRef(mTransitionTimes[1]);
    double x3 = mTimeAxis->timeToPixelRelativeRef(mTransitionTimes[2]);

    painter->setClipRect(infoWidth(), 0, plotWidth(), height());

    // draw width arrow
    painter->drawLine(x1, yForWidth, x2, yForWidth);
    painter->drawLine(x1, yForWidth, x1+3, yForWidth-3);
    painter->drawLine(x1, yForWidth, x1+3, yForWidth+3);
    painter->drawLine(x2, yForWidth, x2-3, yForWidth+3);
    painter->drawLine(x2, yForWidth, x2-3, yForWidth-3);

    // draw period arrow
    painter->drawLine(x1, yForPeriod, x3, yForPeriod);
    painter->drawLine(x1, yForPeriod, x1+3, yForPeriod-3);
    painter->drawLine(x1, yForPeriod, x1+3, yForPeriod+3);
    painter->drawLine(x3, yForPeriod, x3-3, yForPeriod+3);
    painter->drawLine(x3, yForPeriod, x3-3, yForPeriod-3);
}

/*!
    Called when the info width has changed.
*/
void UiDigitalSignal::infoWidthChanged()
{
    doLayout();
}

/*!
    Update the layout of all child widgets.
*/
void UiDigitalSignal::doLayout()
{
    UiSimpleAbstractSignal::doLayout();

    QRect r = infoContentRect();
    int y = r.top();

    mColorLbl->move(r.left(), y);

    int x = mColorLbl->pos().x()+mColorLbl->width() + SignalIdMarginRight;
    mIdLbl->move(x, y);

    x += mIdLbl->width() + SignalIdMarginRight;
    mNameLbl->move(x, y);
    mEditName->move(x, y);

    x = r.right()-mTrigger->width()/*-5*/;
    mTrigger->move(x, y);
}

/*!
    Returns the minimum width of this widget.
*/
int UiDigitalSignal::calcMinimumWidth()
{

    int w = mNameLbl->pos().x() + mNameLbl->minimumSizeHint().width();
    if (mEditName->isVisible()) {
        w = mEditName->pos().x() + mEditName->width();
    }

    w += mTrigger->width()+5+5;

    return w+infoContentMargin().right();
}

/*!
    Called when the trigger has changed.
*/
void UiDigitalSignal::handleTriggerChanged()
{
    mSignal->setTriggerState(mTrigger->state());
    emit triggerSet();
}
