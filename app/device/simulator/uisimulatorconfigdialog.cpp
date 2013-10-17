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
#include "uisimulatorconfigdialog.h"

#include <QDebug>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QLabel>

#include "common/inputhelper.h"

/*!
    \class UiSimulatorConfigDialog
    \brief Dialog window to ask the user about simulator choices

    \ingroup Device

    When testing the capture functionality and having the simulator enabled
    the user is asked which kind of signal to show/generate in the Capture
    window.
*/

/*!
    \enum UiSimulatorConfigDialog::DigitalFunction

    This enum describes the possible digital signals that can be generated
    by the simulator.

    \var UiSimulatorConfigDialog::DigitalFunction UiSimulatorConfigDialog::DigitalFunction_Random
    Random signal data

    \var UiSimulatorConfigDialog::DigitalFunction UiSimulatorConfigDialog::DigitalFunction_I2C
    I2C signal data

    \var UiSimulatorConfigDialog::DigitalFunction UiSimulatorConfigDialog::DigitalFunction_UART
    UART signal data

    \var UiSimulatorConfigDialog::DigitalFunction UiSimulatorConfigDialog::DigitalFunction_SPI
    SPI signal data
*/

/*!
    \enum UiSimulatorConfigDialog::AnalogFunction

    This enum describes the possible analog signals that can be generated
    by the simulator.

    \var UiSimulatorConfigDialog::AnalogFunction UiSimulatorConfigDialog::AnalogFunction_Random
    Random signal data

    \var UiSimulatorConfigDialog::AnalogFunction UiSimulatorConfigDialog::AnalogFunction_Sine
    Signal data with sine waveform
*/


