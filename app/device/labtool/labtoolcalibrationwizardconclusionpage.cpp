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
#include "labtoolcalibrationwizardconclusionpage.h"

#include <QVBoxLayout>

/*!
    \class LabToolCalibrationWizardConclusionPage
    \brief A page in the wizard for hardware calibration.

    \ingroup Device

    The LabToolCalibrationWizardIntroPage class sets up the conclusion page
    in the hardware calibration wizard \ref LabToolCalibrationWizard.

*/

/*!
    Constructs a new wizard page with the given \a parent.
*/
LabToolCalibrationWizardConclusionPage::LabToolCalibrationWizardConclusionPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Calibration Completed"));

    mLabel = new QLabel(tr("The signals have been calibrated but the data is not "
                          "saved yet. To abort press Cancel. By pressing Finish the "
                           "data will be saved to the E2PROM on the LabTool hardware "
                           "to be used in all future capturing.\n\n"
                           "The signals can be recalibrated at any time by running "
                           "this wizard again."));
    mLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabel);

    setLayout(layout);
}
