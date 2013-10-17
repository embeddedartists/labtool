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
#ifndef GENERATORAPP_H
#define GENERATORAPP_H

#include <QObject>
#include <QToolBar>
#include <QAction>
#include <QSettings>

#include "uigeneratorarea.h"

#include "device/device.h"

class GeneratorApp : public QObject
{
    Q_OBJECT
public:
    explicit GeneratorApp(QWidget* uiContext, QObject *parent = 0);

    QToolBar* toolBar() {return mToolBar;}
    UiGeneratorArea* generatorArea() {return mArea;}
    void handleDeviceChanged(Device* activeDevice);
    void handleDeviceStatusChanged(Device *device);

    void resetProject();
    void saveProject(QSettings &project);
    void openProject(QSettings &project);

    bool hasActiveState();
    
signals:
    
public slots:


private:

    enum GenerateState {
        GenerateStateStopped,
        GenerateStateOneShot,
        GenerateStateLoop
    };

    GenerateState mState;

    QWidget* mUiContext;
    QToolBar* mToolBar;
    UiGeneratorArea* mArea;
    QAction* mDigitalAction;
    QAction* mAnalogAction;

    QAction* mTbStartAction;
    QAction* mTbLoopAction;
    QAction* mTbStopAction;

    void createToolBar();
    void updateToolBar();

private slots:
    void setGeneratorEnabled(bool enabled);
    void handleGeneratorClosed(UiGeneratorArea::GeneratorType generator);
    void handleGenerateFinished(bool successful, QString msg);

    void start();
    void startLoop();
    void stop();

    void doStart(bool loop);

    void changeGenerateActions();
    
};

#endif // GENERATORAPP_H
