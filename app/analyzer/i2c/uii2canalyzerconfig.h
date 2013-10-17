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
#ifndef UII2CANALYZERCONFIG_H
#define UII2CANALYZERCONFIG_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>

#include "uii2canalyzer.h"
#include "analyzer/uianalyzerconfig.h"
#include "capture/uicursor.h"

class UiI2CAnalyzerConfig : public UiAnalyzerConfig
{
    Q_OBJECT
public:
    explicit UiI2CAnalyzerConfig(QWidget *parent = 0);
    int sclSignalId();
    int sdaSignalId();
    Types::DataFormat dataFormat();
    void setSclSignalId(int id);
    void setSdaSignalId(int id);
    void setDataFormat(Types::DataFormat format);

    UiCursor::CursorId syncCursor();
    void setSyncCursor(UiCursor::CursorId id);
    
signals:
    
public slots:

private slots:
    void verifyChoice();

private:
    QComboBox* mSclBox;
    QComboBox* mSdaBox;
    QComboBox* mFormatBox;
    QComboBox* mCursorBox;


    
};

#endif // UII2CANALYZERCONFIG_H
