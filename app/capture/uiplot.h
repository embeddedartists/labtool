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
#ifndef UIPLOT_H
#define UIPLOT_H

#include <QWidget>
#include <QWheelEvent>
#include <QAbstractScrollArea>
#include <QPoint>
#include <QPushButton>

#include "uigrid.h"
#include "uicursor.h"
#include "uitimeaxis.h"
#include "uiabstractsignal.h"


class UiPlot : public QAbstractScrollArea
{
    Q_OBJECT
public:
    explicit UiPlot(SignalManager* signalManager, QWidget *parent = 0);

    void zoom(int steps, int xCenter = -1);
    void zoomAll();

    void updateSignals();
    void handleSignalDataChanged();
    
signals:
   void cursorChanged(UiCursor::CursorId, bool, double);
    

protected:
    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent* event);
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void scrollContentsBy(int dx, int dy);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);



private:
    SignalManager* mSignalManager;
    UiTimeAxis* mTimeAxis;
    UiGrid* mGrid;
    UiCursor* mCursor;

    QPushButton* mAddSignalBtn;

    bool mDraggingPlot;
    QPoint mDragPlotPosition;

    bool mAboutToDragSignal;
    bool mDraggingSignal;
    QPoint mDragSignalPosition;
    UiAbstractSignal* mDragSignal;
    UiAbstractSignal* mBelowDragSignal;


    void updateHorizontalScrollBar();
    void updateVerticalScrollBar();
    void positionSignals(int offset);
    QPoint mapToCursor(const QPoint &pos) const;

    double getEndTime();    

private slots:
    void updateLayout();
    void handleSignalsAdded();
    void handleSignalsRemoved();

};

#endif // UIPLOT_H
