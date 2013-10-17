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
#include "uicapturearea.h"

#include <QHBoxLayout>

#include "uimeasurmentarea.h"
#include "uicursorgroup.h"
#include "uidigitalgroup.h"

#include "signalmanager.h"

#include "device/devicemanager.h"
#include "device/capturedevice.h"

/*!
    \class UiCaptureArea
    \brief The UiCaptureArea class is the main UI widget for the capture part
    of this application.

    \ingroup Capture

    The user interface related to capture functionality is created and setup in
    this class.
*/

/*!
    Constructs the UiCaptureArea with the given \a parent. The signal manager
    given by \a signalManager is used to keep track of signal widgets.
*/
UiCaptureArea::UiCaptureArea(SignalManager *signalManager, QWidget *parent) :
    QWidget(parent)
{    
    mSignalManager = signalManager;

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QHBoxLayout* centralLayout = new QHBoxLayout(this);

    //
    //    UiPlot
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mPlot = new UiPlot(mSignalManager, this);
    centralLayout->addWidget(mPlot);


    //
    //    Measurement Area
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    UiMeasurmentArea* measureArea = new UiMeasurmentArea(this);
    centralLayout->addWidget(measureArea);

    //
    //    Cursor measurments
    //

    // Deallocation: measureArea takes ownership of cursor group
    UiCursorGroup* cg = new UiCursorGroup();
    measureArea->addMeasureGroup(cg);

    connect((mPlot),
            SIGNAL(cursorChanged(UiCursor::CursorId, bool, double)),
            cg,
            SLOT(setCursorData(UiCursor::CursorId, bool, double)));


    // Deallocation: measureArea takes ownership of digital group
    UiDigitalGroup* dg = new UiDigitalGroup();
    measureArea->addMeasureGroup(dg);

    connect(mSignalManager,
            SIGNAL(digitalMeasurmentChanged(double,double,double,bool,bool)),
            dg,
            SLOT(setCycleData(double,double,double,bool,bool)));

    // Deallocation: measureArea takes ownership of analog group
    mAnalogGroup = new UiAnalogGroup();
    measureArea->addMeasureGroup(mAnalogGroup);

    connect(mSignalManager,
            SIGNAL(analogMeasurmentChanged(QList<double>,QList<double>,bool)),
            mAnalogGroup,
            SLOT(setMeasurementData(QList<double>,QList<double>,bool)));

}

/*!
    Must be called when signal data has changed. Will make sure relevant
    UI elements are updated.
*/
void UiCaptureArea::handleSignalDataChanged()
{
    // make sure analyzers are updated
    foreach(UiAbstractSignal* s, mSignalManager->signalList()) {
        UiAnalyzer* as = qobject_cast<UiAnalyzer*>(s);
        if (as != NULL) {
            as->analyze();
        }
    }

    mPlot->handleSignalDataChanged();
}

/*!
    Issue an update request to UI elements to make sure they are redrawn.
*/
void UiCaptureArea::updateUi()
{
    mPlot->viewport()->update();
}

/*!
    Updates the state of the analog group. If analog signals aren't supported
    by the active capture device the group will be set to invisible.
*/
void UiCaptureArea::updateAnalogGroup()
{
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (device == NULL) return;

    mAnalogGroup->setNumSignals(device->maxNumAnalogSignals());

    if (device->maxNumAnalogSignals() == 0) {
        mAnalogGroup->setVisible(false);
    }
    else {
        mAnalogGroup->setVisible(true);
    }

}

/*!
    Request to zoom in the UI plot of signals
*/
void UiCaptureArea::zoomIn()
{
    mPlot->zoom(1);
}

/*!
    Request to zoom out the UI plot of signals
*/
void UiCaptureArea::zoomOut()
{
    mPlot->zoom(-1);
}

/*!
    Request to zoom the UI plot to a level where all signals are visible
*/
void UiCaptureArea::zoomAll()
{
    mPlot->zoomAll();
}
