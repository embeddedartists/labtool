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
#include "labtoolcalibrationwizardanalogout.h"
#include "labtoolcalibrationwizard.h"

#include <QBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

/*!
    \class LabToolCalibrationWizardAnalogOut
    \brief A page in the wizard for hardware calibration.

    \ingroup Device

    The LabToolCalibrationWizardAnalogOut class sets up the page
    in the hardware calibration wizard \ref LabToolCalibrationWizard for
    calibration of Analog Outputs.

*/

/*!
    Constructs a new wizard page with the given \a parent.
*/
LabToolCalibrationWizardAnalogOut::LabToolCalibrationWizardAnalogOut(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Calibration of Analog Outputs"));
    //setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark1.png"));

    mLabel = new QLabel(tr("This step will output three different values "
                           "on both analog outputs. For each of the values use a multimeter "
                           "and measure the output voltage. Enter the values you have found in "
                           "the corresponding text fields below.\n\n"
                           "Click on Next Value button to switch among the three different voltage levels.\n"));
    mLabel->setWordWrap(true);

    mLabelCurrentOutput = new QLabel(tr("Nothing outputted yet. Press Next Value to start...\n"));
    mLabelCurrentOutput->setWordWrap(true);

    QFormLayout* formLayoutA0 = new QFormLayout();
    QGroupBox* groupBoxA0 = new QGroupBox(tr("Settings for AOUT_0"));

    mSpinnerSuffix = " V";

    // all Spinners are deactivated by default. Will be activated row by row by
    // nextValueClicked Slot.
    mSpinnerLowA0 = new QDoubleSpinBox();
    mSpinnerLowA0->setDecimals(2);
    mSpinnerLowA0->setRange(2.5, 3.0);
    mSpinnerLowA0->setValue(2.75);
    mSpinnerLowA0->setSingleStep(0.01);
    mSpinnerLowA0->setSuffix(mSpinnerSuffix);
    mSpinnerLowA0->setDisabled(true);
    registerField("a0LowLevel", mSpinnerLowA0, "value", "valueChanged");
    formLayoutA0->addRow(tr("Low level (about 2.75V)"), mSpinnerLowA0);

    mSpinnerMiddleA0 = new QDoubleSpinBox();
    mSpinnerMiddleA0->setDecimals(2);
    mSpinnerMiddleA0->setRange(-0.5, 0.5);
    mSpinnerMiddleA0->setValue(0);
    mSpinnerMiddleA0->setSingleStep(0.01);
    mSpinnerMiddleA0->setSuffix(mSpinnerSuffix);
    mSpinnerMiddleA0->setDisabled(true);
    registerField("a0MiddleLevel", mSpinnerMiddleA0, "value", "valueChanged");
    formLayoutA0->addRow(tr("Middle level (about 0V)"), mSpinnerMiddleA0);

    mSpinnerHighA0 = new QDoubleSpinBox();
    mSpinnerHighA0->setDecimals(2);
    mSpinnerHighA0->setRange(-3.0, -2.5);
    mSpinnerHighA0->setValue(-2.75);
    mSpinnerHighA0->setSingleStep(0.01);
    mSpinnerHighA0->setSuffix(mSpinnerSuffix);
    mSpinnerHighA0->setDisabled(true);
    registerField("a0HighLevel", mSpinnerHighA0, "value", "valueChanged");
    formLayoutA0->addRow(tr("High level (about -2.75V)"), mSpinnerHighA0);

    groupBoxA0->setLayout(formLayoutA0);

    QFormLayout* formLayoutA1 = new QFormLayout();
    QGroupBox* groupBoxA1 = new QGroupBox(tr("Settings for AOUT_1"));

    mSpinnerLowA1 = new QDoubleSpinBox();
    mSpinnerLowA1->setDecimals(2);
    mSpinnerLowA1->setRange(2.5, 3.0);
    mSpinnerLowA1->setValue(2.75);
    mSpinnerLowA1->setSingleStep(0.01);
    mSpinnerLowA1->setSuffix(mSpinnerSuffix);
    mSpinnerLowA1->setDisabled(true);
    registerField("a1LowLevel", mSpinnerLowA1, "value", "valueChanged");
    formLayoutA1->addRow(tr("Low level (about 2.75V)"), mSpinnerLowA1);

    mSpinnerMiddleA1 = new QDoubleSpinBox();
    mSpinnerMiddleA1->setDecimals(2);
    mSpinnerMiddleA1->setRange(-0.5, 0.5);
    mSpinnerMiddleA1->setValue(0);
    mSpinnerMiddleA1->setSingleStep(0.01);
    mSpinnerMiddleA1->setSuffix(mSpinnerSuffix);
    mSpinnerMiddleA1->setDisabled(true);
    registerField("a1MiddleLevel", mSpinnerMiddleA1, "value", "valueChanged");
    formLayoutA1->addRow(tr("Middle level (about 0V)"), mSpinnerMiddleA1);

    mSpinnerHighA1 = new QDoubleSpinBox();
    mSpinnerHighA1->setDecimals(2);
    mSpinnerHighA1->setRange(-3.0, -2.5);
    mSpinnerHighA1->setValue(-2.75);
    mSpinnerHighA1->setSingleStep(0.01);
    mSpinnerHighA1->setSuffix(mSpinnerSuffix);
    mSpinnerHighA1->setDisabled(true);
    registerField("a1HighLevel", mSpinnerHighA1, "value", "valueChanged");
    formLayoutA1->addRow(tr("High level (about -2.75V)"), mSpinnerHighA1);

    groupBoxA1->setLayout(formLayoutA1);

    QBoxLayout* sidebyside = new QBoxLayout(QBoxLayout::LeftToRight);
    sidebyside->addWidget(groupBoxA0);
    sidebyside->addWidget(groupBoxA1);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabel);
    layout->addWidget(mLabelCurrentOutput);

    layout->addLayout(sidebyside);

    setLayout(layout);

    mCurrentLevel = HIGH;

    mOneValueCycle = false;
}

