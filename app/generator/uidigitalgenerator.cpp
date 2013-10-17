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
#include "uidigitalgenerator.h"

#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QToolBar>
#include <QLineEdit>
#include <QLabel>
#include <QAction>

#include "digitalsignals.h"
#include "digitaldelegate.h"
#include "uigeneratorsignaldialog.h"

#include "device/digitalsignal.h"
#include "device/devicemanager.h"
#include "common/stringutil.h"

/*!
    \class UiDigitalGenerator
    \brief UI widget responsible for displaying and controlling
    generation for digital signals.

    \ingroup Generator

    Each digital signal will be shown as a row in a table.
*/

/*!
    Constructs the UiDigitalGenerator with the given \a parent.
*/
UiDigitalGenerator::UiDigitalGenerator(DigitalSignals* digitalSignals,
                                       QWidget *parent) :
    QWidget(parent)
{
    int defaultStates = 32;
    mSignals = digitalSignals;

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device != NULL) {
        defaultStates = device->maxNumDigitalStates();
    }

    // Deallocation: ownership changed when calling setLayout
    QVBoxLayout* verticalLayout = new QVBoxLayout();

    mTable = createTable();

    verticalLayout->addWidget(createToolBar());
    verticalLayout->addWidget(mTable);

    setLayout(verticalLayout);


    setNumStates(defaultStates);
}

/*!
    Returns the rate/frequency to use when generating digital signals.
*/
int UiDigitalGenerator::rate()
{
    QString rate = mRate->text();
    return StringUtil::frequencyToInt(rate);
}

/*!
    Set the rate/frequency to \a rate.
*/
void UiDigitalGenerator::setRate(int rate)
{
    mRate->setText(StringUtil::frequencyToString(rate));
}

/*!
    Remove all digital signals.
*/
void UiDigitalGenerator::removeAllSignals()
{
    mSignals->removeAllSignals();
}

/*!
    Set number of states for all digital signals.
*/
void UiDigitalGenerator::setNumStates(int states)
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (states <= 0 || states > device->maxNumDigitalStates()) return;

    mSignals->setNumStates(states);

    // if this function is called directly (not as a signal from mStatesBox)
    // we have to also update mStatesBox (the spin box)
    QObject* o = sender();
    if (o != mStatesBox) {
        mStatesBox->setValue(states);
    }

    // make sure the columns are resized, but keep the size of column 0
    int c0Width = mTable->columnWidth(0);
    mTable->resizeColumnsToContents();
    mTable->setColumnWidth(0, c0Width);
}

/*!
    Called when the active device has been changed.
*/
void UiDigitalGenerator::handleDeviceChanged()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice
            ()->generatorDevice();
    if (device == NULL) return;

    // ### maximum number of states

    mStatesBox->setMaximum(device->maxNumDigitalStates());

    QList<DigitalSignal*> digitalSignals = device->digitalSignals();

    // ### selected number of states

    int states = device->maxNumDigitalStates();
    if (digitalSignals.size() > 0) {
        // Use num states for the first digital
        // signal as num states. All signals have the same size.
        states = digitalSignals.at(0)->numStates();
    }
    setNumStates(states);

    // ### digital signals
    mSignals->syncSignalsWithDevice();

    mAddAction->setEnabled(device->unusedDigitalIds().size() > 0);
}

/*!
    Create and return the toolbar to use for digital signal generation.
*/
QToolBar* UiDigitalGenerator::createToolBar()
{
    // Deallocation:
    //   Re-parented when calling verticalLayout->addWidget in the constructor
    QToolBar* toolBar = new QToolBar("Digital generator settings");

    mAddAction = toolBar->addAction("Add");
    connect(mAddAction, SIGNAL(triggered()), this, SLOT(addSignal()));

    mRemoveAction = toolBar->addAction("Remove");
    connect(mRemoveAction, SIGNAL(triggered()),
            this, SLOT(removeSelectedSignals()));
    mRemoveAction->setEnabled(false);

    toolBar->addSeparator();
    mRate = createRateBox();
    // Deallocation: Toolbar takes ownership of label
    toolBar->addWidget(new QLabel(tr(" Rate ")));
    toolBar->addWidget(mRate);

    mStatesBox = createStatesBox();
    toolBar->addSeparator();
    // Deallocation: Toolbar takes ownership of label
    toolBar->addWidget(new QLabel(tr(" States ")));
    toolBar->addWidget(mStatesBox);


    return toolBar;
}

