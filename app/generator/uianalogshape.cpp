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
#include "uianalogshape.h"

#include <QDebug>
#include <qmath.h>

/*!
    \class UiAnalogShape
    \brief UI widget drawing a specific analog waveform.

    \ingroup Generator

*/

/*!
    Constructs the UiAnalogShape with the given \a parent.
*/
UiAnalogShape::UiAnalogShape(QWidget *parent) :
    QWidget(parent)
{
    mWaveform = AnalogSignal::WaveformSine;
}

/*!
    \fn AnalogSignal::AnalogWaveform UiAnalogShape::waveform()

    Returns the waveform set for this widget.
*/


/*!
    Set the waveform of this shape to \a form.
*/
void UiAnalogShape::setWaveform(AnalogSignal::AnalogWaveform form)
{
    mWaveform = form;
    update();
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiAnalogShape::paintEvent(QPaintEvent *event)
{
    (void)event;

    QPainter painter(this);

    paintGrid(&painter, width()-1, height()-1);

    painter.save();

    int margin = 3;
    painter.translate(margin, margin);
    int w = width()-1-2*margin;
    int h = height()-1-2*margin;


    switch(mWaveform) {
    case AnalogSignal::WaveformSine:
        paintSine(&painter, w, h);
        break;
    case AnalogSignal::WaveformSquare:
        paintSquare(&painter, w, h);
        break;
    case AnalogSignal::WaveformTriangle:
        paintTriangle(&painter, w, h);
        break;
    default:
        break;
    }

    painter.restore();
}

/*!
    Returns the recommended size for this widget.
*/
QSize UiAnalogShape::sizeHint() const
{
    QSize s(width(), height());

    return s;
}

/*!
    Paint grid part of this shape.
*/
void UiAnalogShape::paintGrid(QPainter* painter, int w, int h)
{
    painter->save();

    QPen pen = painter->pen();
    pen.setStyle(Qt::DashLine);
    painter->setPen(pen);

    painter->drawLine(0, h/2, w, h/2);
    painter->drawLine(w/2, 0, w/2, h);

    painter->restore();

}

/*!
    Paint a sine waveform.
*/
void UiAnalogShape::paintSine(QPainter* painter, int w, int h)
{

    double pi = 3.141592653589793;

    QPainterPath path;
    path.moveTo(0, h/2);

    for (int i = 0; i < w; i++) {
        double part = (double)i/(w-1);

        double y = (h/2) - (h/2)*qSin(2*pi*part);
        path.lineTo(i, y);
    }

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen = painter->pen();
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    painter->setPen(pen);

    painter->drawPath(path);

    painter->restore();

}

/*!
    Paint a square analog waveform.
*/
void UiAnalogShape::paintSquare(QPainter* painter, int w, int h)
{

    painter->save();

    QPen pen = painter->pen();
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    painter->setPen(pen);

    painter->drawLine(0, 0, w/2, 0);
    painter->drawLine(w/2, 0, w/2, h);
    painter->drawLine(w/2, h, w, h);

    painter->restore();

}

/*!
    Paint a triangle analog waveform.
*/
void UiAnalogShape::paintTriangle(QPainter* painter, int w, int h)
{

    painter->save();

    painter->setRenderHint(QPainter::Antialiasing);

    QPen pen = painter->pen();
    pen.setWidth(2);
    pen.setColor(Qt::blue);
    painter->setPen(pen);

    painter->drawLine(0, h/2, w/4, 0);
    painter->drawLine(w/4, 0, 3*w/4, h);
    painter->drawLine(3*w/4, h, w, h/2);

    painter->restore();
}
