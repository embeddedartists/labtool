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
#include "uigeneratorarea.h"

#include <QDebug>
#include <QScrollBar>

#include "uidigitalgenerator.h"
#include "uianaloggenerator.h"

#include "device/devicemanager.h"

/*!
    \class UiGeneratorArea
    \brief This class is the main UI widget for the generate part
    of this application.

    \ingroup Generator

    The user interface related to generate functionality is created and setup
    in this class.
*/

/*!
    \enum UiGeneratorArea::GeneratorType

    This enum describes the different generator types available.

    \var UiGeneratorArea::GeneratorType UiGeneratorArea::DigitalGenerator
    Digital signal generation

    \var UiGeneratorArea::GeneratorType UiGeneratorArea::AnalogGenerator
    Analog signal generation
*/


/*!
    Constructs the UiGeneratorArea with the given \a parent.
*/
UiGeneratorArea::UiGeneratorArea(QWidget *parent) :
    QMdiArea(parent)
{
    mHasBeenShown = false;

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mDigitalSignals = new DigitalSignals(this);

    // Deallocation: Destructor
    mDigitalWin = new QMdiSubWindow();
    mDigitalWin->setWindowFlags(mDigitalWin->windowFlags()
                                & ~(Qt::WindowMinimizeButtonHint|
                                    Qt::WindowMaximizeButtonHint));
    mDigitalWin->setWindowTitle(tr("Digital Signal Generator"));

    //
    // Deallocation:
    // -------------
    // QMdiSubWindow takes responsibility for UiDigitalGenerator when setWidget
    // is called. When mDigitalWin is delted UiDigitalGenerator will be deleted.
    //
    mDigitalGenerator = new UiDigitalGenerator(mDigitalSignals);
    mDigitalWin->setWidget(mDigitalGenerator);
    mDigitalWin->setWindowIcon(QIcon(":/resources/16_digital.png"));
    mDigitalWin->installEventFilter(this);

    // Deallocation: Destructor
    mAnalogWin = new QMdiSubWindow();
    mAnalogWin->setWindowFlags(mAnalogWin->windowFlags()
                                & ~(Qt::WindowMinimizeButtonHint|
                                    Qt::WindowMaximizeButtonHint));
    mAnalogWin->setWindowTitle(tr("Analog Signal Generator"));

    // Deallocation: See above for allocation of UiDigitalGenerator
    mAnalogGenerator = new UiAnalogGenerator();
    mAnalogWin->setWidget(mAnalogGenerator);
    mAnalogWin->setWindowIcon(QIcon(":/resources/16_analog.png"));
    mAnalogWin->installEventFilter(this);

    // Tiled windows
    // -------------
    // The subwindows (digital and analog generator) will in tiled state
    // take as much space as possible in the MDI window and be horizontally
    // aligned.
    //
    // When the subwindows are in tiled state they will be resized when the
    // MDI window is resized.
    //
    // Whenever a user moves or resizes a subwindow they will go out of tiled
    // state.
    //
    // - windows are tiled by default
    mIsSubWindowsTiled = true;

    mTileCalledFromResizeEvent = false;
}

/*!
    Deletes sub windows
*/
UiGeneratorArea::~UiGeneratorArea()
{
    delete mDigitalWin;
    delete mAnalogWin;
}

/*!
    Reimplemented from QObject::eventFilter. Filters events for the watched
    \a object.
*/
bool UiGeneratorArea::eventFilter(QObject *object, QEvent *event)
{
    QMdiSubWindow* win = qobject_cast<QMdiSubWindow*>(object);

    if (win != NULL) {

        switch(event->type()) {
        case QEvent::Move:
        case QEvent::Resize:

            // the widget hasn't been shown yet which means that it isn't
            // the user that is changing the size -> don't modify tile state.
            if (!mHasBeenShown) break;

            // the complete UiGeneratorArea is changing its size -> don't
            // modify tile state
            if (mTileCalledFromResizeEvent) break;

            mIsSubWindowsTiled = false;
            break;
        case QEvent::Close:
            mIsSubWindowsTiled = false;
            removeSubWindow(win);

            if (win == mDigitalWin) {
                emit generatorClosed(DigitalGenerator);
            }
            else if (win == mAnalogWin) {
                emit generatorClosed(AnalogGenerator);
            }

            break;
        default:
            break;
        }

    }

    return QMdiArea::eventFilter(object, event);
}

