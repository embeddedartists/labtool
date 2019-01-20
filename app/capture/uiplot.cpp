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
#include "uiplot.h"

#include <QDebug>
#include <QScrollBar>
#include <QApplication>
#include <QPainter>
#include <QDrag>
#include <QMimeData>

#include "signalmanager.h"
#include "cursormanager.h"
#include "uiselectsignaldialog.h"
#include "common/configuration.h"

#include "device/devicemanager.h"

/*!
    \class UiPlot
    \brief UI area responsible for plotting signals and analyzers

    \ingroup Capture

    UI signal widgets as well as other widgets needed to plot signals
    and analyzers are managed by the UiPlot.
*/

/*!
    Constructs the UiPlot with the given \a parent. The signal manager
    given by \a signalManager is used to keep track of signal widgets.
*/
UiPlot::UiPlot(SignalManager *signalManager, QWidget *parent) :
    QAbstractScrollArea(parent)
{
    mSignalManager = signalManager;

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mTimeAxis = new UiTimeAxis(viewport());

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mGrid = new UiGrid(mTimeAxis, viewport());
    mCursor = CursorManager::instance().createUiCursor(mSignalManager,
                                                       mTimeAxis, viewport());

    connect(mCursor, SIGNAL(cursorChanged(UiCursor::CursorId,bool,double)),
            this, SIGNAL(cursorChanged(UiCursor::CursorId,bool,double)));

    connect(mSignalManager, SIGNAL(signalsAdded()),
            this, SLOT(handleSignalsAdded()));
    connect(mSignalManager, SIGNAL(signalsRemoved()),
            this, SLOT(handleSignalsRemoved()));


    connect(mCursor, SIGNAL(sizeChanged()),
            this, SLOT(updateLayout()));
    connect(mGrid, SIGNAL(sizeChanged()),
            this, SLOT(updateLayout()));
    connect(mTimeAxis, SIGNAL(sizeChanged()),
            this, SLOT(updateLayout()));

    mGrid->move(0, mTimeAxis->height());
    mCursor->move(0, mTimeAxis->height());

    mDraggingPlot = false;
    mDraggingSignal = false;
    mAboutToDragSignal = false;
    mDragSignal = NULL;
    mBelowDragSignal = NULL;

    setAcceptDrops(true);

    QPalette p = palette();
    p.setColor(QPalette::Base, Configuration::instance().outsidePlotColor());
    setPalette(p);
}

/*!
    Zoom the plot by specified number of \a steps centered around the given
    x coordinate \a xCenter.
*/
void UiPlot::zoom(int steps, int xCenter)
{
    int x = xCenter;
    if (xCenter == -1) {
        x = width()/2;
    }

    mTimeAxis->zoom(steps, x);
    updateHorizontalScrollBar();

    viewport()->update();
}

/*!
    Zoom the plot to a level where all signals are visible.
*/
void UiPlot::zoomAll()
{


    mTimeAxis->zoomAll(0, getEndTime());
    updateHorizontalScrollBar();

    viewport()->update();
}

/*!
    Request signals to be redrawn.
*/
void UiPlot::updateSignals()
{
    updateLayout();
}

/*!
    Must be called when signal data changes. Redraws the plot based on the
    current trigger indes.
*/
void UiPlot::handleSignalDataChanged()
{

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    int idx = device->digitalTriggerIndex();

    // We need the trigger position (pixel position) to remain the
    // same between captures. If not, the user experience will be bad since
    // the trigger will move along the x-axis...
    double currRef = mTimeAxis->reference();
    double currTrig = mCursor->cursorPosition(UiCursor::Trigger);
    double newTrig = (double)idx/device->usedSampleRate();
    double newRef = currRef-currTrig+newTrig;

    // cursor times should be relative to the trigger
    for (int i = 0; i < UiCursor::NumCursors; i++) {
        if (i == UiCursor::Trigger) continue;

        double curr = mCursor->cursorPosition(
                    (UiCursor::CursorId)i);
        mCursor->setCursorPosition((UiCursor::CursorId)i,
                                   curr-currTrig+newTrig);
    }

    mTimeAxis->setReference(newRef);
    mCursor->setTrigger(newTrig);

    viewport()->update();
}

