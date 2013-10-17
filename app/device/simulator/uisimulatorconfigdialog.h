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
#ifndef UISIMULATORCONFIGDIALOG_H
#define UISIMULATORCONFIGDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>

#include "common/types.h"


class UiSimulatorConfigDialog : public QDialog
{
    Q_OBJECT
public:

    enum DigitalFunction {
        DigitalFunction_Random,
        DigitalFunction_I2C,
        DigitalFunction_UART,
        DigitalFunction_SPI
    };

    enum AnalogFunction {
        AnalogFunction_Random,
        AnalogFunction_Sine
    };


    explicit UiSimulatorConfigDialog(QWidget *parent = 0);

    DigitalFunction digitalFunction();
    AnalogFunction analogFunction();

    int uartSignalId();
    int uartDataBits();
    int uartStopBits();
    int uartBaudRate();
    Types::UartParity uartParity();

    int i2cSclSignalId();
    int i2cSdaSignalId();
    int i2cRate();
    Types::I2CAddress i2cAddressType();

    int spiSckSignalId();
    int spiMosiSignalId();
    int spiMisoSignalId();
    int spiEnableSignalId();
    int spiRate();
    Types::SpiMode spiMode();
    Types::SpiEnable spiEnableMode();
    int spiDataBits();

signals:
    
public slots:

private slots:
    void handleDigitalFunctionChange(int idx);

private:

    QComboBox* mDigFuncBox;
    QComboBox* mAnFuncBox;

    QWidget* mUartSettings;

    QComboBox* mUartSignalBox;
    QComboBox* mUartDataBitsBox;
    QComboBox* mUartParityBox;
    QLineEdit* mUartBaudRate;
    QComboBox* mUartStopBitsBox;

    QWidget* mI2cSettings;

    QComboBox* mI2cSclSignalBox;
    QComboBox* mI2cSdaSignalBox;
    QLineEdit* mI2cRate;
    QComboBox* mI2cAddressBox;

    QWidget* mSpiSettings;

    QComboBox* mSpiSckSignalBox;
    QComboBox* mSpiMosiSignalBox;
    QComboBox* mSpiMisoSignalBox;
    QComboBox* mSpiEnableSignalBox;
    QLineEdit* mSpiRate;
    QComboBox* mSpiModeBox;
    QComboBox* mSpiEnableModeBox;
    QComboBox* mSpiDataBitsBox;

    QWidget* createUartSettings();
    QWidget* createI2cSettings();
    QWidget* createSpiSettings();
    
};

#endif // UISIMULATORCONFIGDIALOG_H

