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
#include "uigrid.h"

#include <QPainter>
#include <QDebug>


/*!
    \class UiGrid
    \brief UI widget that paints the grid ontop of the plot.

    \ingroup Capture

*/


/*!
    Constructs an UiGrid with the given time axis \a axis
    and \a parent.
*/
UiGrid::UiGrid(UiTimeAxis *axis, QWidget *parent) :
    UiAbstractPlotItem(parent)
{
    mTimeAxis = axis;
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiGrid::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);

    painter.save();
    painter.translate(infoWidth(), 0);

    // draw grid for each main step
    int numMajorSteps = width() / UiTimeAxis::MajorStepPixelWidth + 1;

    for (int i = 0; i < numMajorSteps; i++) {

        QPen pen = painter.pen();
        if (i == UiTimeAxis::ReferenceMajorStep) {
            pen.setColor(Qt::black);
            pen.setStyle(Qt::DashLine);
        } else {
            pen.setColor(Qt::gray);
            pen.setStyle(Qt::DotLine);
        }
        painter.setPen(pen);

        painter.drawLine(i*UiTimeAxis::MajorStepPixelWidth,
                          2,
                          i*UiTimeAxis::MajorStepPixelWidth,
                          height());

    }

    painter.restore();
}
