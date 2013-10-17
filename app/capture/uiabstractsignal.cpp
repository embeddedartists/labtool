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
#include "uiabstractsignal.h"

/*!
    \class UiAbstractSignal
    \brief UiAbstractSignal is the base class for all signal related widgets,
        that is, widgets that will plot signal data in the plot area.

    \ingroup Capture

*/


/*!
    Constructs an UiAbstractSignal with the given \a parent.
*/
UiAbstractSignal::UiAbstractSignal(QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    mTimeAxis = NULL;
    mSelected = false;
}

/*!
    Sets the time axis for this abstract signal to \a axis.
*/
void UiAbstractSignal::setTimeAxis(UiTimeAxis *axis)
{
    mTimeAxis = axis;
}


/*!
    \fn virtual void UiAbstractSignal::handleSignalDataChanged()

    Called when signal data has changed. Default implementation doesn't
    do anything. A sub-class should reimplement this function if it needs
    to know when data changes.
*/

/*!
    \fn void UiAbstractSignal::closed(UiAbstractSignal* s);

    This signal is emitted when a signal is closed.
*/


/*!
    Close this signal.
*/
void UiAbstractSignal::closeSignal()
{
    //
    //    We only send a signal that we have closed. It is then the
    //    responsibility of SignalManager to actually handle the close
    //    and deallocation process
    //

    emit closed(this);
}

/*!
    Paint the background of this signal widget.
*/
void UiAbstractSignal::paintBackground(QPainter* painter)
{
    QLinearGradient gradient(0,height(),0,height()/2);

    if (mSelected) {
        gradient.setColorAt(0, QColor::fromRgbF(0.9, 0.9, 1.0, 0.5));
        gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    }
    else {
        gradient.setColorAt(0, QColor::fromRgbF(0.9, 0.9, 0.9, 0.5));
        gradient.setColorAt(1, QColor::fromRgbF(0, 0, 0, 0));
    }


    QBrush brush(gradient);

    painter->fillRect(0,0,width(),height(), brush);
}

/*!
    Event handler that is called when the mouse cursor enters this widget
*/
void UiAbstractSignal::enterEvent(QEvent* event)
{
    (void)event;
    mSelected = true;
    update();
}

/*!
    Event handler that is called when the mouse cursor leaves this widget
*/
void UiAbstractSignal::leaveEvent(QEvent* event)
{
    (void)event;
    mSelected = false;
    update();
}

/*!
    \enum UiAbstractSignal::Margins

    This enum describes the margins of this widget.

    \var UiAbstractSignal::Margins UiAbstractSignal::InfoMarginTop
    Top margin

    \var UiAbstractSignal::Margins UiAbstractSignal::InfoMarginRight
    Right margin

    \var UiAbstractSignal::Margins UiAbstractSignal::InfoMarginBottom
    Bottom margin

    \var UiAbstractSignal::Margins UiAbstractSignal::InfoMarginLeft
    Left margin
*/

/*!
    \var UiTimeAxis* UiAbstractSignal::mTimeAxis

    The time axis used by this signal widget.
*/

/*!
    \var bool UiAbstractSignal::mSelected

    Set to true if the signal is selected.
*/


/*!
    Returns rectangle offsets for this widget where content can be placed
*/
QRect UiAbstractSignal::infoContentRect()
{
    return QRect(InfoMarginLeft, InfoMarginTop,
                 infoWidth()-InfoMarginLeft-InfoMarginRight,
                 height()-InfoMarginTop-InfoMarginBottom);
}

/*!
    Returns content margins for this widget.
*/
QMargins UiAbstractSignal::infoContentMargin()
{
    return QMargins(InfoMarginLeft, InfoMarginTop, InfoMarginRight,
                    InfoMarginBottom);
}
