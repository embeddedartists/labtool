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
#ifndef CURSORMANAGER_H
#define CURSORMANAGER_H

#include <QObject>
#include "uicursor.h"

class CursorManager
{

public:

    static CursorManager& instance()
    {
        static CursorManager singleton;
        return singleton;
    }

    UiCursor* createUiCursor(SignalManager *signalManager, UiTimeAxis *axis, QWidget *parent);
    QMap<UiCursor::CursorId, QString> activeCursors();
    double cursorPosition(UiCursor::CursorId id);
    void setCursorPosition(UiCursor::CursorId id, double pos);
    bool isCursorOn(UiCursor::CursorId id);
    void enableCursor(UiCursor::CursorId id, bool enable);
    
signals:

public slots:

private:
    UiCursor* mUiCursor;

    explicit CursorManager();
    // hide copy constructor
    CursorManager(const CursorManager&);
    // hide assign operator
    CursorManager& operator=(const CursorManager &);
    
};

#endif // CURSORMANAGER_H
