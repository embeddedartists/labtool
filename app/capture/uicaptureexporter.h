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
#ifndef UICAPTUREEXPORTER_H
#define UICAPTUREEXPORTER_H

#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>

#include "device/capturedevice.h"

class UiCaptureExporter : public QDialog
{
    Q_OBJECT
public:
    explicit UiCaptureExporter(CaptureDevice* device, QWidget *parent = 0);
    
signals:
    
public slots:

private:

    CaptureDevice* mCaptureDevice;
    QVBoxLayout* mMainLayout;
    QComboBox* mExportFormatBox;
    QWidget* mFormatWidget;


    QStringList exportFormats();
    QWidget* createFormat(QString format);
    void exportData(QString format, QWidget* w);


    QWidget* createFormatCsv();
    void exportToCsv(QWidget* w);

private slots:
    void handleFormatChanged(QString format);
    void exportData();
    
};

#endif // UICAPTUREEXPORTER_H
