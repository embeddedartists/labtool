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
#ifndef LABTOOLCALIBRATIONWIZARD_H
#define LABTOOLCALIBRATIONWIZARD_H

#include <QWizard>
#include "labtooldevicecomm.h"
#include "labtoolcalibrationdata.h"

class LabToolCalibrationWizard : public QWizard
{
    Q_OBJECT
public:
    explicit LabToolCalibrationWizard(QWidget *parent = 0);
    ~LabToolCalibrationWizard();

    void accept();
    void reject();

    void setComm(LabToolDeviceComm* comm) { mDeviceComm = comm; }
    LabToolDeviceComm* comm() { return mDeviceComm; }

    void setCalibrationData(LabToolCalibrationData* data);
signals:

public slots:
    void handleConnectedStatus(bool connected);

private slots:
    void handleCalibrationFailed(const char *msg);
    void handleCalibrationSuccess(LabToolCalibrationData* data);

private:

    enum Modes {
        MODE_SAVE,
        MODE_ABORT,
        MODE_RESTORE
    };

    LabToolDeviceComm* mDeviceComm;
    LabToolCalibrationData* mData;
    Modes mMode;
};

#endif // LABTOOLCALIBRATIONWIZARD_H
