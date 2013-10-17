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
#include "uiuartanalyzerconfig.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>

#include "common/inputhelper.h"

/*!
    \class UiUartAnalyzerConfig
    \brief Dialog window used to configure the UART analyzer.

    \ingroup Analyzer

*/


/*!
    Constructs the UiUartAnalyzerConfig with the given \a parent.
*/
UiUartAnalyzerConfig::UiUartAnalyzerConfig(QWidget *parent) :
    UiAnalyzerConfig(parent)
{
    setWindowTitle(tr("UART Analyzer"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation: Re-parented when calling verticalLayout->addLayout
    QFormLayout* formLayout = new QFormLayout;

    mSignalBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("UART Signal: "), mSignalBox);

    mFormatBox = InputHelper::createFormatBox(this, Types::DataFormatHex);
    formLayout->addRow(tr("Data format: "), mFormatBox);

    mBaudRate = InputHelper::createUartBaudRateBox(this, 115200);
    formLayout->addRow(tr("Baud Rate: "), mBaudRate);

    mDataBitsBox = InputHelper::createUartDataBitsBox(this, 8);
    formLayout->addRow(tr("Data bits: "), mDataBitsBox);

    mParityBox = InputHelper::createUartParityBox(this, Types::ParityNone);
    formLayout->addRow(tr("Parity: "), mParityBox);

    mStopBitsBox = InputHelper::createUartStopBitsBox(this, 1);
    formLayout->addRow(tr("Stop bits: "), mStopBitsBox);

    mCursorBox = InputHelper::createActiveCursorsBox(this, UiCursor::NoCursor);
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QLabel* cursorLbl = new QLabel(tr("Synchronize: "), this);
    cursorLbl->setToolTip(tr("Start to analyze from a cursor position"));
    formLayout->addRow(cursorLbl, mCursorBox);


    // Deallocation: Ownership changed when calling setLayout
    QVBoxLayout* verticalLayout = new QVBoxLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QDialogButtonBox* bottonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok,
                Qt::Horizontal,
                this);
    bottonBox->setCenterButtons(true);

    connect(bottonBox, SIGNAL(accepted()), this, SLOT(accept()));

    verticalLayout->addLayout(formLayout);
    verticalLayout->addWidget(bottonBox);


    setLayout(verticalLayout);
}

/*!
    Returns the selected signal ID.
*/
int UiUartAnalyzerConfig::signalId()
{
    return InputHelper::intValue(mSignalBox);
}

/*!
    Set the signal ID to \a id.
*/
void UiUartAnalyzerConfig::setSignalId(int id)
{
    InputHelper::setInt(mSignalBox, id);
}

/*!
    Set the data format to \a format.
*/
void UiUartAnalyzerConfig::setDataFormat(Types::DataFormat format)
{
    InputHelper::setInt(mFormatBox, (int)format);
}

/*!
    Returns the data format.
*/
Types::DataFormat UiUartAnalyzerConfig::dataFormat()
{
    int f = InputHelper::intValue(mFormatBox);
    return (Types::DataFormat)f;
}

/*!
    Set the baud rate to \a rate.
*/
void UiUartAnalyzerConfig::setBaudRate(int rate)
{
    mBaudRate->setText(QString("%1").arg(rate));
}

/*!
    Returns the selected baud rate.
*/
int UiUartAnalyzerConfig::baudRate()
{
    return mBaudRate->text().toInt();
}

/*!
    Set parity to \a parity.
*/
void UiUartAnalyzerConfig::setParity(Types::UartParity parity)
{
    InputHelper::setInt(mParityBox, (int)parity);
}

/*!
    Returns selected parity.
*/
Types::UartParity UiUartAnalyzerConfig::parity()
{
    int f = InputHelper::intValue(mParityBox);
    return (Types::UartParity)f;
}

/*!
    Set number of stop bits to \a bits.
*/
void UiUartAnalyzerConfig::setStopBits(int bits)
{
    InputHelper::setInt(mStopBitsBox, bits);
}

/*!
    Returns selected number of stop bits.
*/
int UiUartAnalyzerConfig::stopBits()
{
    return InputHelper::intValue(mStopBitsBox);
}

/*!
    Set number of data bits to \a bits.
*/
void UiUartAnalyzerConfig::setDataBits(int bits)
{
    InputHelper::setInt(mDataBitsBox, bits);
}

/*!
    Returns selected number of data bits.
*/
int UiUartAnalyzerConfig::dataBits()
{
    return InputHelper::intValue(mDataBitsBox);
}

/*!
    Returns the cursor used for synchronization.
*/
UiCursor::CursorId UiUartAnalyzerConfig::syncCursor()
{
    return (UiCursor::CursorId)InputHelper::intValue(mCursorBox);
}

/*!
    Sets the cursor used for synchronization.
*/
void UiUartAnalyzerConfig::setSyncCursor(UiCursor::CursorId id)
{
    InputHelper::setInt(mCursorBox, id);
}