/*!
    Enable/disable the \a generator according to \a enabled.
*/
void UiGeneratorArea::setGeneratorEnabled(GeneratorType generator, bool enabled)
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    QMdiSubWindow* win;
    if (generator == AnalogGenerator) {
        win = mAnalogWin;
        device->enableAnalogGenerator(enabled);
    }
    else {
        win = mDigitalWin;
        device->enableDigitalGenerator(enabled);
    }

    bool added = (subWindowList().indexOf(win) != -1);

    if (enabled && !added) {
        addSubWindow(win);

        // for some reason the subwindow (win) can have a very small size
        // in some circumstances. Make sure it at least gets the same
        // size as its widget
        if (win->size().width() < win->widget()->size().width() ||
                win->size().height() < win->widget()->size().height()) {
            win->resize(win->widget()->size());
        }
        win->show();
        win->widget()->show();

    }
    else if (!enabled && added) {
        removeSubWindow(win);
    }
}

/*!
    Returns true if the \a generator is enabled; otherwise it returns false.
*/
bool UiGeneratorArea::isGeneratorEnabled(UiGeneratorArea::GeneratorType generator)
{
    QMdiSubWindow* win = mDigitalWin;
    if (generator == AnalogGenerator) {
        win = mAnalogWin;
    }

    return (subWindowList().indexOf(win) != -1);
}

/*!
    Returns the digital rate/frequency to use when generating digital signals.
*/
int UiGeneratorArea::digitalRate()
{
    return mDigitalGenerator->rate();
}

/*!
    Save project settings related to signal generation. The settings are
    stored in \a project.
*/
void UiGeneratorArea::saveProject(QSettings &project)
{
    GeneratorDevice* device = DeviceManager::instance()
            .activeDevice()->generatorDevice();

    project.remove("digitalGenerator");
    project.remove("analogGenerator");
    if (device != NULL) {

        if (device->maxNumDigitalSignals() > 0) {

            bool digEnabled = (subWindowList().indexOf(mDigitalWin) != -1);

            project.beginGroup("digitalGenerator");

            project.setValue("enabled", digEnabled);
            project.setValue("rate", mDigitalGenerator->rate());

            project.beginWriteArray("signals");

            QList<DigitalSignal*> digitalList = device->digitalSignals();
            for (int i = 0; i < digitalList.size(); i++) {
                DigitalSignal* s = digitalList.at(i);
                project.setArrayIndex(i);
                project.setValue("meta", s->toSettingsString());
            }

            project.endArray();

            project.endGroup();
        }

        if (device->maxNumAnalogSignals() > 0) {

            bool anEnabled  = (subWindowList().indexOf(mAnalogWin) != -1);

            project.beginGroup("analogGenerator");
            project.setValue("enabled", anEnabled);

            project.beginWriteArray("signals");

            QList<AnalogSignal*> analogList = device->analogSignals();
            for (int i = 0; i < analogList.size(); i++) {
                AnalogSignal* s = analogList.at(i);
                project.setArrayIndex(i);
                project.setValue("meta", s->toSettingsString());
            }

            project.endArray();

            project.endGroup();
        }

    }
}


