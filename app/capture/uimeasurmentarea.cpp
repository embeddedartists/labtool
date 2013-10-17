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
#include "uimeasurmentarea.h"

#include <QDebug>

//
//    TODO: should probably implement this as a layout instead.
//


/*!
    \class UiMeasurmentArea
    \brief UI widget that contains measurement groups.

    \ingroup Capture

*/


/*!
    Constructs an UiMeasurmentArea with the given \a parent.
*/
UiMeasurmentArea::UiMeasurmentArea(QWidget *parent) :
    QWidget(parent)
{
}

/*!
    Add measurement group \a group to this widget.
*/
void UiMeasurmentArea::addMeasureGroup(QGroupBox* group)
{
    group->setParent(this);
    mGroupList.append(group);

    doLayout();
}

/*!
    This event handler is called when the widget is first made visible.
*/
void UiMeasurmentArea::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
}

/*!
    This resize event handler is called when the size of the widget is changed
*/
void UiMeasurmentArea::resizeEvent (QResizeEvent * event)
{
    (void)event;
    doLayout();
}

/*!
    Position the child widgets.
*/
void UiMeasurmentArea::doLayout()
{
    int yPos = 0;
    int minWidth = 0;

    for (int i = 0; i < mGroupList.size(); i++) {
        QGroupBox* group = mGroupList.at(i);
        group->move(0, yPos);

        yPos += group->minimumSizeHint().height();

        if (minWidth < group->minimumSizeHint().width()) {
            minWidth = group->minimumSizeHint().width();
        }
    }

    for (int i = 0; i < mGroupList.size(); i++) {
        QGroupBox* group = mGroupList.at(i);
        group->resize(minWidth, group->minimumSizeHint().height());
    }

    // the last group should fill the entire height of this area
    if (mGroupList.size() > 0) {
        QGroupBox* group = mGroupList.at(mGroupList.size()-1);
        int y = group->pos().y()+group->height();
        if (y < height()) {
            group->resize(group->width(), group->height()+height()-y-1);
        }
    }

    setMinimumWidth(minWidth);
}

