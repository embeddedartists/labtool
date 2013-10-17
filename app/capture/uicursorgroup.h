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
#ifndef UICURSORGROUP_H
#define UICURSORGROUP_H

#include <QGroupBox>
#include <QLabel>
#include "uicursor.h"

class UiCursorGroup : public QGroupBox
{
    Q_OBJECT
public:
    explicit UiCursorGroup(QWidget *parent = 0);
    
signals:
    
public slots:
     void setCursorData(UiCursor::CursorId cursor, bool enabled, double time);

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
        HoriDistBetweenRelated = 5,
        VertDistBetweenUnrelated = 7
    };

    /*
        'UiCursor::NumCursors-1' since the Trigger cursor is not
        included.
    */

    QLabel mCursorTimeLbl[UiCursor::NumCursors-1];
    QLabel mCursorTime[UiCursor::NumCursors-1];

    QLabel mCursorPairFreqLbl[(UiCursor::NumCursors-1)/2];
    QLabel mCursorPairFreq[(UiCursor::NumCursors-1)/2];

    QLabel mCursorPairTimeLbl[(UiCursor::NumCursors-1)/2];
    QLabel mCursorPairTime[(UiCursor::NumCursors-1)/2];

    double mCursorTimes[UiCursor::NumCursors-1];

    QSize mMinSize;

    void setupLabels();
    void doLayout();
    
};

#endif // UICURSORGROUP_H
