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
#include "cursormanager.h"

/*!
    \class CursorManager
    \brief The cursor manager is responsible for creating and giving
    access to the cursor widget.

    \ingroup Capture

*/



/*!
    Constructs the cursor manager.
*/
CursorManager::CursorManager()
{
    mUiCursor = NULL;
}


/*!
    \fn CursorManager& CursorManager::instance()

    This function returns an instance of the CursorManager which is a
    singleton class.
*/

/*!
    Create the cursor widget with given \a signalManager, \a axis
    and \a parent.
*/
UiCursor* CursorManager::createUiCursor(SignalManager* signalManager,
                                        UiTimeAxis* axis, QWidget *parent)
{
    if (mUiCursor == NULL) {
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mUiCursor = new UiCursor(signalManager, axis, parent);
    }

    return mUiCursor;
}

/*!
    Returns a map with cursor IDs and cursor names. Only enabled
    cursors will be available in the list.
*/
QMap<UiCursor::CursorId, QString> CursorManager::activeCursors()
{
    if (mUiCursor != NULL) {
        return mUiCursor->activeCursors();
    }

    return QMap<UiCursor::CursorId, QString>();
}

/*!
    Return the time position for the cursor with ID \a id.
*/
double CursorManager::cursorPosition(UiCursor::CursorId id)
{
    if (mUiCursor == NULL || id >= UiCursor::NumCursors) return 0;

    return mUiCursor->cursorPosition(id);
}

/*!
   Set the time position \a pos for the cursor with ID \a id.
*/
void CursorManager::setCursorPosition(UiCursor::CursorId id, double pos)
{
    if (mUiCursor == NULL || id >= UiCursor::NumCursors) return;

    mUiCursor->setCursorPosition(id, pos);
}

/*!
    Return true if the cursor with ID \a id is enabled; otherwise
    false is returned.
*/
bool CursorManager::isCursorOn(UiCursor::CursorId id)
{
    if (mUiCursor == NULL) return false;

    return mUiCursor->isCursorOn(id);
}

/*!
    Set the enabled state of cursor \a id to \a enable.
*/
void CursorManager::enableCursor(UiCursor::CursorId id, bool enable)
{
    if (mUiCursor == NULL) return;

    mUiCursor->enableCursor(id, enable);
}
