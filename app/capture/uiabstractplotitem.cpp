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
#include "uiabstractplotitem.h"

#include <QDebug>

/*!
    \class UiAbstractPlotItem
    \brief UiAbstractPlotItem is the base class for all widgets that can be
        added to UiPlot.

    \ingroup Capture

*/




/*!
    Constructs the UiAbstractPlotItem with the given \a parent.
*/
UiAbstractPlotItem::UiAbstractPlotItem(QWidget *parent) :
    QWidget(parent)
{
    mMinimumInfoWidth = 0;
    mInfoWidth = 0;
}

/*!
    Sets the info width to \a width. All plot items have an info part which is
    separated from the plottable area. The info part is the left section of the
    item where, for example, the name, ID, and close button of a digital signal
    is painted.
*/
void UiAbstractPlotItem::setInfoWidth(int width)
{
    if (width < mMinimumInfoWidth) return;

    mInfoWidth = width;

    infoWidthChanged();
    update();
}

/*!
    Returns the info width of the plot item.

    \sa setInfoWidth()
*/
int UiAbstractPlotItem::infoWidth(void)
{
    return mInfoWidth;
}

/*!
    Returns the minimum info width for this plot item.

    \sa setInfoWidth()
*/
int UiAbstractPlotItem::minimumInfoWidth(void)
{
    return mMinimumInfoWidth;
}


/*!
    \fn virtual void UiAbstractPlotItem::infoWidthChanged() = 0

    Called to tell a sub class that the info width has changed.
*/

/*!
    Sets the minimum info width to \a width.

    \sa setInfoWidth()
*/
void UiAbstractPlotItem::setMinimumInfoWidth(int width)
{

    // set the info width if it is smaller than minimum width
    if (mInfoWidth < width) {
        mInfoWidth = width;
    }

    if (mMinimumInfoWidth != width) {
        mMinimumInfoWidth = width;

        emit sizeChanged();
    }

}

/*!
    Returns the width of the plottable area of the this plot item.
*/
int UiAbstractPlotItem::plotWidth()
{
    return width()-mInfoWidth;
}

/*!
    Returns the first x-coordinate of the plot area.
*/
int UiAbstractPlotItem::plotX()
{
    return mInfoWidth;
}

/*!
    \fn void UiAbstractPlotItem::sizeChanged()

    This signal is emitted when the size of the plot item is changed.
*/

/*!
    \var int UiAbstractPlotItem::mInfoWidth

    The info width of this plot item
*/

/*!
    \var int UiAbstractPlotItem::mMinimumInfoWidth

    The minimum info width of this plot item
*/

