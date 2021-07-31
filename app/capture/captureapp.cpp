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
#include "captureapp.h"

#include <QComboBox>
#include <QFile>
#include <QDataStream>

#include "uiselectsignaldialog.h"
#include "cursormanager.h"
#include "uicaptureexporter.h"

#include "device/devicemanager.h"
#include "analyzer/analyzermanager.h"
#include "common/configuration.h"
#include "common/stringutil.h"

/*!
    \class CaptureApp
    \brief The CaptureApp class is responsible for the capture part of this
           application.

    \ingroup Capture

    The CaptureApp class is responsible for everything related to the capture part
    of the application, that is, capturing digital and/or analog signals. It includes
    creating UI elements such as menu, toolbar, and main widget. It also includes
    project file handling (load, save) and issuing capture requests.

*/

/*!
    Constructs the CaptureApp with the given \a parent and \a uiContext. The
    uiContext is used when showing dialog windows.
*/
CaptureApp::CaptureApp(QWidget* uiContext, QObject *parent) :
    QObject(parent)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mSignalManager = new SignalManager(this);

    mUiContext = uiContext;
    mCaptureActive = false;


    mContinuous = false;
    // Deallocation: uiContext is set as parent
    mArea = new UiCaptureArea(mSignalManager, uiContext);

    mMenu = NULL;

    createToolBar();
    createMenu();

    QList<Device*> devices = DeviceManager::instance().devices();
    for (int i = 0; i < devices.size(); i++) {
        Device* device = devices.at(i);

        if (device->captureDevice() != NULL) {
            connect(device->captureDevice(),
                    SIGNAL(captureFinished(bool,QString)),
                    this, SLOT(handleCaptureFinished(bool,QString)));
        }
    }


    mStreamingActive = false;
    captureStreamer = new UiCaptureStreamer(DeviceManager::instance().activeDevice()->captureDevice(), mUiContext);

}

/*!
    Deletes menu.
*/
CaptureApp::~CaptureApp()
{
    if (mMenu != NULL) {
        delete mMenu;
    }
}

/*!
    Return the main widget area for the capture part of the application.
*/
UiCaptureArea* CaptureApp::captureArea()
{
    return mArea;
}

/*!
    Return the toolbar valid for the capture part of the application.
*/
QToolBar* CaptureApp::toolBar()
{
    return mToolBar;
}

/*!
    Return the menu valid for the capture part of the application.
*/
QMenu* CaptureApp::menu()
{
    return mMenu;
}

/*!
    Reset the current project (and ui) to its default state (as if
    you were opening an empty project).
*/
void CaptureApp::resetProject()
{
    mSignalManager->closeAllSignals(true);

    Device* device = DeviceManager::instance().activeDevice();
    CaptureDevice* captureDevice = device->captureDevice();
    if (captureDevice != NULL) {
        captureDevice->clearSignalData();
    }
}

/*!
    Open and load the project settings that are related to the Capture
    part of the application. The settings are available in \a project.
*/
void CaptureApp::openProject(QSettings &project)
{
    QString projectFile = project.fileName();

    // open file used for signal data
    QString binDataFile = projectFile.replace(
                Configuration::ProjectFileExt,
                Configuration::ProjectBinFileExt);
    QFile file(binDataFile);
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    Device* device = DeviceManager::instance().activeDevice();
    CaptureDevice* captureDevice = device->captureDevice();
    if (captureDevice != NULL) {

        // load capture settings

        project.beginGroup("capture");

        int sampleRate = project.value("sampleRate", 1).toInt();
        int digTrigger = project.value("digitalTrigger", 0).toInt();

        captureDevice->setUsedSampleRate(sampleRate);
        setSampleRate(sampleRate);
        captureDevice->setDigitalTriggerIndex(digTrigger);

        // set the position for the trigger. This is needed since
        // when handleSignalDataChanged is called it will recalulate
        // the cursor positions relative to the trigger position
        // If the trigger position hasn't been set before there will
        // be an offset problem each time a project is loaded
        CursorManager::instance().setCursorPosition(UiCursor::Trigger,
                                                    (double)digTrigger/sampleRate);

        mSignalManager->loadSignalsFromSettings(project, in);

        // cursor positions

        int numCursors = project.beginReadArray("cursors");

        for (int i = 0; i < numCursors; i++) {

            project.setArrayIndex(i);
            QString meta = project.value("meta").toString();

            QStringList list = meta.split(";");

            if (list.size() != 3) continue;

            bool ok = false;
            int d = list.at(0).toInt(&ok);
            if (!ok) continue;
            if (d < 0 || d >= UiCursor::NumCursors) continue;
            UiCursor::CursorId id = (UiCursor::CursorId)d;

            // skip the Trigger since the position is retrieved from the
            // CaptureDevice)
            if (id == UiCursor::Trigger) continue;

            double pos = list.at(1).toDouble(&ok);
            if (!ok) continue;

            int on = list.at(2).toInt(&ok);
            if (!ok) continue;
            bool enable = (on == 1);

            CursorManager::instance().setCursorPosition(id, pos);
            CursorManager::instance().enableCursor(id, enable);


        }
        project.endArray();

        project.endGroup();

        // no capture settings available -> setup default
        QStringList groups = project.childGroups();
        if (!groups.contains("capture")) {
            resetProject();
        }

    }

    mArea->handleSignalDataChanged();
}

