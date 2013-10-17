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
#ifndef UISPIANALYZERCONFIG_H
#define UISPIANALYZERCONFIG_H

#include <QWidget>
#include <QComboBox>

#include "analyzer/uianalyzerconfig.h"
#include "common/types.h"
#include "capture/uicursor.h"

class UiSpiAnalyzerConfig : public UiAnalyzerConfig
{
    Q_OBJECT
public:
    explicit UiSpiAnalyzerConfig(QWidget *parent = 0);
    
    void setSckSignal(int id);
    int sckSignal();

    void setMosiSignal(int id);
    int mosiSignal();

    void setMisoSignal(int id);
    int misoSignal();

    void setEnableSignal(int id);
    int enableSignal();

    void setMode(Types::SpiMode mode);
    Types::SpiMode mode();

    void setEnableMode(Types::SpiEnable mode);
    Types::SpiEnable enableMode();

    void setDataBits(int bits);
    int dataBits();

    Types::DataFormat dataFormat();
    void setDataFormat(Types::DataFormat format);

    UiCursor::CursorId syncCursor();
    void setSyncCursor(UiCursor::CursorId id);

signals:
    
public slots:

private slots:
    void verifyChoice();

private:

    QComboBox* mSpiSckSignalBox;
    QComboBox* mSpiMosiSignalBox;
    QComboBox* mSpiMisoSignalBox;
    QComboBox* mSpiEnableSignalBox;
    QComboBox* mSpiModeBox;
    QComboBox* mSpiEnableModeBox;
    QComboBox* mSpiDataBitsBox;
    QComboBox* mFormatBox;
    QComboBox* mCursorBox;
    
};

#endif // UISPIANALYZERCONFIG_H
