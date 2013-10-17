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
#include "generatorapp.h"

#include "device/devicemanager.h"
#include "device/generatordevice.h"

/*!
    \class GeneratorApp
    \brief The GeneratorApp class is responsible for the generator part of this
           application.

    \ingroup Generator

    The GeneratorApp class is responsible for everything related to the
    generator part of the application, that is, generating digital and/or
    analog signals. It includes creating UI elements such as menu, toolbar,
    and main widget. It also includes project file handling (load, save)
    and issuing generate requests.
*/

/*!
    Constructs the GeneratorApp with the given \a parent and \a uiContext. The
    uiContext is used when showing dialog windows.
*/
GeneratorApp::GeneratorApp(QWidget* uiContext, QObject *parent) :
    QObject(parent)
{
    mUiContext = uiContext;
    mState = GenerateStateStopped;

    // Deallocation: uiContext is set as parent
    mArea = new UiGeneratorArea(uiContext);
    connect(mArea, SIGNAL(generatorClosed(UiGeneratorArea::GeneratorType)),
            this, SLOT(handleGeneratorClosed(UiGeneratorArea::GeneratorType)));

    createToolBar();

    QList<Device*> devices = DeviceManager::instance().devices();
    for (int i = 0; i < devices.size(); i++) {
        Device* device = devices.at(i);

        if (device->generatorDevice() != NULL) {
            connect(device->generatorDevice(),
                    SIGNAL(generateFinished(bool,QString)),
                    this, SLOT(handleGenerateFinished(bool,QString)));
        }

    }
}


/*!
    \fn QToolBar* GeneratorApp::toolBar()

    Returns the toolbar valid for the capture part of the application.
*/

/*!
    \fn UiGeneratorArea* GeneratorApp::generatorArea()

    Returns the main widget area for the generator part of the application.
*/


/*!
    Handle that the \a activeDevice has been set to the new
    active device.
*/
void GeneratorApp::handleDeviceChanged(Device* activeDevice)
{
    (void) activeDevice;
    mArea->handleDeviceChanged();
    updateToolBar();
}

/*!
    Handle that the status (availability) of \a device has changed.
*/
void GeneratorApp::handleDeviceStatusChanged(Device *device)
{
    if (!device->isAvailable()) {
        mState = GenerateStateStopped;
        changeGenerateActions();
    }
}

/*!
    Reset the current project (and ui) to its default state (as if
    you were opening an empty project).
*/
void GeneratorApp::resetProject()
{
    mArea->resetProject();
}

/*!
    Save the project settings that are related to the Generator
    part of the application. The settings are available in \a project.
*/
void GeneratorApp::saveProject(QSettings &project)
{
    mArea->saveProject(project);
}

/*!
    Open and load the project settings that are related to the Generator
    part of the application. The settings are available in \a project.
*/
void GeneratorApp::openProject(QSettings &project)
{
    mArea->openProject(project);


    // make sure generator icons in toolbar are correctly set

    if (!mArea->isGeneratorEnabled(UiGeneratorArea::DigitalGenerator)) {
        mDigitalAction->setChecked(false);
    }

    if (!mArea->isGeneratorEnabled(UiGeneratorArea::AnalogGenerator)) {
        mAnalogAction->setChecked(false);
    }
}

/*!
    Returns true if a signal generation is currently in process;
    otherwise false.
*/
bool GeneratorApp::hasActiveState()
{
    return (mState != GenerateStateStopped);
}

/*!
    Create capture toolbar.
*/
void GeneratorApp::createToolBar()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mToolBar = new QToolBar(tr("Generator toolbar"), mUiContext);


    mTbStartAction = mToolBar->addAction(QIcon(":/resources/16_start.png"),
                                         "Generate - one shot");
    connect(mTbStartAction, SIGNAL(triggered()), this, SLOT(start()));

    mTbLoopAction = mToolBar->addAction(QIcon(":/resources/16_recurring.png"),
                                        "Generate - loop");
    connect(mTbLoopAction, SIGNAL(triggered()), this, SLOT(startLoop()));

    mTbStopAction = mToolBar->addAction(QIcon(":/resources/16_stop.png"),
                                        "Stop");
    mTbStopAction->setEnabled(false);
    connect(mTbStopAction, SIGNAL(triggered()), this, SLOT(stop()));
    mToolBar->addSeparator();


    mDigitalAction = mToolBar->addAction(QIcon(":/resources/16_digital.png"),
                                         "Digital Generator");
    mDigitalAction->setObjectName("actionDigitalGenerator");
    mDigitalAction->setCheckable(true);
    connect(mDigitalAction, SIGNAL(triggered(bool)), this,
            SLOT(setGeneratorEnabled(bool)));

    mAnalogAction = mToolBar->addAction(QIcon(":/resources/16_analog.png"),
                                        "Analog Generator");
    mAnalogAction->setObjectName("actionAnalogGenerator");
    mAnalogAction->setCheckable(true);
    connect(mAnalogAction, SIGNAL(triggered(bool)), this,
            SLOT(setGeneratorEnabled(bool)));

    QAction* action = mToolBar->addAction(QIcon(":/resources/16_tile.png"),
                                          "Tile windows");
    connect(action, SIGNAL(triggered()), mArea, SLOT(tileSubWindows()));
}

