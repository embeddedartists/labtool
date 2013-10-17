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
#include <QLineEdit>
#include <QDoubleValidator>

#include "labtooldevicecomm.h"

class LabToolCalibrationWizardAnalogOut : public QWizardPage
{
    Q_OBJECT
public:
    explicit LabToolCalibrationWizardAnalogOut(QWidget *parent = 0);

    void setVisible(bool visible);

    enum Level {
        LOW    = 256,
        MIDDLE = 512,
        HIGH   = 768
    };

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

    QLineEdit *mLineEditLowA0;
    QLineEdit *mLineEditMiddleA0;
    QLineEdit *mLineEditHighA0;

    QLineEdit *mLineEditLowA1;
    QLineEdit *mLineEditMiddleA1;
    QLineEdit *mLineEditHighA1;

    QDoubleValidator mValidator;

    Level mCurrentLevel;
};

#endif // LABTOOLCALIBRATIONWIZARDANALOGOUT_H