/*!
    \fn void UiPlot::cursorChanged(UiCursor::CursorId, bool, double)

    This signal is emitted when a cursor has been changed (moved, enabled,
    or disabled)
*/



/*!
    The paint event handler requesting to repaint this widget.
*/
void UiPlot::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(viewport());

    QRect rect(mTimeAxis->plotX(), 0, width(), height());
    painter.fillRect(rect, Configuration::instance().plotBackgroundColor());

    if (mDraggingSignal) {

        painter.save();

        QPen pen = painter.pen();
        pen.setColor(Qt::darkGray);
        pen.setStyle(Qt::DashLine);
        painter.setPen(pen);

        QRect rect = mDragSignal->geometry();
        rect.adjust(4, 4, -4, -4);
        painter.drawRoundedRect(rect, 10, 10);

        painter.restore();
    }

    // also update the palette
    QPalette p = palette();
    p.setColor(QPalette::Base, Configuration::instance().outsidePlotColor());
    setPalette(p);
}

/*!
    The resize event handler which goes through all widget children and updates
    their sizes.
*/
void UiPlot::resizeEvent(QResizeEvent* event)
{

    mTimeAxis->resize(viewport()->width(), mTimeAxis->height());
    mGrid->resize(viewport()->width(), viewport()->height());
    mCursor->resize(viewport()->width(), viewport()
                    ->height()-mTimeAxis->height());

    foreach(UiAbstractSignal* s, mSignalManager->signalList()) {
        s->resize(viewport()->width(), s->height());
    }

    updateHorizontalScrollBar();

    if (event->oldSize().height() != event->size().height()) {
        updateVerticalScrollBar();
    }
}

/*!
    The wheel event handler is called when a mouse wheel event occurs,
    that is, when the user scrolls the mouse wheel. This implementation
    zooms the plot when the wheel is moved.
*/
void UiPlot::wheelEvent(QWheelEvent *event)
{
    int step = 1;
    if (event->delta() < 0) {
        step = -1;
    }


    zoom(step, event->pos().x());
}

/*!
    The mouse press event handler is called when a mouse button is pressed.
    This implementation will move the entire plot, an individual signal,
    or a cursor depending on where the press event occurs.
*/
void UiPlot::mousePressEvent(QMouseEvent* event)
{
    // get the info width from the grid which is always available.
    int plotXPosition = mGrid->infoWidth();

    if (event->pos().y() >= mCursor->pos().y()&&
            event->pos().y() <= mCursor->pos().y() + mCursor->height()) {

        // Note: Read comment in constructor of UiCursor
        QPoint p = mapToCursor(event->pos());
        if (mCursor->mousePressed(event->button(), p)) {
            return;
        }
    }

    // press on plot area -> drag the plot
    if (event->button() == Qt::LeftButton && event->pos().x() > plotXPosition) {
        mDraggingPlot = true;
        mDragPlotPosition = event->pos();
    }
    // press on info area -> possibly a drag of signal widget
    else if (event->button() == Qt::LeftButton && event->pos().x() < plotXPosition) {

        UiAbstractSignal* child = qobject_cast<UiAbstractSignal*>(
                    childAt(event->pos()));
        if (child != NULL) {
            mDragSignal = child;
            mDragSignalPosition = event->pos();
            mAboutToDragSignal = true;
        }

    }

}

/*!
    The mouse release event handler is called when a mouse button is released.
    This implementation will finsish the dragging of a signal
*/
void UiPlot::mouseReleaseEvent(QMouseEvent *event)
{

    if (event->pos().y() >= mCursor->pos().y()&&
            event->pos().y() <= mCursor->pos().y() + mCursor->height()) {

        // Note: Read comment in constructor of UiCursor
        QPoint p = mapToCursor(event->pos());
        if (mCursor->mouseReleased(event->button(), p)) {
            return;
        }
    }

    if (event->button() == Qt::LeftButton) {
        mDraggingPlot = false;

        mAboutToDragSignal = false;
        mDraggingSignal = false;
    }
}

