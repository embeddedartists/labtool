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
#include "labtoolcalibrationwizardanalogin.h"
#include "labtoolcalibrationwizardanalogout.h"
#include "labtoolcalibrationwizard.h"

#include <QVBoxLayout>
#include <QVariant>
#include <QMessageBox>
#include <QAbstractButton>

/*!
    \class LabToolCalibrationWizardAnalogIn
    \brief A page in the wizard for hardware calibration.

    \ingroup Device

    The LabToolCalibrationWizardAnalogIn class sets up the page
    in the hardware calibration wizard \ref LabToolCalibrationWizard for
    calibration of Analog Inputs.

*/

/*!
    Constructs a new wizard page with the given \a parent.
*/
LabToolCalibrationWizardAnalogIn::LabToolCalibrationWizardAnalogIn(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Calibration of Analog Inputs"));
    //setPixmap(QWizard::WatermarkPixmap, QPixmap(":/images/watermark1.png"));

    mLabel = new QLabel(tr("This step will calibrate the analog inputs.\n\n"
                           "Please connect the AIN_0 input (connector J4-26) to "
                           "the A_OUT0 output (connector J4-20) and the AIN_1 "
                           "input (connector J4-24) to the A_OUT1 output "
                           "(connector J4-18).\n"));
    mLabel->setWordWrap(true);

    mLabelResult = new QLabel(tr("No calibration data yet. Click ReCalibrate to continue..."));
    mLabelResult->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabel);
    layout->addWidget(mLabelResult);

    setLayout(layout);

    mA0[0] = mA0[1] = mA0[2] = 0;
    mA1[0] = mA1[1] = mA1[2] = 0;
    mLevel[0] = mLevel[1] = mLevel[2] = 0;
    mIsCalibrated = false;
}

/*!
    Overrides the QWizardPage::setVisible() function to detect when the page is
    entered/left to configure the ReCalibrate button.
*/
void LabToolCalibrationWizardAnalogIn::setVisible(bool visible)
{
    QWizardPage::setVisible(visible);

    if (visible) {
        wizard()->setButtonText(QWizard::CustomButton1, tr("&ReCalibrate"));
        wizard()->setOption(QWizard::HaveCustomButton1, true);
        connect(wizard(), SIGNAL(customButtonClicked(int)),
                this, SLOT(recalibrateClicked()));
        wizard()->button(QWizard::CustomButton1)->setEnabled(true);
    } else {
        wizard()->setOption(QWizard::HaveCustomButton1, false);
        disconnect(wizard(), SIGNAL(customButtonClicked(int)),
                   this, SLOT(recalibrateClicked()));
    }
    emit completeChanged();
}

/*!
    Overrides the QWizardPage::isComplete() function to prevent the Next button from
    being enabled until some calibration data is present.
*/
bool LabToolCalibrationWizardAnalogIn::isComplete() const
{
    return mIsCalibrated;
}

/*!
    Overrides the QWizardPage::initializePage() function to fill in values entered on the
    previous page. All the values that the user measured and entered durin Analog Output
    calibration \ref LabToolCalibrationWizardAnalogOut will be extracted for use in the
    communication with the LabTool Hardware.
*/
void LabToolCalibrationWizardAnalogIn::initializePage()
{
    mA0[0] = field("a0LowLevel").toDouble();
    mA0[1] = field("a0MiddleLevel").toDouble();
    mA0[2] = field("a0HighLevel").toDouble();

    mA1[0] = field("a1LowLevel").toDouble();
    mA1[1] = field("a1MiddleLevel").toDouble();
    mA1[2] = field("a1HighLevel").toDouble();

    mLevel[0] = LabToolCalibrationWizardAnalogOut::LOW;
    mLevel[1] = LabToolCalibrationWizardAnalogOut::MIDDLE;
    mLevel[2] = LabToolCalibrationWizardAnalogOut::HIGH;

    wizard()->button(QWizard::CustomButton1)->setEnabled(true);
}

/*!
    Called when the user clicks the ReCalibrate button. Sends a message to the
    LabTool Hardware to start the calibration process. The result is reported
    in either \ref handleCalibrationFailed or \ref handleCalibrationSuccess.
*/
void LabToolCalibrationWizardAnalogIn::recalibrateClicked()
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(false);

    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();

    QObject::connect(comm, SIGNAL(calibrationFailed(const char*)),
            this, SLOT(handleCalibrationFailed(const char*)));

    QObject::connect(comm, SIGNAL(calibrationSuccess(LabToolCalibrationData*)),
            this, SLOT(handleCalibrationSuccess(LabToolCalibrationData*)));

    comm->calibrateAnalogIn(mA0, mA1, mLevel);
}

/*!
    Called if the LabTool Hardware could not complete the calibration process.
    Presents the user with the error message and disables the Next button.
*/
void LabToolCalibrationWizardAnalogIn::handleCalibrationFailed(const char *msg)
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);

    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();
    comm->disconnect(this);

    QString message = QString("Calibration failed: %1").arg(msg);
    mLabelResult->setText(message);
    mIsCalibrated = false;
    emit completeChanged();
}

/*!
    Called if the LabTool Hardware completed the calibration process.
    Runs a sanity test on the new calibration \a data and enables the
    Next button if it passes. The new calibration data is stored in the
    wizard.
*/
void LabToolCalibrationWizardAnalogIn::handleCalibrationSuccess(LabToolCalibrationData *data)
{
    wizard()->button(QWizard::CustomButton1)->setEnabled(true);

    LabToolCalibrationWizard* wiz = (LabToolCalibrationWizard*)wizard();
    LabToolDeviceComm* comm = wiz->comm();
    comm->disconnect(this);

    wiz->setCalibrationData(data);
    data->printRawInfo();
    data->printCalibrationInfo();

    if (data->isDataReasonable())
    {
        mLabelResult->setText("Calibration data gathered. Press Next to continue...");
    }
    else
    {
        QString s("The data contains one or more values that "
                  "seems to be out of range.\n\n"
                  "This is typically because the signals were "
                  "not properly connected on the hardware.\n\n"
                  "Please check the wiring and then press "
                  "ReCalibrate again. If you are sure the values "
                  "are correct then continue by pressing Next..."
                  "\n\nIf this happens even if the wires are correcly "
                  "connected it can be because of incorrecly entered "
                  "values for the analog out. Click Back to go to the "
                  "previous step and verify that the values (particularly "
                  "the signs) are correct.");
        QMessageBox msgBox(parentWidget());
        msgBox.setText("Got possibly faulty calibration data.");
        msgBox.setInformativeText(s);
        msgBox.exec();
        mLabelResult->setText(s);
    }

    mIsCalibrated = true;
    emit completeChanged();
}