/*!
    Constructs a simulator dialog the given \a parent.
*/
UiSimulatorConfigDialog::UiSimulatorConfigDialog(QWidget *parent) :
    QDialog(parent)
{    

    setWindowTitle(tr("Simulator Settings"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation:
    //   formLayout will be re-parented when calling verticalLayout->addLayout
    //   which means that it will be deleted when UiSimulatorConfigDialog is
    //   deleted.
    QFormLayout* formLayout = new QFormLayout;

    // digital functions

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mDigFuncBox = new QComboBox(this);
    mDigFuncBox->setObjectName("digitalFuncBox");

    mDigFuncBox->addItem("Random", QVariant(UiSimulatorConfigDialog::DigitalFunction_Random));
    mDigFuncBox->addItem("I2C", QVariant(UiSimulatorConfigDialog::DigitalFunction_I2C));
    mDigFuncBox->addItem("UART", QVariant(UiSimulatorConfigDialog::DigitalFunction_UART));
    mDigFuncBox->addItem("SPI", QVariant(UiSimulatorConfigDialog::DigitalFunction_SPI));

    formLayout->addRow(tr("Digital: "), mDigFuncBox);

    // analog functions

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mAnFuncBox = new QComboBox(this);
    mAnFuncBox->setObjectName("analogFuncBox");

    mAnFuncBox->addItem("Random", QVariant(UiSimulatorConfigDialog::AnalogFunction_Random));
    mAnFuncBox->addItem("Sine", QVariant(UiSimulatorConfigDialog::AnalogFunction_Sine));

    formLayout->addRow(tr("Analog: "), mAnFuncBox);

    // Deallocation:
    //   Ownership is transfered to UiSimulatorConfigDialog when calling
    //   setLayout below.
    QVBoxLayout* verticalLayout = new QVBoxLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QDialogButtonBox* bottonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok,
                Qt::Horizontal,
                this);
    bottonBox->setCenterButtons(true);

    connect(bottonBox, SIGNAL(accepted()), this, SLOT(accept()));

    verticalLayout->addLayout(formLayout);


    // --- UART settings
    mUartSettings = createUartSettings();
    verticalLayout->addWidget(mUartSettings);


    // --- I2C settings
    mI2cSettings = createI2cSettings();
    verticalLayout->addWidget(mI2cSettings);

    // --- SPI settings
    mSpiSettings = createSpiSettings();
    verticalLayout->addWidget(mSpiSettings);


    verticalLayout->addWidget(bottonBox);

    connect(mDigFuncBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(handleDigitalFunctionChange(int)));

    setLayout(verticalLayout);
}

/*!
    Returns the digital function selected by the user.
*/
UiSimulatorConfigDialog::DigitalFunction UiSimulatorConfigDialog::digitalFunction()
{
    int func = mDigFuncBox->itemData(mDigFuncBox->currentIndex()).toInt();
    return (UiSimulatorConfigDialog::DigitalFunction)func;
}

/*!
    Returns the analog function selected by the user.
*/
UiSimulatorConfigDialog::AnalogFunction UiSimulatorConfigDialog::analogFunction()
{
    int func = mAnFuncBox->itemData(mAnFuncBox->currentIndex()).toInt();
    return (UiSimulatorConfigDialog::AnalogFunction)func;
}

/*!
    Returns signal ID to use for the UART signal.
*/
int UiSimulatorConfigDialog::uartSignalId()
{
    return InputHelper::intValue(mUartSignalBox);
}

/*!
    Returns number of data bits to use for the UART signal.
*/
int UiSimulatorConfigDialog::uartDataBits()
{
    return InputHelper::intValue(mUartDataBitsBox);
}

/*!
    Returns number of stop bits to use for the UART signal.
*/
int UiSimulatorConfigDialog::uartStopBits()
{
    return InputHelper::intValue(mUartStopBitsBox);
}

/*!
    Returns baud rate to use for the UART signal.
*/
int UiSimulatorConfigDialog::uartBaudRate()
{
    return InputHelper::intValue(mUartBaudRate);
}

/*!
    Returns the parity to use for the UART signal.
*/
Types::UartParity UiSimulatorConfigDialog::uartParity()
{
    int f = InputHelper::intValue(mUartParityBox);
    return (Types::UartParity)f;
}

/*!
    Returns signal ID to use for I2C SCL (clock) signal.
*/
int UiSimulatorConfigDialog::i2cSclSignalId()
{
    return InputHelper::intValue(mI2cSclSignalBox);
}

/*!
    Returns signal ID to use for I2C SDA (data) signal.
*/
int UiSimulatorConfigDialog::i2cSdaSignalId()
{
    return InputHelper::intValue(mI2cSdaSignalBox);
}

/*!
    Returns rate/frequency in Hz to use for I2C signal.
*/
int UiSimulatorConfigDialog::i2cRate()
{
    return InputHelper::intValue(mI2cRate);
}

/*!
    Returns address type to use for the I2C signal.
*/
Types::I2CAddress UiSimulatorConfigDialog::i2cAddressType()
{
    int f = InputHelper::intValue(mI2cAddressBox);
    return (Types::I2CAddress)f;
}

/*!
    Returns signal ID to use for the SPI SCK (clock) signal.
*/
int UiSimulatorConfigDialog::spiSckSignalId()
{
    return InputHelper::intValue(mSpiSckSignalBox);
}

/*!
    Returns signal ID to use for the SPI MOSI (master output) signal.
*/
int UiSimulatorConfigDialog::spiMosiSignalId()
{
    return InputHelper::intValue(mSpiMosiSignalBox);
}

/*!
    Returns signal ID to use for the SPI MISO (slave output) signal.
*/
int UiSimulatorConfigDialog::spiMisoSignalId()
{
    return InputHelper::intValue(mSpiMisoSignalBox);
}

/*!
    Returns signal ID to use for the SPI enable (chip-select) signal.
*/
int UiSimulatorConfigDialog::spiEnableSignalId()
{
    return InputHelper::intValue(mSpiEnableSignalBox);
}

/*!
    Returns rate/frequency to use for the SPI signal.
*/
int UiSimulatorConfigDialog::spiRate()
{
    return InputHelper::intValue(mSpiRate);
}

/*!
    Returns the mode to use for the SPI signal.
*/
Types::SpiMode UiSimulatorConfigDialog::spiMode()
{
    int f = InputHelper::intValue(mSpiModeBox);
    return (Types::SpiMode)f;
}

/*!
    Returns the enable mode to use for the SPI signal.
*/
Types::SpiEnable UiSimulatorConfigDialog::spiEnableMode()
{
    int f = InputHelper::intValue(mSpiEnableModeBox);
    return (Types::SpiEnable)f;
}

/*!
    Returns the number of data bits to use for the SPI signal.
*/
int UiSimulatorConfigDialog::spiDataBits()
{
    return InputHelper::intValue(mSpiDataBitsBox);
}

/*!
    Handles a change of digital function.
*/
void UiSimulatorConfigDialog::handleDigitalFunctionChange(int idx)
{
    mUartSettings->hide();
    mI2cSettings->hide();
    mSpiSettings->hide();

    switch (idx) {
    case UiSimulatorConfigDialog::DigitalFunction_I2C:
        mI2cSettings->show();
        break;
    case UiSimulatorConfigDialog::DigitalFunction_UART:
        mUartSettings->show();
        break;
    case UiSimulatorConfigDialog::DigitalFunction_SPI:
        mSpiSettings->show();
        break;
    default:
        break;
    }

    adjustSize();
}

/*!
    Create widget with UART signal settings.
*/
QWidget* UiSimulatorConfigDialog::createUartSettings()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation:
    //   w->setlayout takes ownership of formLayout-
    QFormLayout* formLayout = new QFormLayout;

    mUartSignalBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("UART Signal: "), mUartSignalBox);

    mUartBaudRate = InputHelper::createUartBaudRateBox(this, 115200);
    formLayout->addRow(tr("Baud Rate: "), mUartBaudRate);

    mUartDataBitsBox = InputHelper::createUartDataBitsBox(this, 8);
    formLayout->addRow(tr("Data bits: "), mUartDataBitsBox);

    mUartParityBox = InputHelper::createUartParityBox(this, Types::ParityNone);
    formLayout->addRow(tr("Parity: "), mUartParityBox);

    mUartStopBitsBox = InputHelper::createUartStopBitsBox(this, 1);
    formLayout->addRow(tr("Stop bits: "), mUartStopBitsBox);


    w->hide();
    w->setLayout(formLayout);

    return w;
}

