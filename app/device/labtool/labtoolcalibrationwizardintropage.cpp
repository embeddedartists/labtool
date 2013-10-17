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
#include "labtoolcalibrationwizardintropage.h"

#include <QVBoxLayout>

/*!
    \class LabToolCalibrationWizardIntroPage
    \brief A page in the wizard for hardware calibration.

    \ingroup Device

    The LabToolCalibrationWizardIntroPage class sets up the introduction page
    in the hardware calibration wizard \ref LabToolCalibrationWizard.

*/

/*!
    Constructs a new wizard page with the given \a parent.
*/
LabToolCalibrationWizardIntroPage::LabToolCalibrationWizardIntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    mLabel = new QLabel(tr("This wizard will guide you through the steps "
                          "of calibrating the LabTool hardware.\n\n"
                           "You will need a multimeter for this."));
    mLabel->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(mLabel);
    setLayout(layout);
}
