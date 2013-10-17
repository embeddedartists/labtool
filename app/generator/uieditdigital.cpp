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
#include "uieditdigital.h"

#include <QDebug>
#include <QFormLayout>
#include <QIcon>
#include <QGroupBox>
#include <QPushButton>
#include <QSpinBox>
#include <QMessageBox>

#define TYPE_WIDGET_INDEX (1)

/*!
    \class UiEditDigital
    \brief UI widget that is responsible for digital signal generation settings.

    \ingroup Generator

*/


/*!
    Constructs an UiEditDigital with the given digital signal \a signal
    and \a parent.
*/
UiEditDigital::UiEditDigital(DigitalSignal* signal, QWidget *parent) :
    QWidget(parent)
{
    setWindowFlags((windowFlags()
                    & ~(Qt::WindowMinimizeButtonHint|
                        Qt::WindowMaximizeButtonHint))
                   | Qt::Window);
    setWindowTitle(tr("Edit digital signal settings"));
    setWindowIcon(QIcon(":/resources/16_digital.png"));

    mTypeWidget = NULL;
    mSignal = signal;

    // Deallocation: Ownership changed when calling setLayout.
    mMainLayout = new QVBoxLayout();

    //
    //    ### Signal Name ###
    //

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mNameEdit = new QLineEdit(signal->name(), this);
    connect(mNameEdit, SIGNAL(editingFinished()),
            this, SLOT(handleNameEdited()));

    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(tr("Name: "), mNameEdit);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mGenTypeBox = new QComboBox(this);
    foreach(QString t, generateTypes()) {
        mGenTypeBox->addItem(t);
    }
    connect(mGenTypeBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(handleTypeChanged(QString)));
    mGenTypeBox->setToolTip(tr("Output a signal of this type"));
    formLayout->addRow(tr("Output: "), mGenTypeBox);

    mMainLayout->addLayout(formLayout);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QPushButton* genBtn = new QPushButton("Generate", this);
    connect(genBtn, SIGNAL(clicked()), this, SLOT(generateOutput()));

    // Deallocation: Re-parented when calling mMainLayout->addLayout.
    QHBoxLayout* hLayout = new QHBoxLayout();
    hLayout->addWidget(genBtn);
    hLayout->addStretch();
    mMainLayout->addLayout(hLayout);

    mMainLayout->addStretch();

    setLayout(mMainLayout);

    // force type widget to be displayed
    handleTypeChanged(generateTypes().at(0));
}

/*
    ---------------------------------------------------------------------------
    >>>> BEGIN -- Handle Output types >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    ---------------------------------------------------------------------------
*/


#define TYPE_CONSTANT "Constant"
#define TYPE_CLOCK    "Clock"

/*!
    Returns the generate type.
*/
QStringList UiEditDigital::generateTypes()
{
    return QList<QString>()
            << TYPE_CONSTANT
            << TYPE_CLOCK;
}

/*!
    Creates widget used for a the given generate type. The widget allows the
    user to generate a signal according to the specified \a type.
*/
QWidget* UiEditDigital::createType(QString type)
{
    if (TYPE_CONSTANT == type) {
        return createTypeConstant();
    }

    if (TYPE_CLOCK == type) {
        return createTypeClock();
    }

    return NULL;
}

/*!
    Generate signal data based on \a type. Settings are retrieved from the
    widget \a w and signal data is stored in the vector \a data. If a problem
    occurs a warning message is given in the out parameter \a warnMsg.

    The function returns true if signal data was generated; otherwise it
    returns false
*/
bool UiEditDigital::generateOutput(QString type, QWidget *w,
                                   QVector<bool> &data, QString &warnMsg)
{

    if (TYPE_CONSTANT == type) {
        return generateConstantOutput(w, data, warnMsg);
    }

    if (TYPE_CLOCK == type) {
        return generateClockOutput(w, data, warnMsg);
    }

    return false;
}

