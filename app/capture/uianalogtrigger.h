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
#ifndef UIANALOGTRIGGER_H
#define UIANALOGTRIGGER_H

#include <QWidget>
#include <QMouseEvent>
#include <QSlider>
#include <QLabel>

#include "device/analogsignal.h"

class UiAnalogTrigger : public QWidget
{
    Q_OBJECT
public:
    explicit UiAnalogTrigger(QWidget *parent = 0);
    AnalogSignal::AnalogTriggerState state();
    void setState(AnalogSignal::AnalogTriggerState state);
    double level();
    void setLevel(double level);
    void setVPerDiv(double vPerDiv);

signals:
    void triggerChanged();
    void levelChanged();
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void resizeEvent(QResizeEvent* event);

private slots:
    void setTriggerLevel(int level);

private:

    AnalogSignal::AnalogTriggerState mState;
    QSlider* mLevel;
    QLabel* mLevelLbl;
    double mVPerDiv;
    int mScale;

    void doLayout();
    
};

#endif // UIANALOGTRIGGER_H
