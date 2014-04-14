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
#include "uigeneratorsignaldialog.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>

#include "common/configuration.h"

/*!
    \class UiGeneratorSignalDialog
    \brief Dialog window used by user to select which signals to use for
    signal generation.

    \ingroup Generator

*/

/*!
    \enum UiGeneratorSignalDialog::SignalType

    This enum describes the different signal types available.

    \var UiGeneratorSignalDialog::SignalType UiGeneratorSignalDialog::SignalDigital
    Digital signal

    \var UiGeneratorSignalDialog::SignalType UiGeneratorSignalDialog::SignalAnalog
    Analog signal
*/



/*!
    Constructs the UiGeneratorSignalDialog with the given \a parent and list
    with unused signals.
*/
UiGeneratorSignalDialog::UiGeneratorSignalDialog(
        QMap<SignalType, QList<int> > unused,
        QWidget *parent) :
    QDialog(parent),
    mDigitalSignalsMap()
{

    setWindowTitle(tr("Add signal(s)"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation: Re-parented by call to verticalLayout->addLayout
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setRowWrapPolicy(QFormLayout::WrapAllRows);

    foreach(UiGeneratorSignalDialog::SignalType type, unused.keys()) {
        QList<int> unusedIds = unused.value(type);
        QString lbl = "";

        switch (type) {
        case UiGeneratorSignalDialog::SignalDigital:
            lbl = tr("Digital signals: ");
            break;
        case UiGeneratorSignalDialog::SignalAnalog:
            lbl = tr("Analog signals: ");
            break;
        }

        formLayout->addRow(lbl, createSignalBox(type, unusedIds));
    }

    // Deallocation: Ownership changed when calling setLayout
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
    Returns list with selected signals for given signal \a type.
*/
QList<int> UiGeneratorSignalDialog::selectedSignals(SignalType type)
{

    QList<int> list;
    QMap<int, QCheckBox*>* map = NULL;

    QList<int> ids;
    switch(type) {
    case SignalDigital:
        ids = mDigitalSignalsMap.keys();
        map = &mDigitalSignalsMap;
        break;
    case SignalAnalog:
        ids = mAnalogSignalsMap.keys();
        map = &mAnalogSignalsMap;
        break;
    }


    for (int i = 0; i < ids.size(); i++) {
        if (map->value(ids.at(i))->isChecked()) {
            list.append(ids.at(i));
        }
    }

    return list;
}

/*!
    Create widget used for selecting signals of \type. Valid signals to select
    are specified in \a list.
*/
QWidget* UiGeneratorSignalDialog::createSignalBox(SignalType type,
                                                  QList<int> &list)
{
    QMap<int, QCheckBox*>* map = NULL;

    // Deallocation: Ownership changed when calling setLayout
    QGridLayout* l = new QGridLayout();
    l->setSizeConstraint(QLayout::SetFixedSize);


    for (int i = 0; i < list.size(); i++) {
        int id = list.at(i);
        QLabel* lbl = NULL;

        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QLabel* cl = new QLabel("    ");
        QString color;

        switch(type) {
        case SignalDigital:
            color = Configuration::instance().digitalCableColor(id).name();
            cl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            lbl = new QLabel(QString("D%1").arg(id), this);
            map = &mDigitalSignalsMap;
            break;
        case SignalAnalog:
            color = Configuration::instance().analogOutCableColor(id).name();
            cl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            lbl = new QLabel(QString("A%1").arg(id), this);
            map = &mAnalogSignalsMap;
            break;
        }
        l->addWidget(cl, 0, i);
        l->addWidget(lbl, 1, i);
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        QCheckBox* cb = new QCheckBox(this);
        l->addWidget(cb, 2, i);
        map->insert(id, cb);
    }

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QWidget* digitalGroup = new QWidget(this);
    digitalGroup->setLayout(l);

    return digitalGroup;

}
