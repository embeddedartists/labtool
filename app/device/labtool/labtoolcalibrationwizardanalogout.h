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
#ifndef LABTOOLCALIBRATIONWIZARDANALOGOUT_H
#define LABTOOLCALIBRATIONWIZARDANALOGOUT_H

#include <QWizardPage>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QDoubleValidator>

#include "labtooldevicecomm.h"

#define SPINNER_DECIMALS 3
#define SPINNER_SINGLE_STEP 0.001

class LabToolCalibrationWizardAnalogOut : public QWizardPage
{
    Q_OBJECT
public:
    enum Level {
        LOW    = 256,
        MIDDLE = 512,
        HIGH   = 768
    };

    explicit LabToolCalibrationWizardAnalogOut(QWidget *parent = 0);

    // QWizardPage interface
    void setVisible(bool visible);
    bool isComplete() const;

protected:
    void initializePage();

signals:

public slots:

private slots:
    void nextValueClicked();
    void handleCalibrationFailed(const char *msg);
    void handleCalibrationSuccess(LabToolCalibrationData* data);

private:

    QLabel *mLabel;
    QLabel *mLabelCurrentOutput;

    QString mSpinnerSuffix;

    QDoubleSpinBox *mSpinnerLowA0;
    QDoubleSpinBox *mSpinnerMiddleA0;
    QDoubleSpinBox *mSpinnerHighA0;

    QDoubleSpinBox *mSpinnerLowA1;
    QDoubleSpinBox *mSpinnerMiddleA1;
    QDoubleSpinBox *mSpinnerHighA1;

    Level mCurrentLevel;
    bool mOneValueCycle;

};

#endif // LABTOOLCALIBRATIONWIZARDANALOGOUT_H
