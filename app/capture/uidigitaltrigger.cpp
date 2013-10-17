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
#include "uidigitaltrigger.h"

#include <QDebug>
#include <QPainter>

/*!
    \class UiDigitalTrigger
    \brief UI widget that is responsible for controlling trigger state
    of a digital signal

    \ingroup Capture

*/


/*!
    Constructs an UiDigitalTrigger with the given \a parent.
*/
UiDigitalTrigger::UiDigitalTrigger(QWidget *parent) :
    QWidget(parent)
{
    mState = DigitalSignal::DigitalTriggerNone;
    resize(15, 15);
}

/*!
    Returns the trigger state.
*/
DigitalSignal::DigitalTriggerState UiDigitalTrigger::state()
{
    return mState;
}

/*!
    Sets the trigger state to \a state.
*/
void UiDigitalTrigger::setState(DigitalSignal::DigitalTriggerState state)
{
    mState = state;
    update();
}

/*!
    \fn void UiDigitalTrigger::triggerSet()

    This signal is emitted when the trigger state is changed.
*/



/*!
    Paint event handler responsible for painting this widget.
*/
void UiDigitalTrigger::paintEvent(QPaintEvent *event)
{
    (void)event;
    int m = 3;
    int mid = width() / 2;
    QPainter painter(this);


//    painter.setBrush(QColor(255,255,255));
//    painter.drawRoundRect(0, 0, width()-1, height()-1, 10, 10);

    painter.drawLine(1, 0, width()-2, 0);
    painter.drawLine(width()-1, 1, width()-1, height()-2);
    painter.drawLine(1, height()-1, width()-2, height()-1);
    painter.drawLine(0, 1, 0, height()-2);
    painter.fillRect(1, 1, width()-2, height()-2, QColor(255,255,255));

    QPen pen = painter.pen();
    pen.setWidth(2);
    painter.setPen(pen);

    switch(mState) {
    case DigitalSignal::DigitalTriggerNone:
        // nothing to draw
        break;
    case DigitalSignal::DigitalTriggerHighLow:

        // top line
        painter.drawLine(m, m, mid, m);
        // transition
        painter.drawLine(mid, m, mid, height()-m);
        // bottom line
        painter.drawLine(mid, height()-m, width()-m-1, height()-m);

        break;
    case DigitalSignal::DigitalTriggerLowHigh:

        // bottom line
        painter.drawLine(m, height()-m, mid, height()-m);
        // transition
        painter.drawLine(mid, m, mid, height()-m);
        // top line
        painter.drawLine(mid, m, width()-m-1, m);

        break;
#if 0 // disabling high level and low level as trigger levels
    case DigitalSignal::DigitalTriggerHigh:

        // top line
        painter.drawLine(m, m, width()-m, m);

        break;

    case DigitalSignal::DigitalTriggerLow:

        // bottom line
        painter.drawLine(m, height()-m-1, width()-m, height()-m-1);

        break;
#endif
    default:
        break;
    }

}

/*!
    Mouse press event handler called when a mouse button is pressed.
*/
void UiDigitalTrigger::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        int s = (int) mState;
        s++;
        if (s >= DigitalSignal::DigitalTriggerNum) s = 0;
        mState = (DigitalSignal::DigitalTriggerState)s;
        update();

        emit triggerSet();
    }
}
