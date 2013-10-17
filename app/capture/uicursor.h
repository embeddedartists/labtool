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
#ifndef UICURSOR_H
#define UICURSOR_H

#include <QWidget>
#include <QMouseEvent>

#include "uiabstractplotitem.h"
#include "uitimeaxis.h"
#include "signalmanager.h"

class UiCursor : public UiAbstractPlotItem
{
    Q_OBJECT
public:

    enum Constants {
        CursorBarHeight = 30
    };

    enum CursorId {
        Trigger = 0,
        Cursor1 = 1,
        Cursor2 = 2,
        Cursor3 = 3,
        Cursor4 = 4,
        NoCursor,
        NumCursors = NoCursor
    };

    explicit UiCursor(SignalManager* signalManager, UiTimeAxis* axis, QWidget *parent);

    bool mousePressed(Qt::MouseButton button, const QPoint &pos);
    bool mouseReleased(Qt::MouseButton button,const QPoint &pos);
    bool mouseMoved(Qt::MouseButton, const QPoint &pos);

    void setTrigger(double t);

    double cursorPosition(UiCursor::CursorId id);
    void setCursorPosition(UiCursor::CursorId id, double t);
    bool isCursorOn(UiCursor::CursorId id);
    void enableCursor(UiCursor::CursorId id, bool enable);
    QMap<CursorId, QString> activeCursors();


    
signals:
    void cursorChanged(UiCursor::CursorId, bool, double);

public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:

    SignalManager* mSignalManager;
    UiTimeAxis *mTimeAxis;

    enum PrivConstants {
        CursorWidth = 8,
        CursorHeight = 8
    };

    bool mCursorOn[NumCursors];
    double mCursor[NumCursors];
    CursorId mCursorDrag;
    int mPressXPos;
    int mPressYPos;
    bool mMinWidthSet;

    
    CursorId findCursor(const QPoint &pos);
    void paintCursorSymbol(QPainter* painter, int cursorId);
    void paintCursors(QPainter* painter);
    int calcCursorYPosition(int cursorId);
    int calcCursorXPosition(int cursorId);

    void infoWidthChanged() {}
};

#endif // UICURSOR_H
