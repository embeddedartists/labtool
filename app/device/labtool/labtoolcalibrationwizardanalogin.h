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
#ifndef LABTOOLCALIBRATIONWIZARDANALOGIN_H
#define LABTOOLCALIBRATIONWIZARDANALOGIN_H

#include <QWizardPage>
#include <QLabel>

#include "labtooldevicecomm.h"

class LabToolCalibrationWizardAnalogIn : public QWizardPage
{
    Q_OBJECT
public:
    explicit LabToolCalibrationWizardAnalogIn(QWidget *parent = 0);

    void setVisible(bool visible);
    bool isComplete() const;

protected:
    void initializePage();

signals:

public slots:

private slots:
    void recalibrateClicked();
    void handleCalibrationFailed(const char *msg);
    void handleCalibrationSuccess(LabToolCalibrationData* data);

private:
    QLabel *mLabel;
    QLabel *mLabelResult;

    double mA0[3];
    double mA1[3];
    int mLevel[3];

    bool mIsCalibrated;
};

#endif // LABTOOLCALIBRATIONWIZARDANALOGIN_H