/*!
    Load project settings related to signal generation. The settings are
    loaded from \a project.
*/
void UiGeneratorArea::openProject(QSettings &project)
{
    GeneratorDevice* device = DeviceManager::instance()
            .activeDevice()->generatorDevice();
    if (device == NULL) return;

    if (device->maxNumDigitalSignals() > 0) {
        project.beginGroup("digitalGenerator");

        bool enabled = project.value("enabled", true).toBool();
        setGeneratorEnabled(UiGeneratorArea::DigitalGenerator,
                            enabled);

        mDigitalGenerator->setRate(project.value("rate", 10000).toInt());

        mDigitalSignals->removeAllSignals();
        int numSignals = project.beginReadArray("signals");

        for (int i = 0; i < numSignals; i++) {
            project.setArrayIndex(i);
            QString meta = project.value("meta").toString();

            DigitalSignal tmp = DigitalSignal::fromSettingsString(meta);

            DigitalSignal* signal = mDigitalSignals->addSignal(tmp.id());
            if (signal == NULL) continue;
            *signal = tmp;

        }

        project.endArray();

        project.endGroup();
    }

    // analog signals

    if (device->maxNumAnalogSignals() > 0) {
        project.beginGroup("analogGenerator");

        bool enabled = project.value("enabled", true).toBool();
        setGeneratorEnabled(UiGeneratorArea::AnalogGenerator,
                            enabled);

        device->removeAllAnalogSignals();
        int numSignals = project.beginReadArray("signals");

        for (int i = 0; i < numSignals; i++) {
            project.setArrayIndex(i);
            QString meta = project.value("meta").toString();

            AnalogSignal tmp = AnalogSignal::fromSettingsString(meta);
            if (tmp.frequency() < device->minAnalogRate()
                    || tmp.frequency() > device->maxAnalogRate()) continue;
            if (tmp.amplitude() > device->maxAnalogAmplitude()) continue;

            AnalogSignal* signal = device->addAnalogSignal(tmp.id());

            if (signal == NULL) continue;

            *signal = tmp;
        }

        project.endArray();

        project.endGroup();
    }


    mDigitalGenerator->handleDeviceChanged();
    mAnalogGenerator->handleDeviceChanged();
}

/*!
    Reset the current project (and ui) to its default state (as if
    you were opening an empty project).
*/
void UiGeneratorArea::resetProject()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    // when a project is reset all added signals are removed

    mDigitalGenerator->removeAllSignals();
    mAnalogGenerator->removeAllSignals();
}


/*!
    \fn void UiGeneratorArea::generatorClosed(UiGeneratorArea::GeneratorType generator);

    This signal is emitted when a generator is closed.
*/


/*!
    Called when there is a new active device.
*/
void UiGeneratorArea::handleDeviceChanged()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if(device == NULL) return;

    setGeneratorEnabled(DigitalGenerator, (device->maxNumDigitalSignals() > 0));
    setGeneratorEnabled(AnalogGenerator, (device->maxNumAnalogSignals() > 0));
    tileSubWindows();

    mDigitalGenerator->handleDeviceChanged();
    mAnalogGenerator->handleDeviceChanged();
}

/*!
    Tile the windows. There is one window for digital signal generation
    and one window for analog signal generation.
*/
void UiGeneratorArea::tileSubWindows()
{
    QList<QMdiSubWindow*> windows = subWindowList();

    if (windows.size() == 0) return;

    int wHeight = (height()) / windows.size();

    int y = 0;
    for (int i = 0; i < windows.size(); i++) {
        QMdiSubWindow* win = windows.at(i);
        win->resize(width(), wHeight);
        win->move(0, y);
        y += wHeight;
    }

    mIsSubWindowsTiled = true;
}

/*!
    This event handler is called when the widget is resized.
*/
void UiGeneratorArea::resizeEvent(QResizeEvent* event)
{
    (void)event;
    if (mIsSubWindowsTiled) {
        mTileCalledFromResizeEvent = true;
        tileSubWindows();
        mTileCalledFromResizeEvent = false;
        mIsSubWindowsTiled = true;
    }
}

/*!
    This event handler is called when the widget is first shown.
*/
void UiGeneratorArea::showEvent(QShowEvent* event)
{
    (void)event;
    mHasBeenShown = true;
}


