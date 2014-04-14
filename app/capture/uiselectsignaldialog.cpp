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
#include "uiselectsignaldialog.h"

#include <QDebug>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include <QButtonGroup>
#include <QCheckBox>

#include "signalmanager.h"
#include "uidigitalsignal.h"
#include "uianalogsignal.h"
#include "common/configuration.h"
#include "device/devicemanager.h"
#include "analyzer/analyzermanager.h"


/*!
    \class UiSelectSignalDialog
    \brief UI widget used to select signals and analyzers to add to
    the signal plot.

    \ingroup Capture

*/


/*!
    Constructs an UiSelectSignalDialog with the given \a parent.
*/
UiSelectSignalDialog::UiSelectSignalDialog(QWidget *parent) :
    QDialog(parent),
    mDigitalSignalsMap(),
    mAnalogSignalsMap()
{
    mAnalyzersBox = NULL;

    setWindowTitle(tr("Add Signal or Analyzer"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation:
    //   formLayout will be re-parented when calling verticalLayout->addLayout
    //   which means that it will be deleted when UiSimulatorConfigDialog is
    //   deleted.
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    // digital signals
    QList<int> s = device->unusedDigitalIds();
    if (s.size() > 0) {
        formLayout->addRow(tr("Digital signals: "), createDigitalSignalBox(s));
    }


    // analog signals
    s = device->unusedAnalogIds();
    if (s.size() > 0) {
        formLayout->addRow(tr("Analog signals: "), createAnalogSignalBox(s));
    }

    // analyzers
    mAnalyzersBox = createAnalyzerBox();
    formLayout->addRow(tr("Analyzers: "), mAnalyzersBox);

    // Deallocation:
    //   Ownership is transfered to UiSelectSignalDialog when calling
    //   setLayout below.
    QVBoxLayout* verticalLayout = new QVBoxLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QDialogButtonBox* bottonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok|QDialogButtonBox::Cancel,
                Qt::Horizontal,
                this);
    bottonBox->setCenterButtons(true);

    connect(bottonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bottonBox, SIGNAL(rejected()), this, SLOT(reject()));

    verticalLayout->addLayout(formLayout);
    verticalLayout->addWidget(bottonBox);


    setLayout(verticalLayout);
}

/*!
    Returns a list with selected digital signal IDs
*/
QList<int> UiSelectSignalDialog::selectedDigitalSignals()
{

    QList<int> list;

    QList<int> ids = mDigitalSignalsMap.keys();
    for (int i = 0; i < ids.size(); i++) {
        if (mDigitalSignalsMap.value(ids.at(i))->isChecked()) {
            list.append(ids.at(i));
        }
    }

    return list;
}

/*!
    Returns a list with selected analog signal IDs
*/
QList<int> UiSelectSignalDialog::selectedAnalogSignals()
{
    QList<int> list;

    QList<int> ids = mAnalogSignalsMap.keys();
    for (int i = 0; i < ids.size(); i++) {
        if (mAnalogSignalsMap.value(ids.at(i))->isChecked()) {
            list.append(ids.at(i));
        }
    }

    return list;
}

/*!
    Returns the name of the selected analyzer.
*/
QString UiSelectSignalDialog::selectedAnalyzer()
{
    return mAnalyzersBox->itemText(mAnalyzersBox->currentIndex());
}

/*!
   Create a signal box widget for the \a list of digital signals.
*/
QWidget* UiSelectSignalDialog::createDigitalSignalBox(QList<int> &list)
{

    // Deallocation:
    //   Ownership is taken over by digitalGroup by the call to
    //   digitalGroup->setlayout
    QGridLayout* l = new QGridLayout();
    l->setSizeConstraint(QLayout::SetFixedSize);


    for (int i = 0; i < list.size(); i++) {
        int id = list.at(i);

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QLabel* cl = new QLabel("    ");
        QString color = Configuration::instance().digitalCableColor(id).name();
        cl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));
        l->addWidget(cl, 0, i);

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        l->addWidget(new QLabel(QString("D%1").arg(id), this), 1, i);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QCheckBox* cb = new QCheckBox(this);
        l->addWidget(cb, 2, i);
        mDigitalSignalsMap.insert(id, cb);
    }


    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QWidget* digitalGroup = new QWidget(this);
    digitalGroup->setLayout(l);

    return digitalGroup;

}

/*!
   Create a signal box widget for the \a list of analog signals.
*/
QWidget* UiSelectSignalDialog::createAnalogSignalBox(QList<int> &list)
{

    // Deallocation:
    //   Ownership is taken over by call to w->setlayout
    QGridLayout* l = new QGridLayout();
    l->setSizeConstraint(QLayout::SetFixedSize);

    for (int i = 0; i < list.size(); i++) {
        int id = list.at(i);

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QLabel* cl = new QLabel("    ");
        QString color = Configuration::instance().analogInCableColor(id).name();
        cl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));
        l->addWidget(cl, 0, i);

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        l->addWidget(new QLabel(QString("A%1").arg(id), this), 1, i);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QCheckBox* cb = new QCheckBox(this);
        l->addWidget(cb, 2, i);
        mAnalogSignalsMap.insert(id, cb);
    }

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QWidget* w = new QWidget(this);
    w->setLayout(l);

    return w;

}

/*!
   Create a signal box widget for the supported analyzers.
*/
QComboBox* UiSelectSignalDialog::createAnalyzerBox()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(this);
    box->addItem("<No Analyzer Selected>", QVariant(-1));

    QList<QString> analyzers =  AnalyzerManager::analyzers();
    for (int i = 0; i < analyzers.size(); i++) {
        box->addItem(analyzers.at(i));
    }

    return box;
}
