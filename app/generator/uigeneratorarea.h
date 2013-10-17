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
#ifndef UIGENERATORAREA_H
#define UIGENERATORAREA_H

#include <QMdiArea>
#include <QMdiSubWindow>
#include <QResizeEvent>
#include <QSettings>

#include "digitalsignals.h"
#include "uidigitalgenerator.h"
#include "uianaloggenerator.h"

#include "device/generatordevice.h"

class UiGeneratorArea : public QMdiArea
{
    Q_OBJECT
public:

    enum GeneratorType {
        DigitalGenerator,
        AnalogGenerator,
        NumGenerators // must be last
    };

    explicit UiGeneratorArea(QWidget *parent = 0);
    ~UiGeneratorArea();

    bool eventFilter(QObject *object, QEvent *event);
    void setGeneratorEnabled(GeneratorType generator, bool enabled);
    bool isGeneratorEnabled(UiGeneratorArea::GeneratorType generator);

    int digitalRate();
    void saveProject(QSettings &project);
    void openProject(QSettings &project);
    void resetProject();


    
signals:
    void generatorClosed(UiGeneratorArea::GeneratorType generator);
    
public slots:
    void handleDeviceChanged();
    void tileSubWindows();

protected:
    void resizeEvent(QResizeEvent* event);
    void showEvent(QShowEvent* event);

private:
    QMdiSubWindow* mDigitalWin;
    QMdiSubWindow* mAnalogWin;
    UiDigitalGenerator* mDigitalGenerator;
    UiAnalogGenerator* mAnalogGenerator;

    DigitalSignals* mDigitalSignals;

    bool mIsSubWindowsTiled;
    bool mTileCalledFromResizeEvent;
    bool mHasBeenShown;
    
};

#endif // UIGENERATORAREA_H
