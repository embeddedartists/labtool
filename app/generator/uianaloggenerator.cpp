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
#include "uianaloggenerator.h"

#include <QMessageBox>
#include <QVBoxLayout>
#include <QMdiArea>
#include <QMdiSubWindow>

#include "uieditanalog.h"
#include "uigeneratorsignaldialog.h"

#include "device/devicemanager.h"


/*!
    \class UiAnalogGenerator
    \brief UI widget responsible for displaying and controlling
    generation for analog signals.

    \ingroup Generator

    Each analog signal will be shown in a separate window put in a MDI
    area.
*/

/*!
    Constructs the UiAnalogGenerator with the given \a parent.
*/
UiAnalogGenerator::UiAnalogGenerator(QWidget *parent) :
    QWidget(parent)
{
    // Deallocation: ownership changed when calling setLayout
    QVBoxLayout* verticalLayout = new QVBoxLayout();

    verticalLayout->addWidget(createToolBar());

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mWinArea = new QMdiArea(this);
    verticalLayout->addWidget(mWinArea);

    setLayout(verticalLayout);
}

/*!
    Remove all analog signals associated with this widget.
*/
void UiAnalogGenerator::removeAllSignals()
{
    QList<QMdiSubWindow*> list = mWinArea->subWindowList();
    foreach(QMdiSubWindow* win, list) {
        closeWindow(win, true);
    }
}

/*!
    Update the widget based on the new active device.
*/
void UiAnalogGenerator::handleDeviceChanged()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    // Begin by closing sub windows since we need to load signals
    // from the new device.

    // We cannot call removeAllSignals here since we would then delete
    // signals in the active device.

    QList<QMdiSubWindow*> list = mWinArea->subWindowList();
    foreach(QMdiSubWindow* win, list) {
        closeWindow(win, false);
    }

    QList<AnalogSignal*> analogSignals = device->analogSignals();
    foreach(AnalogSignal* s, analogSignals) {
        addSignal(s, device);
    }

    mAddAction->setEnabled(device->unusedAnalogIds().size() > 0);
}

/*!
    Reimplemented from QObject::eventFilter. Filters events for the watched
    \a object.
*/
bool UiAnalogGenerator::eventFilter(QObject *object, QEvent *event)
{
    QMdiSubWindow* win = qobject_cast<QMdiSubWindow*>(object);

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (win != NULL && device != NULL) {

        switch(event->type()) {

        case QEvent::Close:

            closeWindow(win, true);
            mAddAction->setEnabled(device->unusedAnalogIds().size() > 0);

            return true;
        default:
            break;
        }

    }

    return QWidget::eventFilter(object, event);
}

/*!
    Create a toolbar for this widget.
*/
QToolBar* UiAnalogGenerator::createToolBar()
{
    // Deallocation:
    //   Re-parented when calling verticalLayout->addWidget in constructor
    QToolBar* toolBar = new QToolBar("Analog generator settings");

    mAddAction = toolBar->addAction("Add");
    connect(mAddAction, SIGNAL(triggered()), this, SLOT(addSignal()));

    return toolBar;
}

/*!
    Add analog signal \a signal.
*/
void UiAnalogGenerator::addSignal(AnalogSignal* signal,
                                  GeneratorDevice* device)
{
    // Deallocation:
    //   Ownership transferred to QMdiSubWindow when calling setWidget
    UiEditAnalog* editAnalog = new UiEditAnalog(signal);

    // Deallocation: Deleted by closeWindow and, removeAllSignals. The call to
    //               mWinArea->addSubWindow will set the parent to the viewarea
    //               of mWinArea.
    QMdiSubWindow* win = new QMdiSubWindow();

    win->setWidget(editAnalog);
    mWinArea->addSubWindow(win);

    win->setWindowTitle(QString("Channel - A%1").arg(signal->id()));
    win->setWindowFlags(win->windowFlags()
                                & ~(Qt::WindowMinimizeButtonHint|
                                    Qt::WindowMaximizeButtonHint));
    win->installEventFilter(this);
    win->setWindowIcon(windowIcon());

    win->show();

    /*
        Issue: When clicking on the close button (X in top right corner)
               of the sub window we are getting a warning message ->
               "QMdiArea::removeSubWindow: window is not inside workspace"

               This message is printed before we call removeSubWindow
               ourselves in eventFilter() so it is not our call to
               removeSubWindow that triggers this. I don't know why this
               message is printed. Can it be because UiAnalogGenerator
               itself is a QMdiSubWindow placed in a QMdiArea?
     */

    mAddAction->setEnabled(device->unusedAnalogIds().size() > 0);
}

/*!
    Close a specific MDI window \a win (responsible for one analog signal).
    The parameter \a removeSignal is set to true if the signal associated
    with the window should be removed.
*/
void UiAnalogGenerator::closeWindow(QMdiSubWindow* win, bool removeSignal)
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    UiEditAnalog* editWidget = qobject_cast<UiEditAnalog*>(win->widget());
    if (editWidget == NULL || device == NULL) return;

    AnalogSignal* signal = editWidget->signal();

    editWidget->invalidateSignal();

    mWinArea->removeSubWindow(win);

    if (removeSignal) {
        device->removeAnalogSignal(signal);
    }

    // when deleting win the associated widget (editWidget) will also
    // be deleted since win has ownership of editWidget
    delete win;
}

/*!
    Called when user clicks the add button in the toolbar.
*/
void UiAnalogGenerator::addSignal()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    QMap<UiGeneratorSignalDialog::SignalType, QList<int> > map;
    map.insert(UiGeneratorSignalDialog::SignalAnalog,
               device->unusedAnalogIds());
    UiGeneratorSignalDialog dialog(map, this);
    int result = dialog.exec();

    if (result == QDialog::Accepted) {

        QList<int> ids = dialog.selectedSignals(
                    UiGeneratorSignalDialog::SignalAnalog);
        foreach(int id, ids) {
            AnalogSignal* s = device->addAnalogSignal(id);
            addSignal(s, device);
        }

    }



}




