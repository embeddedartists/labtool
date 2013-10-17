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
#ifndef UIANALOGGENERATOR_H
#define UIANALOGGENERATOR_H

#include <QWidget>
#include <QMdiArea>
#include <QToolBar>
#include <QAction>

#include "device/generatordevice.h"


class UiAnalogGenerator : public QWidget
{
    Q_OBJECT
public:
    explicit UiAnalogGenerator(QWidget *parent = 0);    

    void removeAllSignals();
    void handleDeviceChanged();
    bool eventFilter(QObject *object, QEvent *event);
    
signals:
    
public slots:

private:

    QMdiArea* mWinArea;

    QAction* mAddAction;

    QToolBar* createToolBar();
    void addSignal(AnalogSignal* signal, GeneratorDevice *device);
    void closeWindow(QMdiSubWindow* win, bool removeSignal);


private slots:
    void addSignal();


    
};

#endif // UIANALOGGENERATOR_H