/*!
    Save the project settings that are related to the Capture
    part of the application. The settings are available in \a project.
*/
void CaptureApp::saveProject(QSettings &project)
{
    QString projectFile = project.fileName();

    CaptureDevice* captureDevice = DeviceManager::instance()
            .activeDevice()->captureDevice();

    if (captureDevice != NULL) {

        // open file and stream to be used for signal data
        QString binDataFile = projectFile.replace(
                    Configuration::ProjectFileExt,
                    Configuration::ProjectBinFileExt);

        QFile file(binDataFile);
        file.open(QIODevice::WriteOnly);
        QDataStream out(&file);

        project.remove("capture");
        project.beginGroup("capture");

        project.setValue("sampleRate", captureDevice->usedSampleRate());
        project.setValue("digitalTrigger",
                         captureDevice->digitalTriggerIndex());
        mSignalManager->saveSignalSettings(project, out);

        // save cursor positions
        project.beginWriteArray("cursors");
        int idx = 0;
        for (int i = 0; i < UiCursor::NumCursors; i++) {

            // skip the Trigger since the position is retrieved from the
            // CaptureDevice (above)
            if (i == UiCursor::Trigger) continue;

            project.setArrayIndex(idx++);
            project.setValue("meta", QString("%1;%2;%3")
                             .arg(i)
                             .arg(CursorManager::instance().cursorPosition((UiCursor::CursorId)i))
                             .arg(CursorManager::instance().isCursorOn((UiCursor::CursorId)i)));

        }
        project.endArray();
        project.endGroup();
        file.close();
    }
}

/*!
    Handle that the \a activeDevice has been set to the new
    active device.
*/
void CaptureApp::handleDeviceChanged(Device* activeDevice)
{
    // recreate captureStreamer (even if not running)
    delete captureStreamer;
    captureStreamer = new UiCaptureStreamer(activeDevice->captureDevice(), mUiContext);
    // update UI to follow up
    // this will make it end up in the "stopped" UI state
    mStreamingActive = true;
    streamData();

    setupRates(activeDevice->captureDevice());
    mSignalManager->reloadSignalsFromDevice();
    mArea->updateAnalogGroup();
}

/*!
    Handle that the status (availability) of \a device has changed.
*/
void CaptureApp::handleDeviceStatusChanged(Device *device)
{
    if(!device->isAvailable()) {
        // if device is no longer available make sure continuous mode is
        // cancelled.
        mContinuous = false;

        // if no longer available make sure capture actions are reset
        changeCaptureActions(false);

    }
}

/*!
    Updates/redraws the UI.
*/
void CaptureApp::updateUi()
{
    mArea->updateUi();
}

/*!
    \fn bool CaptureApp::hasActiveState()

    Returns true if a capture is currently in process; otherwise false.
*/

/*
    ---------------------------------------------------------------------------
    #### Private methods
    ---------------------------------------------------------------------------
*/


