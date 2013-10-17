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
#include "labtoolcalibrationwizard.h"
#include "labtoolcalibrationwizardintropage.h"
#include "labtoolcalibrationwizardanalogout.h"
#include "labtoolcalibrationwizardanalogin.h"
#include "labtoolcalibrationwizardconclusionpage.h"

#include <QMessageBox>
#include <QAbstractButton>

/*!
    \class LabToolCalibrationWizard
    \brief A graphical wizard for hardware calibration.

    \ingroup Device

    The LabToolCalibrationWizard class provides a wizard guiding the
    user through the steps needed to calibrate the LabTool Hardware in
    order to get the optimal performance out of it.

*/

/*!
    Constructs a new wizard with the given \a parent.
*/
LabToolCalibrationWizard::LabToolCalibrationWizard(QWidget *parent) :
    QWizard(parent)
{
    addPage(new LabToolCalibrationWizardIntroPage);
    addPage(new LabToolCalibrationWizardAnalogOut);
    addPage(new LabToolCalibrationWizardAnalogIn);
    addPage(new LabToolCalibrationWizardConclusionPage);

    setWindowTitle(tr("Calibration Wizard"));

    mData = NULL;
    mDeviceComm = NULL;
    mMode = MODE_ABORT;
}

/*!
    Deletes the calibration data object if it has been created.
*/
LabToolCalibrationWizard::~LabToolCalibrationWizard()
{
    if (mData != NULL) {
        delete mData;
    }
}

/*!
    Overrides the QWizard::accept() function to detect when the user has clicked
    the Finish button. Saves the calibration in the LabTool Hardware's persistant
    memory.
*/
void LabToolCalibrationWizard::accept()
{
    button(QWizard::CancelButton)->setEnabled(false);
    button(QWizard::FinishButton)->setEnabled(false);
    button(QWizard::BackButton)->setEnabled(false);

    QObject::connect(mDeviceComm, SIGNAL(calibrationFailed(const char*)),
            this, SLOT(handleCalibrationFailed(const char*)));

    QObject::connect(mDeviceComm, SIGNAL(calibrationSuccess(LabToolCalibrationData*)),
            this, SLOT(handleCalibrationSuccess(LabToolCalibrationData*)));

    mMode = MODE_SAVE;
    mDeviceComm->calibrationSaveData(mData);
}

/*!
    Overrides the QWizard::reject() function to detect when the user has clicked
    the Cancel button or the Esc key. Presents the user with options to
        -# Erase the calibration data currently stored in the LabTool Hardware
        -# Exit the wizard
        -# Return to the wizard

*/
void LabToolCalibrationWizard::reject()
{
    QString message = QString("What do you want to do?");
    message.append("\n\nPressing Restore Defaults will erase the calibration data from the hardware.");
    message.append("\nPressing Close will exit the wizard and will not modify the calibration data in the hardware.");
    message.append("\nPressing Cancel will return you to the wizard.");

    QMessageBox msgBox(parentWidget());
    msgBox.setText("You have selected to cancel the calibration wizard.");
    msgBox.setInformativeText(message);
    msgBox.setStandardButtons(QMessageBox::RestoreDefaults | QMessageBox::Cancel | QMessageBox::Close);
    msgBox.setDefaultButton(QMessageBox::Close);

    int ret = msgBox.exec();
    if (ret == QMessageBox::Cancel)
    {
        // go back to the wizard
        return;
    }

    button(QWizard::CancelButton)->setEnabled(false);
    button(QWizard::FinishButton)->setEnabled(false);
    button(QWizard::BackButton)->setEnabled(false);

    QObject::connect(mDeviceComm, SIGNAL(calibrationFailed(const char*)),
            this, SLOT(handleCalibrationFailed(const char*)));

    QObject::connect(mDeviceComm, SIGNAL(calibrationSuccess(LabToolCalibrationData*)),
            this, SLOT(handleCalibrationSuccess(LabToolCalibrationData*)));

    switch(ret) {
    case QMessageBox::RestoreDefaults:
        mMode = MODE_RESTORE;
        mDeviceComm->calibrationRestoreDefaults();
        break;
    case QMessageBox::Close:
    default:
        mMode = MODE_ABORT;
        mDeviceComm->calibrationEnd();
        break;
    }
}

/*!
    \fn void LabToolCalibrationWizard::setComm(LabToolDeviceComm* comm)

    Sets a new interface for the device communication.
*/


/*!
    \fn LabToolDeviceComm* LabToolCalibrationWizard::comm()

    Returns the interface for the device communication.
*/


/*!
    Replaces the current calibration data (if any) with \a data.

*/
void LabToolCalibrationWizard::setCalibrationData(LabToolCalibrationData *data)
{
    if (mData != NULL) {
        delete mData;
    }
    mData = data;
}

/*!
    Called if the connection to the LabTool Hardware is changed while the wizard is
    shown. Shows a warning message and exits the wizard with a rejected status.

    The \a connected parameter will always be false to indicate that a connection
    has been lost.
*/
void LabToolCalibrationWizard::handleConnectedStatus(bool connected)
{
    if (!connected) {
        mDeviceComm = NULL;
        QMessageBox msgBox(parentWidget());
        msgBox.setText("Lost connection to Hardware.");
        msgBox.setInformativeText("The connection to the hardware has been lost and "
                                  "the wizard will be closed without having modified "
                                  "any calibration data in the device.\n\n"
                                  "Reconnect the hardware and start the calibration "
                                  "wizard again.");
        msgBox.exec();
        QDialog::reject();
    }
}

/*!
    Called if the calibration could not be saved to or erased from the LabTool Hardware.
    The \a msg will be shown to the user in a dialog asking how to continue.
*/
void LabToolCalibrationWizard::handleCalibrationFailed(const char *msg)
{
    mDeviceComm->disconnect(this);

    if (mMode == MODE_ABORT)
    {
        qDebug("Failed to end calibration: %s", msg);
        QDialog::reject();
    }
    else
    {
        button(QWizard::CancelButton)->setEnabled(true);
        button(QWizard::FinishButton)->setEnabled(true);
        button(QWizard::BackButton)->setEnabled(true);

        QString message = QString("Error message: %1").arg(msg);
        message.append("\n\nPressing Discard will exit the wizard.");
        message.append("\nPressing Retry will let your try again.");
        QMessageBox msgBox(parentWidget());
        if (mMode == MODE_RESTORE) {
            msgBox.setText("Failed to erase stored calibration data.");
        } else {
            msgBox.setText("Failed to save calibration data.");
        }
        msgBox.setInformativeText(message);
        msgBox.setStandardButtons(QMessageBox::Discard | QMessageBox::Retry);
        msgBox.setDefaultButton(QMessageBox::Retry);
        int ret = msgBox.exec();
        if (ret == QMessageBox::Discard) {
            // exit the dialog
            QDialog::reject();
        }
    }
}

/*!
    Called if the calibration operation finished successfully.
    The \a data will always be NULL.
*/
void LabToolCalibrationWizard::handleCalibrationSuccess(LabToolCalibrationData *data)
{
    mDeviceComm->disconnect(this);

    (void)data; // To avoid warning

    if (mMode == MODE_ABORT)
    {
        qDebug("Chose to end calibration");
        QDialog::reject();
    }
    else
    {
        // inform the user
        QMessageBox msgBox(parentWidget());
        if (mMode == MODE_RESTORE) {
            msgBox.setText("The calibration data has been erased!");
        } else {
            msgBox.setText("The calibration data has been saved!");
        }
        msgBox.exec();

        // shut down wizard
        QDialog::accept();
    }
}