/*!
    Create a widget for constant (high or low) output generation.
*/
QWidget* UiEditDigital::createTypeConstant()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation: Ownership changed when calling setLayout
    QFormLayout* l = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* lvlBox = new QComboBox(w);
    lvlBox->setObjectName("constantLevel");
    lvlBox->addItem("0 - Low", 0);
    lvlBox->addItem("1 - High", 1);
    l->addRow(tr("Level:"), lvlBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QSpinBox* fromBox = new QSpinBox(w);
    fromBox->setObjectName("constantFrom");
    fromBox->setMinimum(0);
    fromBox->setMaximum(mSignal->numStates()-1);
    l->addRow(tr("From:"), fromBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QSpinBox* toBox = new QSpinBox(w);
    toBox->setObjectName("constantTo");
    toBox->setMinimum(0);
    toBox->setMaximum(mSignal->numStates()-1);
    toBox->setValue(mSignal->numStates()-1);
    l->addRow(tr("To:"), toBox);


    w->setLayout(l);

    return w;
}

/*!
    Generate signal data for constant output. Settings are retrieved from the
    widget \a w and signal data is stored in the vector \a data. If a problem
    occurs a warning message is given in the out parameter \a warnMsg.

    The function returns true if signal data was generated; otherwise it
    returns false
*/
bool UiEditDigital::generateConstantOutput(QWidget* w, QVector<bool> &data,
                                           QString &warnMsg)
{

    do {
        QComboBox* lvlBox = w->findChild<QComboBox*>("constantLevel");
        if (lvlBox == NULL) break;
        int lvl = lvlBox->itemData(lvlBox->currentIndex()).toInt();

        QSpinBox* fromBox = w->findChild<QSpinBox*>("constantFrom");
        if (fromBox == NULL) break;
        int from = fromBox->value();

        QSpinBox* toBox = w->findChild<QSpinBox*>("constantTo");
        if (toBox == NULL) break;
        int to = toBox->value();

        if (to < from) {
            warnMsg = "'From' state larger than 'to' state";
            break;
        }
        if (from < 0 || to >= mSignal->numStates()) break;

        setStates(data, (lvl == 1), from, to);


        return true;
    } while(false);

    return false;
}

/*!
    Create a widget for clock output generation, that is, a
    signal that oscillates between high and low states.
*/
QWidget* UiEditDigital::createTypeClock()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation: Ownership changed when calling setLayout
    QFormLayout* l = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* startBox = new QComboBox(w);
    startBox->setObjectName("clockStartLevel");
    startBox->addItem("0 - Low", 0);
    startBox->addItem("1 - High", 1);
    l->addRow(tr("Start level:"), startBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QSpinBox* dutyBox = new QSpinBox(w);
    dutyBox->setObjectName("clockDutyCycle");
    dutyBox->setMinimum(0);
    dutyBox->setMaximum(100);
    dutyBox->setValue(50);
    l->addRow(tr("Duty cycle:"), dutyBox);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QSpinBox* fromBox = new QSpinBox(w);
    fromBox->setObjectName("clockFromState");
    fromBox->setMinimum(0);
    fromBox->setMaximum(mSignal->numStates());
    l->addRow(tr("From:"), fromBox);

    w->setLayout(l);
    return w;
}

/*!
    Generate signal data for clock output. Settings are retrieved from the
    widget \a w and signal data is stored in the vector \a data. If a problem
    occurs a warning message is given in the out parameter \a warnMsg.

    The function returns true if signal data was generated; otherwise it
    returns false
*/
bool UiEditDigital::generateClockOutput(QWidget *w, QVector<bool> &data,
                                        QString &warnMsg)
{
    do {
        QComboBox* lvlBox = w->findChild<QComboBox*>("clockStartLevel");
        if (lvlBox == NULL) break;
        int startLevel = lvlBox->itemData(lvlBox->currentIndex()).toInt();

        QSpinBox* dutyBox = w->findChild<QSpinBox*>("clockDutyCycle");
        if (dutyBox == NULL) break;
        int dutyCycle = dutyBox->value();

        QSpinBox* fromBox = w->findChild<QSpinBox*>("clockFromState");
        if (fromBox == NULL) break;
        int from = fromBox->value();

        if (from < 0 || from >= data.size()) break;

        // greatest common factor
        int factor = dutyCycle;
        int b = 100;
        int tmp = 0;
        while(b != 0) {
            tmp = factor % b;
            factor = b;
            b = tmp;
        }

        int numStates = 100 / factor;
        int numHigh = dutyCycle/factor;
        int numLow = numStates - numHigh;

        // not enough states
        if (numStates > mSignal->numStates()-from ||
                ((mSignal->numStates()-from) % numStates) != 0 ) {
            warnMsg = QString("Output will be truncated! Number of states must be a multiple of %1 + the offset given in the 'From' field").arg(numStates);
        }


        bool genHighOutput = (startLevel == 1);
        while (from < mSignal->numStates()) {
            if (genHighOutput) {
                setStates(data, true, from, from+numHigh);
                from += numHigh;
            }
            else {
                setStates(data, false, from, from+numLow);
                from += numLow;
            }

            genHighOutput = !genHighOutput;
        }



        return true;

    } while(false);

    return false;
}

/*!
    Set all states in \a data to \a high between indexes \a from and \a to.
*/
void UiEditDigital::setStates(QVector<bool> &data, bool high, int from,
                              int to) const
{
    if (from > to) return;
    if (to >= data.size()) {
        to = data.size()-1;
    }

    for (int i = from; i <= to; i++) {
        data[i] = high;
    }
}

/*
    ---------------------------------------------------------------------------
    <<<< END -- Handle Output types <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
    ---------------------------------------------------------------------------
*/



/*!
    Called when the name of the signal is changed.
*/
void UiEditDigital::handleNameEdited()
{
    QString n = mNameEdit->text();
    if (n.isEmpty() || n.isNull()) {
        n = mSignal->name();
    }

    mSignal->setName(n);
}


/*!
    Called when the generate type has been changed.
*/
void UiEditDigital::handleTypeChanged(QString type)
{
    if (mTypeWidget != NULL) {
        mTypeWidget->close();
        delete mTypeWidget;
    }

    mTypeWidget = createType(type);
    mMainLayout->insertWidget(TYPE_WIDGET_INDEX, mTypeWidget);


    adjustSize();
}


/*!
    Called when output should be generated.
*/
void UiEditDigital::generateOutput()
{
    QString warnMsg;
    QVector<bool> data = mSignal->data();
    bool ok = generateOutput(
                mGenTypeBox->currentText(),
                mTypeWidget,
                data, warnMsg);
    if (ok) {
        mSignal->setData(data);
        if (parentWidget()) {
            parentWidget()->update();
        }
    }

    if (!ok && warnMsg.isNull()) {
        warnMsg = "Could not generate any output because of problems with your settings";
    }

    if (!warnMsg.isNull()) {
        QMessageBox::warning(this,
                             "Problem with output",
                             warnMsg);
    }
}