/*!
    Create capture toolbar.
*/
void CaptureApp::createToolBar()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mToolBar = new QToolBar(tr("Capture toolbar"), mUiContext);

    // Deallocation: mToolBar takes ownership when calling addWidget
    mRateBox = new QComboBox();
    mRateBox->setToolTip("Selected sample rate");
    connect(mRateBox, SIGNAL(currentIndexChanged(int)), this,
            SLOT(sampleRateChanged(int)));

    // Deallocation: mToolBar takes ownership when calling addWidget
    mToolBar->addWidget(new QLabel(tr("Sample Rate ")));
    mToolBar->addWidget(mRateBox);
    mToolBar->addSeparator();


    mTbStartAction = mToolBar->addAction(QIcon(":/resources/16_start.png"),
                                         "Capture");
    connect(mTbStartAction, SIGNAL(triggered()), this, SLOT(start()));

    mTbContinuousAction = mToolBar->addAction(
                QIcon(":/resources/16_recurring.png"), "Continuous capture");
    connect(mTbContinuousAction, SIGNAL(triggered()), this,
            SLOT(startContinuous()));

    mTbStopAction = mToolBar->addAction(QIcon(":/resources/16_stop.png"),
                                        "Stop");
    mTbStopAction->setEnabled(false);
    connect(mTbStopAction, SIGNAL(triggered()), this, SLOT(stop()));
    mToolBar->addSeparator();

    QAction* action = mToolBar->addAction(QIcon(":/resources/16_zoom_in.png"),
                                          "Zoom In");
    action->setData("Zoom In");
    connect(action, SIGNAL(triggered()), mArea, SLOT(zoomIn()));

    action = mToolBar->addAction(QIcon(":/resources/16_zoom_out.png"),
                                 "Zoom Out");
    action->setData("Zoom Out");
    connect(action, SIGNAL(triggered()), mArea, SLOT(zoomOut()));

    action = mToolBar->addAction(QIcon(":/resources/16_zoom_all.png"),
                                 "Zoom All");
    action->setData("Zoom All");
    connect(action, SIGNAL(triggered()), mArea, SLOT(zoomAll()));
    mToolBar->addSeparator();

    action = mToolBar->addAction("Add Signal");
    connect(action, SIGNAL(triggered()), this, SLOT(selectSignalsToAdd()));
}

/*!
    Create capture related menu.
*/
void CaptureApp::createMenu()
{
    // Deallocation: Destructor
    //   Ownership doesn't seem to be transferred to the menu bar when addMenu is
    //   called by UiMainWindow.
    mMenu = new QMenu(tr("&Capture"));
    mMenu->setObjectName(QString::fromUtf8("captureMenu"));

    //
    //  Start capture
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mMenuStartAction = new QAction(tr("Start"), this);
    mMenuStartAction->setToolTip(tr("Start capture"));
    connect(mMenuStartAction, SIGNAL(triggered()), this, SLOT(start()));
    mMenu->addAction(mMenuStartAction);

    //
    //  Continuous capture
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mMenuContinuousAction = new QAction(tr("Continuous"), this);
    mMenuContinuousAction->setToolTip(tr("Continuous capture"));
    connect(mMenuContinuousAction, SIGNAL(triggered()),
            this, SLOT(startContinuous()));
    mMenu->addAction(mMenuContinuousAction);

    //
    //  Stop capture
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mMenuStopAction = new QAction(tr("Stop"), this);
    mMenuStopAction->setToolTip(tr("Stop capture"));
    mMenuStopAction->setDisabled(true);
    connect(mMenuStopAction, SIGNAL(triggered()), this, SLOT(stop()));
    mMenu->addAction(mMenuStopAction);

    mMenu->addSeparator();

    //
    //    Trigger settings
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QAction* action = new QAction(tr("Trigger settings"), this);
    action->setData("Trigger settings");
    action->setToolTip("Change trigger settings");
    connect(action, SIGNAL(triggered()), this, SLOT(triggerSettings()));
    mMenu->addAction(action);

    mMenu->addSeparator();

    //
    //    Calibration settings
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    action = new QAction(tr("Calibrate Hardware"), this);
    action->setData("Calibrate Hardware");
    action->setToolTip("(Re)Calibrate the Hardware");
    connect(action, SIGNAL(triggered()), this, SLOT(calibrationSettings()));
    mMenu->addAction(action);

    //
    //    Export Data
    //

    mMenu->addSeparator();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    action = new QAction(tr("Export Data"), this);
    action->setData("Export Data");
    action->setToolTip("Export captured signal data to file");
    connect(action, SIGNAL(triggered()), this, SLOT(exportData()));
    mMenu->addAction(action);


    //
    //    Set Up Streaming via Network
    //

    mStreamAction = new QAction(tr("Stream Data to Socket"), this);
    mStreamAction->setData("Stream Data to Socket");
    mStreamAction->setToolTip("Open a socket and send the currently captured data there");
    connect(mStreamAction, SIGNAL(triggered()), this, SLOT(streamData()));
    mMenu->addAction(mStreamAction);


}

