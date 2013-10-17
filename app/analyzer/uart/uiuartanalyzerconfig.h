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
#ifndef UIUARTANALYZERCONFIG_H
#define UIUARTANALYZERCONFIG_H

#include <QWidget>

#include "uiuartanalyzer.h"
#include "analyzer/uianalyzerconfig.h"
#include "capture/uicursor.h"


class UiUartAnalyzerConfig : public UiAnalyzerConfig
{
    Q_OBJECT
public:
    explicit UiUartAnalyzerConfig(QWidget *parent = 0);
    int signalId();
    void setSignalId(int id);

    Types::DataFormat dataFormat();
    void setDataFormat(Types::DataFormat format);

    void setBaudRate(int rate);
    int baudRate();

    void setParity(Types::UartParity parity);
    Types::UartParity parity();

    void setStopBits(int bits);
    int stopBits();

    void setDataBits(int bits);
    int dataBits();

    UiCursor::CursorId syncCursor();
    void setSyncCursor(UiCursor::CursorId id);



signals:
    
public slots:

private:

    QComboBox* mSignalBox;
    QComboBox* mFormatBox;
    QComboBox* mDataBitsBox;
    QComboBox* mParityBox;
    QLineEdit* mBaudRate;
    QComboBox* mStopBitsBox;
    QComboBox* mCursorBox;

    
};

#endif // UIUARTANALYZERCONFIG_H