/*!
    Create and return the table used to visualize digital signals.
*/
QTableView* UiDigitalGenerator::createTable()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QTableView* table = new QTableView(this);

    QItemSelectionModel* m = table->selectionModel();
    if (m != NULL) delete m;
    table->setModel(mSignals);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    DigitalDelegate* delegate = new DigitalDelegate(this);
    QAbstractItemDelegate* d = table->itemDelegate();
    if (d != NULL) delete d;
    table->setItemDelegate(delegate);

    table->resizeColumnsToContents();
    table->resizeRowsToContents();

    connect(table->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this,
            SLOT(handleSelectionChanged(QItemSelection,QItemSelection)));


    return table;
}

/*!
    Create the box used to set the rate/frequency.
*/
QLineEdit* UiDigitalGenerator::createRateBox()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QLineEdit* rate = new QLineEdit(this);
    rate->setToolTip(tr("Frequency"));
    rate->setMaximumWidth(80);

    mLastRateText = StringUtil::frequencyToString(device->maxDigitalRate());
    rate->setText(mLastRateText);

    connect(rate, SIGNAL(editingFinished()), this, SLOT(updateRate()));


    return rate;
}

/*!
    Create and return the box used to modify number of states.
*/
QSpinBox* UiDigitalGenerator::createStatesBox()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    // Deallocation:
    //   Toolbar takes ownership when adding this box by call to
    //   toolBar->addWidget(mStatesBox) in createToolBar
    QSpinBox* box = new QSpinBox();
    box->setToolTip(tr("The number of digital states to use"));


    box->setRange(2, device->maxNumDigitalStates());

    connect(box, SIGNAL(valueChanged(int)), this, SLOT(setNumStates(int)));

    return box;
}

/*!
    Return signals that have been selected in the table.
*/
QList<DigitalSignal*> UiDigitalGenerator::selectedSignals()
{
    QList<DigitalSignal*> result;

    QModelIndexList list = mTable->selectionModel()->selectedIndexes();


    foreach(QModelIndex index, list) {

        if (index.column() > 0 ) continue;

        QVariant d = index.data();
        if (!d.canConvert<DigitalSignal*>()) continue;
        DigitalSignal* s = d.value<DigitalSignal*>();
        if (s == NULL) continue;

        result.append(s);
    }

    return result;
}

/*!
    Called when asking the user to add a signal.
*/
void UiDigitalGenerator::addSignal()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;


    QMap<UiGeneratorSignalDialog::SignalType, QList<int> > map;
    map.insert(UiGeneratorSignalDialog::SignalDigital,
               device->unusedDigitalIds());

    UiGeneratorSignalDialog dialog(map, this);
    int result = dialog.exec();

    if (result == QDialog::Accepted) {

        QList<int> digitalIds = dialog.selectedSignals(
                    UiGeneratorSignalDialog::SignalDigital);
        foreach(int id, digitalIds) {
            mSignals->addSignal(id);
        }

    }

    mAddAction->setEnabled(device->unusedDigitalIds().size() > 0);
}

/*!
    Remove the signal that are selected in the table.
*/
void UiDigitalGenerator::removeSelectedSignals()
{

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    QList<DigitalSignal*> list = selectedSignals();
    if (list.size() > 0) {
        QString msg = "Do you want to remove the selected";
        if (list.size() == 1) {
            msg += " signal?";
        }
        else {
            msg += " signals?";
        }
        int result = QMessageBox::question(this,
                                           tr("Remove signals?"),
                                           msg,
                                           QMessageBox::Yes | QMessageBox::No);
        if (result == QMessageBox::Yes) {
            foreach(DigitalSignal* s, list) {
                mSignals->removeSignal(s);
            }
        }
    }

    if (device->digitalSignals().size() < device->maxNumDigitalSignals()) {
        mAddAction->setEnabled(true);
    }
}

/*!
    Called when the selection in the table has changed.
*/
void UiDigitalGenerator::handleSelectionChanged(QItemSelection selected,
                                                QItemSelection deselected)
{
    (void)deselected;

    if (selected.size() == 0) {
        mRemoveAction->setEnabled(false);
        return;
    }

    QList<DigitalSignal*> list = selectedSignals();
    if (list.size() > 0) {
        mRemoveAction->setEnabled(true);
    }
    else {
        mRemoveAction->setEnabled(false);
    }

}

/*!
    Called to update the rate/frequency
*/
void UiDigitalGenerator::updateRate()
{

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    QString t = mRate->text().trimmed();

    do {

        if (!StringUtil::isFrequencyStringValid(t)) {
            mRate->setText(mLastRateText);
            break;
        }

        int freq = StringUtil::frequencyToInt(t);

        if (freq < device->minDigitalRate()) {
            freq = device->minDigitalRate();
        }

        if (freq > device->maxDigitalRate()) {
            freq = device->maxDigitalRate();
        }

        t = StringUtil::frequencyToString(freq);

        mRate->setText(t);

        mLastRateText = t;

    } while (false);

}