/*!
    \enum LabToolCalibrationWizardAnalogOut::Level

    The different levels of analog output.
*/
/*!
    \var LabToolCalibrationWizardAnalogOut::Level LabToolCalibrationWizardAnalogOut::LOW
    Represents the low output level, i.e. -2.75V
*/
/*!
    \var LabToolCalibrationWizardAnalogOut::Level LabToolCalibrationWizardAnalogOut::MIDDLE
    Represents the middle output level, i.e. 0V
*/
/*!
    \var LabToolCalibrationWizardAnalogOut::Level LabToolCalibrationWizardAnalogOut::HIGH
    Represents the high output level, i.e. +2.75V
*/

/*!
    Overrides the QWizardPage::setVisible() function to detect when the page is
    entered/left to configure the Next Value button.
*/
void LabToolCalibrationWizardAnalogOut::setVisible(bool visible)
{
    QWizardPage::setVisible(visible);

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("Next &Value"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        connect(wizard(), SIGNAL(customButtonClicked(int)),
                this, SLOT(nextValueClicked()));
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        disconnect(wizard(), SIGNAL(customButtonClicked(int)),
                   this, SLOT(nextValueClicked()));
    }
}

/*!
 * \brief Validate the Next Button of the Wizard Page
 */
bool LabToolCalibrationWizardAnalogOut::isComplete() const
{
    return (mOneValueCycle == true) ? true : false;
}

/*!
    Overrides the QWizardPage::initializePage() function to fill in any default values.

    If any default values should be entered (e.g. the old calibrations
    values) it must be done here as the algorithm for determining when
    the Next button is enabled compares the current values of the
    fields with the values they had BEFORE the call to initializePage.
    That means that the values cannot be entered in the constructor.
*/
void LabToolCalibrationWizardAnalogOut::initializePage()
{
//    mLineEditLowA0->setText("2.73");
//    mLineEditMiddleA0->setText("-0.023");
//    mLineEditHighA0->setText("-2.771");
//    mLineEditLowA1->setText("2.723");
//    mLineEditMiddleA1->setText("-0.034");
//    mLineEditHighA1->setText("-2.797");
}

/*!
    Called when the user clicks the Next Value button. Sends a message to the
    LabTool Hardware to output the next analog value in the sequence. The result is
    reported in either \ref handleCalibrationFailed or \ref handleCalibrationSuccess.
*/
void LabToolCalibrationWizardAnalogOut::nextValueClicked()
{
    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();

    QObject::connect(comm, SIGNAL(calibrationFailed(const char*)),
            this, SLOT(handleCalibrationFailed(const char*)));

    QObject::connect(comm, SIGNAL(calibrationSuccess(LabToolCalibrationData*)),
            this, SLOT(handleCalibrationSuccess(LabToolCalibrationData*)));

    switch (mCurrentLevel)
    {
    case LOW:
        mSpinnerMiddleA0->setDisabled(false);
        mSpinnerMiddleA1->setDisabled(false);

        mSpinnerLowA0->setDisabled(true);
        mSpinnerLowA1->setDisabled(true);

        mCurrentLevel = MIDDLE;
        break;
    case MIDDLE:
        mSpinnerHighA0->setDisabled(false);
        mSpinnerHighA1->setDisabled(false);

        mSpinnerMiddleA0->setDisabled(true);
        mSpinnerMiddleA1->setDisabled(true);

        mOneValueCycle = true;
        mCurrentLevel = HIGH;
        break;
    case HIGH:
        mSpinnerLowA0->setDisabled(false);
        mSpinnerLowA1->setDisabled(false);

        mSpinnerHighA0->setDisabled(true);
        mSpinnerHighA1->setDisabled(true);

        mCurrentLevel = LOW;
        break;
    default:
        break;
    }

    comm->calibrateAnalogOut(mCurrentLevel);

    if(mOneValueCycle)
        emit completeChanged();
}

/*!
    Called if the LabTool Hardware could not complete the calibration process.
    Presents the user with the error message.
*/
void LabToolCalibrationWizardAnalogOut::handleCalibrationFailed(const char *msg)
{
    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();
    comm->disconnect(this);

    QString message = QString("Failed to set output: %1\n").arg(msg);
    mLabelCurrentOutput->setText(message);
}

/*!
    Called if the LabTool Hardware could switch to the new output level.
*/
void LabToolCalibrationWizardAnalogOut::handleCalibrationSuccess(LabToolCalibrationData *data)
{
    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();
    comm->disconnect(this);

    (void)data; //To avoid warning

    switch (mCurrentLevel)
    {
    case LOW:
        mLabelCurrentOutput->setText(tr("Output on both A0 and A1 is about 2.75V\n"));
        break;
    case MIDDLE:
        mLabelCurrentOutput->setText(tr("Output on both A0 and A1 is about 0V\n"));
        break;
    case HIGH:
        mLabelCurrentOutput->setText(tr("Output on both A0 and A1 is about -2.75V\n"));
        break;
    default:
        mLabelCurrentOutput->setText(tr("Unknown state"));
        break;
    }
}