/*!
    Update the toolbar based on current active device.
*/
void GeneratorApp::updateToolBar()
{
    GeneratorDevice* genDevice
            = DeviceManager::instance().activeDevice()->generatorDevice();
    if (genDevice != NULL) {
        mDigitalAction->setVisible(genDevice->maxNumDigitalSignals() > 0);
        mDigitalAction->setChecked(genDevice->isDigitalGeneratorEnabled());

        mAnalogAction->setVisible(genDevice->maxNumAnalogSignals() > 0);
        mAnalogAction->setChecked(genDevice->isAnalogGeneratorEnabled());
    }

}

/*!
    Enable/disable the generator according to \a enabled.
*/
void GeneratorApp::setGeneratorEnabled(bool enabled)
{
    UiGeneratorArea::GeneratorType generator
            = UiGeneratorArea::DigitalGenerator;
    QObject* o = QObject::sender();
    if (o == mAnalogAction) {
       generator =  UiGeneratorArea::AnalogGenerator;
    }

    mArea->setGeneratorEnabled(generator, enabled);
}

/*!
    Called when generator \a generator has been closed.
*/
void GeneratorApp::handleGeneratorClosed(
        UiGeneratorArea::GeneratorType generator)
{
    mArea->setGeneratorEnabled(generator, false);

    switch(generator) {
    case UiGeneratorArea::DigitalGenerator:
        mDigitalAction->setChecked(false);

        break;
    case UiGeneratorArea::AnalogGenerator:
        mAnalogAction->setChecked(false);
        break;
    default:
        break;
    }
}

/*!
    Called when a signal generation has finished. The parameter
    \a successful indicates if the generation succeeded or not. If it failed
    \a msg contains the error reason.
*/
void GeneratorApp::handleGenerateFinished(bool successful, QString msg)
{
    mState = GenerateStateStopped;
    changeGenerateActions();

    if (!successful) {
        QMessageBox::warning(mUiContext,
                             tr("Generate failed"),
                             msg);
    }
}

/*!
    Called when the user selects start in either the menu or on the toolbar.
*/
void GeneratorApp::start()
{
    doStart(false);
}

/*!
    Called when the user selects continuous in either the menu or on the
    toolbar.
*/
void GeneratorApp::startLoop()
{
    doStart(true);
}

/*!
    Called when the user selects stop in either the menu or on the toolbar.
*/
void GeneratorApp::stop()
{
    mState = GenerateStateStopped;
    changeGenerateActions();

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device != NULL) {
        device->stop();
    }
}

/*!
    Issue the start request to the active device.
*/
void GeneratorApp::doStart(bool loop)
{
    Device* device = DeviceManager::instance().activeDevice();
    QString msg = "";

    do {
        if (!device->isAvailable()) {
            msg = tr("Device is not available");
            break;
        }

        GeneratorDevice* genDevice = device->generatorDevice();
        if (genDevice == NULL) {
            msg = tr("Signal generation is not supported");
            break;
        }


        bool digitalEnabled = mDigitalAction->isChecked();
        bool analogEnabled = mAnalogAction->isChecked();

        if (!digitalEnabled && !analogEnabled) {
            msg = tr("Nothing to generate since all generators are disabled");
            break;
        }

        bool signalsAvailable =
                (digitalEnabled && genDevice->digitalSignals().size() > 0) ||
                (analogEnabled && genDevice->analogSignals().size() > 0);

        if (!signalsAvailable) {
            msg = tr("Nothing to generate since no signal data has been configured");
            break;
        }

        if(loop) {
            mState = GenerateStateLoop;
        }
        else {
            mState = GenerateStateOneShot;
        }

        changeGenerateActions();

        genDevice->start(mArea->digitalRate(), loop);

        return;

    } while (false);


    QMessageBox::warning(
                mUiContext,
                tr("Cannot start generation"),
                msg);

}


/*!
    Change the state of the start/stop action based on current state.
*/
void GeneratorApp::changeGenerateActions() {

    mTbStartAction->setEnabled(mState != GenerateStateOneShot);
    mTbLoopAction->setEnabled(mState != GenerateStateLoop);
    mTbStopAction->setEnabled(mState != GenerateStateStopped);

}