/*!
    The mouse move event handler is called when a mouse is moved.
    This implementation will move a cursor or a dragged signal.
*/
void UiPlot::mouseMoveEvent(QMouseEvent *event)
{

    if (event->pos().y() >= mCursor->pos().y()&&
            event->pos().y() <= mCursor->pos().y() + mCursor->height()) {

        QPoint p = mapToCursor(event->pos());
        if (mCursor->mouseMoved(event->button(), p)) {
            return;
        }
    }

    if (mDraggingPlot) {

        mTimeAxis->moveAxis(mDragPlotPosition.x()-event->pos().x());
        updateHorizontalScrollBar();

        mDragPlotPosition = event->pos();
        viewport()->update();
    }

    else if (mAboutToDragSignal && (event->buttons() & Qt::LeftButton)) {
        if ((event->pos()-mDragSignalPosition).manhattanLength() >=
                QApplication::startDragDistance())
        {
            mDraggingSignal = true;

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            QDrag *drag = new QDrag(this);


            // Deallocation:
            //   QDrag takes ownership of mimData when calling
            //   drag->setMimeData
            QMimeData *mimeData = new QMimeData;
            mimeData->setData("application/x-uisignal", NULL);

            drag->setMimeData(mimeData);

            // hide the signal that is being moved and then start the drag
            // action
            mDragSignal->hide();
            updateLayout();

            // don't care about the drop action. Success or failure should still
            // result in the signal being enabled/shown again
            drag->exec(Qt::MoveAction);

            // when exec returns we have moved the signal

            mDragSignal->show();

            mDraggingSignal = false;
            mAboutToDragSignal = false;
            mDragSignal = NULL;
            mBelowDragSignal = NULL;

            updateLayout();

        }
    }
}

/*!
    This event handler is called when a scroll bar is moved. The plot is
    updated, that is, signals moved based on the given \a dx and \a dy.
*/
void UiPlot::scrollContentsBy(int dx, int dy)
{

    if (dx != 0) {

        double t = mTimeAxis->pixelToTime(horizontalScrollBar()->value()
                + UiTimeAxis::MajorStepPixelWidth
                * UiTimeAxis::ReferenceMajorStep);

        mTimeAxis->setReference(t);
    }

    if (dy != 0) {
        positionSignals(-verticalScrollBar()->value());
    }


    viewport()->update();

}

/*!
    This event handler is called when a drag is in progress and the mouse
    enters this widget.
*/
void UiPlot::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-uisignal")) {
        event->acceptProposedAction();
    }
}

/*!
    This event handler is called when a drag is in progress and the mouse
    enters this widget.
*/
void UiPlot::dragMoveEvent(QDragMoveEvent *event)
{

    if (event->mimeData()->hasFormat("application/x-uisignal")) {


        do {
            int toIdx = -1;
            QList<UiAbstractSignal*> &signalList = mSignalManager
                    ->signalList();

            // find where the drag signal is located in the list
            int fromIdx = signalList.indexOf(mDragSignal);
            if (fromIdx == -1) break;

            QWidget* childWidget = childAt(event->pos());
            UiAbstractSignal* childSignal
                    = qobject_cast<UiAbstractSignal*>(childWidget);

            // continuing to move over same signal -> don't do anything
            if (childSignal != NULL && mBelowDragSignal == childSignal) break;

            if (childSignal != NULL) {
                toIdx = signalList.indexOf(childSignal);
                mBelowDragSignal = childSignal;
            }
            else if (qobject_cast<UiGrid*>(childWidget) != NULL){
                // moving over "empty" space when it is the grid we are getting
                mBelowDragSignal = NULL;
            }

            if (toIdx == -1) break;

            signalList.move(fromIdx, toIdx);

            updateLayout();
            update();

        } while(0);


        updateLayout();

        event->acceptProposedAction();
    }

}

/*!
    This event handler is called when a drag is dropped on this widget.
*/
void UiPlot::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-uisignal")) {
        event->acceptProposedAction();
    }
}

