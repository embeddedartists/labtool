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

    mValidator.setRange(-5.0, 5.0, 5);

    mLineEditLowA0 = new QLineEdit();
    mLineEditLowA0->setValidator(&mValidator);
    registerField("a0LowLevel*", mLineEditLowA0);
    formLayoutA0->addRow(tr("Low level (about 2.75V)"), mLineEditLowA0);

    mLineEditMiddleA0 = new QLineEdit();
    mLineEditMiddleA0->setValidator(&mValidator);
    registerField("a0MiddleLevel*", mLineEditMiddleA0);
    formLayoutA0->addRow(tr("Middle level (about 0V)"), mLineEditMiddleA0);

    mLineEditHighA0 = new QLineEdit();
    mLineEditHighA0->setValidator(&mValidator);
    registerField("a0HighLevel*", mLineEditHighA0);
    formLayoutA0->addRow(tr("High level (about -2.75V)"), mLineEditHighA0);

    groupBoxA0->setLayout(formLayoutA0);

    QFormLayout* formLayoutA1 = new QFormLayout();
    QGroupBox* groupBoxA1 = new QGroupBox(tr("Settings for AOUT_1"));

    mLineEditLowA1 = new QLineEdit();
    mLineEditLowA1->setValidator(&mValidator);
    registerField("a1LowLevel*", mLineEditLowA1);
    formLayoutA1->addRow(tr("Low level (about 2.75V)"), mLineEditLowA1);

    mLineEditMiddleA1 = new QLineEdit();
    mLineEditMiddleA1->setValidator(&mValidator);
    registerField("a1MiddleLevel*", mLineEditMiddleA1);
    formLayoutA1->addRow(tr("Middle level (about 0V)"), mLineEditMiddleA1);

    mLineEditHighA1 = new QLineEdit();
    mLineEditHighA1->setValidator(&mValidator);
    registerField("a1HighLevel*", mLineEditHighA1);
    formLayoutA1->addRow(tr("High level (about -2.75V)"), mLineEditHighA1);

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
        mCurrentLevel = MIDDLE;
        break;
    case MIDDLE:
        mCurrentLevel = HIGH;
        break;
    case HIGH:
    default:
        mCurrentLevel = LOW;
        break;
    }

    comm->calibrateAnalogOut(mCurrentLevel);
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
