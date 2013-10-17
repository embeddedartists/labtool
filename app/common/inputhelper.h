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
#ifndef INPUTHELPER_H
#define INPUTHELPER_H

#include <QObject>
#include <QComboBox>
#include <QLineEdit>

#include "common/types.h"

class InputHelper
{
public:


    static int intValue(QComboBox* box);
    static int intValue(QLineEdit* box);
    static void setInt(QComboBox* box, int value);

    static QComboBox* createSignalBox(QWidget* parent, int selected);
    static QComboBox* createActiveCursorsBox(QWidget* parent, int selected);
    static QComboBox* createFormatBox(QWidget* parent, Types::DataFormat selectedFormat);

    static QLineEdit* createUartBaudRateBox(QWidget* parent, int rate);
    static QComboBox* createUartParityBox(QWidget* parent, Types::UartParity parity);
    static QComboBox* createUartStopBitsBox(QWidget* parent, int selectedBits);
    static QComboBox* createUartDataBitsBox(QWidget* parent, int selectedBits);

    static QLineEdit* createI2cRateBox(QWidget* parent, int rate);
    static QComboBox* createI2cAddressLengthBox(QWidget* parent, Types::I2CAddress address);

    static QLineEdit* createSpiRateBox(QWidget* parent, int rate);
    static QComboBox* createSpiModeBox(QWidget* parent, Types::SpiMode mode);
    static QComboBox* createSpiDataBitsBox(QWidget* parent, int selectedBits);
    static QComboBox* createSpiEnableModeBox(QWidget* parent, Types::SpiEnable mode);
    
private:
    explicit InputHelper();
    
};

#endif // INPUTHELPER_H