/*!
    Create widget with I2C signal settings.
*/
QWidget* UiSimulatorConfigDialog::createI2cSettings()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation:
    //   w->setLayout takes ownership of formLayout
    QFormLayout* formLayout = new QFormLayout;

    mI2cSclSignalBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("SCL (Clock): "), mI2cSclSignalBox);

    mI2cSdaSignalBox = InputHelper::createSignalBox(this, 1);
    formLayout->addRow(tr("SDA (Data): "), mI2cSdaSignalBox);

    mI2cRate = InputHelper::createI2cRateBox(this, 100000);
    formLayout->addRow(tr("Bit rate: "), mI2cRate);

    mI2cAddressBox = InputHelper::createI2cAddressLengthBox(this, Types::I2CAddress_7bit);
    formLayout->addRow(tr("Address: "), mI2cAddressBox);

    w->hide();
    w->setLayout(formLayout);

    return w;
}

/*!
    Create widget with SPI signal settings.
*/
QWidget* UiSimulatorConfigDialog::createSpiSettings()
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QFrame* w = new QFrame(this);
    w->setFrameShape(QFrame::StyledPanel);

    // Deallocation:
    //    w->setLayout takes ownership of formLayout
    QFormLayout* formLayout = new QFormLayout;

    mSpiSckSignalBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("SCK (Clock): "), mSpiSckSignalBox);

    mSpiMosiSignalBox = InputHelper::createSignalBox(this, 1);
    formLayout->addRow(tr("MOSI: "), mSpiMosiSignalBox);

    mSpiMisoSignalBox = InputHelper::createSignalBox(this, 2);
    formLayout->addRow(tr("MISO: "), mSpiMisoSignalBox);

    mSpiEnableSignalBox = InputHelper::createSignalBox(this, 3);
    formLayout->addRow(tr("Enable (Chip-Select): "), mSpiEnableSignalBox);

    mSpiRate = InputHelper::createSpiRateBox(this, 1000000);
    formLayout->addRow(tr("Bit rate: "), mSpiRate);

    mSpiModeBox = InputHelper::createSpiModeBox(this, Types::SpiMode_0);
    formLayout->addRow(tr("Mode: "), mSpiModeBox);

    mSpiDataBitsBox = InputHelper::createSpiDataBitsBox(this, 8);
    formLayout->addRow(tr("Data bits: "), mSpiDataBitsBox);

    mSpiEnableModeBox = InputHelper::createSpiEnableModeBox(this, Types::SpiEnableLow);
    formLayout->addRow(tr("Enable mode: "), mSpiEnableModeBox);


    w->hide();
    w->setLayout(formLayout);

    return w;
}