/*!
    Change UI elements (menu, toolbar) to show that a capture is
    active/inactive as specified by \a captureActive.
*/
void CaptureApp::changeCaptureActions(bool captureActive) {

    mCaptureActive = captureActive;

    if (captureActive) {
        mMenuStartAction->setEnabled(mContinuous);
        mTbStartAction->setEnabled(mContinuous);
        mMenuContinuousAction->setEnabled(!mContinuous);
        mTbContinuousAction->setEnabled(!mContinuous);

        mMenuStopAction->setEnabled(true);
        mTbStopAction->setEnabled(true);
    } else {
        mMenuStartAction->setEnabled(true);
        mTbStartAction->setEnabled(true);
        mMenuContinuousAction->setEnabled(true);
        mTbContinuousAction->setEnabled(true);

        mMenuStopAction->setEnabled(false);
        mTbStopAction->setEnabled(false);
    }

}

/*!
    Request the capture device to start a capture based on the current
    configuration.
*/
void CaptureApp::doStart()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    device->configureBeforeStart(mUiContext);
    int rate = mRateBox->itemData(mRateBox->currentIndex()).toInt();
    device->start(rate);
}

/*!
    Setup the sample rates valid for the given \a device.
*/
void CaptureApp::setupRates(CaptureDevice *device)
{
    if (device != NULL) {
        mRateBox->clear();

        QList<int> rates = device->supportedSampleRates();
        for (int i = 0; i < rates.size(); i++) {
            mRateBox->addItem(StringUtil::frequencyToString(rates.at(i)),
                              QVariant(rates.at(i)));
        }
    }

}

/*!
    Set the selected sample rate given by \a rate.
*/
void CaptureApp::setSampleRate(int rate)
{

    for (int i = 0; i < mRateBox->count(); i++) {

        if (mRateBox->itemData(i).toInt() == rate) {
            mRateBox->setCurrentIndex(i);
        }
    }

}

/*!
    Called when the user selects start in either the menu or on the toolbar.
*/
void CaptureApp::start()
{

    Device* device = DeviceManager::instance().activeDevice();

    if (device != NULL && device->isAvailable()
            && device->supportsCaptureDevice()) {

        changeCaptureActions(true);

        if (mContinuous) {
            stop();
        }

        doStart();
    }
    else {
        QString msg = tr("The device is not available");
        if (device != NULL && !device->supportsCaptureDevice()) {
            msg = tr("Capture is not supported");
        }
        QMessageBox::warning(
                    mUiContext,
                    tr("Action not supported"),
                    msg);
    }

}

/*!
    Called when the user selects continuous in either the menu or on the
    toolbar.
*/
void CaptureApp::startContinuous()
{
    Device* device = DeviceManager::instance().activeDevice();
    if (device != NULL && device->isAvailable()) {
        mContinuous = true;
        changeCaptureActions(true);

        doStart();
    }
    else {
        QString msg = tr("The device is not available");
        if (device != NULL && !device->supportsCaptureDevice()) {
            msg = tr("Capture is not supported");
        }
        QMessageBox::warning(
                    mUiContext,
                    tr("Action not supported"),
                    msg);
    }
}

/*!
    Called when the user selects stop in either the menu or on the toolbar.
*/
void CaptureApp::stop()
{
    mContinuous = false;
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device != NULL) {
        device->stop();
    }
}