/*!
    Updates the layout, that is, rearranges the widgets added to UiPlot.
*/
void UiPlot::updateLayout()
{

    int yPos = mTimeAxis->height();
    int w = mCursor->minimumInfoWidth();


    // get the widest info width
    foreach(UiAbstractSignal* signal, mSignalManager->signalList()) {

        signal->move(0, yPos);
        yPos += signal->height();
        if(w < signal->minimumInfoWidth()) {
            w = signal->minimumInfoWidth();
        }
    }

    // update info width part for all widgets
    mTimeAxis->setInfoWidth(w);
    mGrid->setInfoWidth(w);
    mCursor->setInfoWidth(w);

    foreach(UiAbstractSignal* signal, mSignalManager->signalList()) {
        signal->setInfoWidth(w);
    }

    updateHorizontalScrollBar();
    viewport()->update();
}

/*!
    Update the horizontal scrollbar
*/
void UiPlot::updateHorizontalScrollBar()
{

    int plotWidth = viewport()->width()-mTimeAxis->infoWidth();

    double endTime = getEndTime();
    int maxX = (int)mTimeAxis->timeToPixel(endTime);

    // upper range can be larger than end time if scrolling by 'mouse drag'
    if (mTimeAxis->rangeUpper() > endTime) {
        maxX = (int)mTimeAxis->timeToPixel(mTimeAxis->rangeUpper());
    }

    int minX = 0;
    // lower range can be less than 0 if scrolling by mouse drag
    if (mTimeAxis->rangeLower() < 0) {
        minX = mTimeAxis->timeToPixel(mTimeAxis->rangeLower());
    }

    int curr = (int)mTimeAxis->timeToPixel(mTimeAxis->reference())
            - UiTimeAxis::MajorStepPixelWidth*UiTimeAxis::ReferenceMajorStep;

    horizontalScrollBar()->setRange(minX, maxX - plotWidth);
    horizontalScrollBar()->setPageStep(plotWidth);
    horizontalScrollBar()->setValue(curr);
}

/*!
    Update the vertical scrollbar
*/
void UiPlot::updateVerticalScrollBar()
{
    int h = 0;
    int signalAreaHeight = 0;

    // calculate total height for all added signals
    foreach(UiAbstractSignal* s, mSignalManager->signalList()) {
        h += s->height();
    }

    signalAreaHeight = viewport()->height()-mTimeAxis->height()
            -UiCursor::CursorBarHeight;

    verticalScrollBar()->setRange(0, h-signalAreaHeight);
    verticalScrollBar()->setPageStep(viewport()->height());
}

void UiPlot::positionSignals(int offset)
{
    int yPos = offset+mTimeAxis->height();

    foreach(UiAbstractSignal* s, mSignalManager->signalList()) {
        if (!s->isVisible()) continue;

        s->move(0, yPos);
        yPos = s->pos().y() + s->height();
    }
}

/*!
    Map a position to a position relative to the cursor widget.
*/
QPoint UiPlot::mapToCursor(const QPoint &pos) const
{
    return QPoint(pos.x(), pos.y()-mCursor->pos().y());
}

/*!
    Get the last sample time.
*/
double UiPlot::getEndTime()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()->captureDevice();
    return (double)device->lastSampleIndex() / device->usedSampleRate();
}

/*!
    Update the UiPlot when a signal has been added to the signal manager.
*/
void UiPlot::handleSignalsAdded()
{
    foreach(UiAbstractSignal* signal, mSignalManager->signalList()) {

        // add the signal to this UiPlot (viewport) if not already added
        if (signal->parent() != viewport()) {

            signal->setParent(viewport());
            signal->setTimeAxis(mTimeAxis);
            signal->show();
            signal->resize(viewport()->width(), signal->height());

            // Make sure time axis/cursor bar is always on top among widget
            // siblings. The reason is to make sure the signals are put behind
            // the time axis/cursor bar during a vertical scroll
            mTimeAxis->raise();
            mCursor->raise();


            connect(signal, SIGNAL(sizeChanged()),
                    this, SLOT(updateLayout()));

        }
    }

    updateVerticalScrollBar();
    updateLayout();
}

/*!
    Update the UiPlot when a signal has been removed from the signal manager.
*/
void UiPlot::handleSignalsRemoved()
{
    updateVerticalScrollBar();
    updateLayout();
}
