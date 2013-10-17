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
#ifndef CAPTUREAPP_H
#define CAPTUREAPP_H

#include <QObject>
#include <QToolBar>
#include <QMenu>
#include <QAction>
#include <QComboBox>
#include <QSettings>

#include "uicapturearea.h"
#include "device/device.h"

class CaptureApp : public QObject
{
    Q_OBJECT
public:
    explicit CaptureApp(QWidget* uiContext, QObject *parent = 0);
    ~CaptureApp();

    UiCaptureArea* captureArea();
    QToolBar* toolBar();
    QMenu* menu();

    void resetProject();
    void openProject(QSettings &project);
    void saveProject(QSettings &project);

    void handleDeviceChanged(Device* activeDevice);
    void handleDeviceStatusChanged(Device *device);
    void updateUi();

    bool hasActiveState() {return mCaptureActive;}
    
signals:
    
public slots:

private:
    SignalManager* mSignalManager;
    QWidget* mUiContext;
    QToolBar* mToolBar;
    QMenu* mMenu;
    UiCaptureArea* mArea;
    bool mContinuous;

    QAction* mMenuStartAction;
    QAction* mMenuContinuousAction;
    QAction* mMenuStopAction;
    QAction* mTbStartAction;
    QAction* mTbContinuousAction;
    QAction* mTbStopAction;

    QComboBox* mRateBox;

    bool mCaptureActive;

    void createToolBar();
    void createMenu();
    void changeCaptureActions(bool captureActive);
    void doStart();
    void setupRates(CaptureDevice* device);
    void setSampleRate(int rate);


private slots:
    void start();
    void startContinuous();
    void stop();
    void handleCaptureFinished(bool successful, QString msg);
    void triggerSettings();
    void calibrationSettings();
    void selectSignalsToAdd();
    void exportData();
    void sampleRateChanged(int rateIndex);

    
};

#endif // CAPTUREAPP_H
