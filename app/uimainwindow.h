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
#ifndef UIMAINWINDOW_H
#define UIMAINWINDOW_H

#include <QMainWindow>
#include "device/device.h"

#include "generator/uigeneratorarea.h"
#include "generator/generatorapp.h"

#include "capture/captureapp.h"

class UiMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit UiMainWindow(QWidget *parent = 0);

    
signals:
    
public slots:

protected:
    void closeEvent(QCloseEvent * event);


private:

    enum AppState {
        AppStateCapture,
        AppStateGenerator,
        AppStateNum // must be last
    };

    QString mProjectFile;
    int mCaptureTabIdx;
    int mGeneratorTabIdx;
    AppState mAppState;

    GeneratorApp* mGenerator;
    CaptureApp* mCapture;

    QMenu* mDeviceMenu;
    QMenu* mColorSchemeMenu;

    QLabel* mDeviceLabel;

    void createMenubar();
    void createFileMenu();
    void createDeviceMenu();
    void createOptionMenu();
    void createHelpMenu();
    void updateMenu();

    void createToolbar();
    void createDeviceToolbar();
    void updateToolbar();

    void createCentralWidget();

    void changeToDevice(Device *device);
    void changeToScheme(QString &scheme);
    void saveSettings();
    void loadSettings();
    void openProject(QString projectFile);
    void saveProject(QString projectFile);


    void setActiveProjectFile(QString file);

    bool warnedAboutActiveState(QString action);

private slots:
    void handleTabChanged(int index);
    void changeDevice();
    void changeDeviceStatus(Device* device);
    void changeColorScheme();

    void newProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void about();
    
};

#endif // UIMAINWINDOW_H
