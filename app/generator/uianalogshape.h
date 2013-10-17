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
#ifndef UIANALOGSHAPE_H
#define UIANALOGSHAPE_H

#include <QWidget>
#include <QPaintEvent>
#include <QPainter>

#include "device/analogsignal.h"

class UiAnalogShape : public QWidget
{
    Q_OBJECT
public:
    explicit UiAnalogShape(QWidget *parent = 0);

    AnalogSignal::AnalogWaveform waveform() {return mWaveform;}
    void setWaveform(AnalogSignal::AnalogWaveform form);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    QSize sizeHint() const;


private:
    AnalogSignal::AnalogWaveform mWaveform;

    void paintGrid(QPainter* painter, int w, int h);
    void paintSine(QPainter* painter, int w, int h);
    void paintSquare(QPainter* painter, int w, int h);
    void paintTriangle(QPainter* painter, int w, int h);
    
};

#endif // UIANALOGSHAPE_H