/*!
    Handles that a capture request has finished. The status of the
    request is specified by \a successful and any error message is
    given by \a msg.
*/
void CaptureApp::handleCaptureFinished(bool successful, QString  msg)
{

    if (!mContinuous || !successful) {
        changeCaptureActions(false);
    }

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device != NULL) {

        if (successful) {
            mArea->handleSignalDataChanged();

            if (mContinuous && device->supportsContinuousCapture()) {
                doStart();
            }
        }
        else {
            // always make sure continuous mode is reset if capture fails.
            mContinuous = false;

            QMessageBox::warning(mUiContext,
                                 tr("Capture Failed"),
                                 msg);

        }

    }

}

/*!
    Called when the user selects to change trigger settings.
*/
void CaptureApp::triggerSettings()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    if (device != NULL) {
        device->configureTrigger(mUiContext);
    }
}

/*!
    Called when the user selects to calibrate the hardware.
*/
void CaptureApp::calibrationSettings()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    if (device != NULL) {
        device->calibrate(mUiContext);
    }
}

/*!
    Called when the user selects to enable more signals.
*/
void CaptureApp::selectSignalsToAdd()
{

    UiSelectSignalDialog dialog(mUiContext);
    int result = dialog.exec();

    if (result == QDialog::Accepted) {

        UiAnalyzer* analyzer = AnalyzerManager::createAnalyzer(
                    dialog.selectedAnalyzer());
        if (analyzer != NULL) {

            analyzer->configure(mUiContext);
            analyzer->analyze();
            mSignalManager->addAnalyzer(analyzer);
        }

        QList<int> digitalIds = dialog.selectedDigitalSignals();
        foreach(int id, digitalIds) {
            mSignalManager->addDigitalSignal(id);
        }

        QList<int> analogIds = dialog.selectedAnalogSignals();
        foreach(int id, analogIds) {
            mSignalManager->addAnalogSignal(id);
        }

    }

}

/*!
    Called when the user selects to export data.
*/
void CaptureApp::exportData()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device == NULL) return;

    QList<DigitalSignal*> digitalSignals = device->digitalSignals();
    QList<AnalogSignal*> analogSignals = device->analogSignals();

    // check if there is data to export
    do {
        bool dataToExport = false;

        foreach(DigitalSignal* s, digitalSignals) {
            QVector<int>* d = device->digitalData(s->id());
            if (d != NULL && d->size() > 0) {
                dataToExport = true;
                break;
            }
        }

        if (dataToExport) break;

        foreach(AnalogSignal* s, analogSignals) {
            QVector<double>* d = device->analogData(s->id());
            if (d != NULL && d->size() > 0) {
                dataToExport = true;
                break;
            }
        }

        if (dataToExport) break;

        QMessageBox::warning(mUiContext,
                             "No data to export",
                             "There is no data to export!");

        return;


    } while(false);

    UiCaptureExporter exporter(device, mUiContext);
    exporter.exec();

}

/*!
    Called when the user selects to stream data to socket, adapted from exportData
*/
void CaptureApp::streamData()
{
    if(mStreamingActive) {
        // currently streaming, so stop now
        emit captureStreamer->stopWorker();
        mStreamingActive = false;
        mStreamAction->setText(tr("Stream Data to Socket"));
        mStreamAction->setData("Stream Data to Socket");

    } else {
        // currently not streaming, so (try to) start now

        // checks before streaming
        CaptureDevice* device = DeviceManager::instance().activeDevice()
                ->captureDevice();
        if (device == NULL) {
            return;
        }
        // check if there is data to stream
        if(device->digitalSignals().empty() && device->analogSignals().empty()) {
            QMessageBox::warning(mUiContext,
                                 "No signal found",
                                 "Please add at least one signal!");
            return;
        }


        if(captureStreamer->exec() != QDialog::Accepted) {
            // not accepted, abort
            return;
        }

        mStreamingActive = true;
        mStreamAction->setText(tr("Stop Streaming"));
        mStreamAction->setData("Stop Streaming");
    }

}

/*!
    Called when the sample rate has changed.
*/
void CaptureApp::sampleRateChanged(int rateIndex)
{
    int rate = mRateBox->itemData(rateIndex).toInt();
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device != NULL) {
        device->reconfigure(rate);
    }
}
