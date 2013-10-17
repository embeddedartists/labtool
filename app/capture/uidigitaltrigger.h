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
#ifndef UIDIGITALTRIGGER_H
#define UIDIGITALTRIGGER_H

#include <QWidget>
#include <QMouseEvent>

#include "device/digitalsignal.h"

class UiDigitalTrigger : public QWidget
{
    Q_OBJECT
public:
    explicit UiDigitalTrigger(QWidget *parent = 0);
    DigitalSignal::DigitalTriggerState state();
    void setState(DigitalSignal::DigitalTriggerState state);
    
signals:
    void triggerSet();
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent* event);

private:

    DigitalSignal::DigitalTriggerState mState;
};

#endif // UIDIGITALTRIGGER_H
