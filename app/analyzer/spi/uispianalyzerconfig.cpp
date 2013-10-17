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
#include "uispianalyzerconfig.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "common/inputhelper.h"


/*!
    \class UiSpiAnalyzerConfig
    \brief Dialog window used to configure the SPI analyzer.

    \ingroup Analyzer

*/


/*!
    Constructs the UiSpiAnalyzerConfig with the given \a parent.
*/
UiSpiAnalyzerConfig::UiSpiAnalyzerConfig(QWidget *parent) :
    UiAnalyzerConfig(parent)
{
    setWindowTitle(tr("SPI Analyzer"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation: Re-parented when calling verticalLayout->addLayout
    QFormLayout* formLayout = new QFormLayout;

    mSpiSckSignalBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("SCK (Clock): "), mSpiSckSignalBox);

    mSpiMosiSignalBox = InputHelper::createSignalBox(this, 1);
    formLayout->addRow(tr("MOSI: "), mSpiMosiSignalBox);

    mSpiMisoSignalBox = InputHelper::createSignalBox(this, 2);
    formLayout->addRow(tr("MISO: "), mSpiMisoSignalBox);

    mSpiEnableSignalBox = InputHelper::createSignalBox(this, 3);
    formLayout->addRow(tr("Enable (Chip-Select): "), mSpiEnableSignalBox);

    mFormatBox = InputHelper::createFormatBox(this, Types::DataFormatHex);
    formLayout->addRow(tr("Data format: "), mFormatBox);

    mSpiModeBox = InputHelper::createSpiModeBox(this, Types::SpiMode_0);
    formLayout->addRow(tr("Mode: "), mSpiModeBox);

    mSpiDataBitsBox = InputHelper::createSpiDataBitsBox(this, 8);
    formLayout->addRow(tr("Data bits: "), mSpiDataBitsBox);

    mSpiEnableModeBox = InputHelper::createSpiEnableModeBox(this, Types::SpiEnableLow);
    formLayout->addRow(tr("Enable mode: "), mSpiEnableModeBox);

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

    connect(bottonBox, SIGNAL(accepted()), this, SLOT(verifyChoice()));

    verticalLayout->addLayout(formLayout);
    verticalLayout->addWidget(bottonBox);


    setLayout(verticalLayout);
}

/*!
    Set the SCK signal ID to \a id.
*/
void UiSpiAnalyzerConfig::setSckSignal(int id)
{
    InputHelper::setInt(mSpiSckSignalBox, id);
}

/*!
    Returns the SCK signal ID.
*/
int UiSpiAnalyzerConfig::sckSignal()
{
    return InputHelper::intValue(mSpiSckSignalBox);
}

/*!
    Set the MOSI signal ID to \a id.
*/
void UiSpiAnalyzerConfig::setMosiSignal(int id)
{
    InputHelper::setInt(mSpiMosiSignalBox, id);
}

/*!
    Returns the MOSI signal ID.
*/
int UiSpiAnalyzerConfig::mosiSignal()
{
    return InputHelper::intValue(mSpiMosiSignalBox);
}

/*!
    Set the MISO signal ID to \a id.
*/
void UiSpiAnalyzerConfig::setMisoSignal(int id)
{
    InputHelper::setInt(mSpiMisoSignalBox, id);
}

/*!
    Returns the MISO signal ID.
*/
int UiSpiAnalyzerConfig::misoSignal()
{
    return InputHelper::intValue(mSpiMisoSignalBox);
}

/*!
    Set the Enable (CS) signal ID to \a id.
*/
void UiSpiAnalyzerConfig::setEnableSignal(int id)
{
    InputHelper::setInt(mSpiEnableSignalBox, id);
}

/*!
    Returns the Enable signal ID.
*/
int UiSpiAnalyzerConfig::enableSignal()
{
    return InputHelper::intValue(mSpiEnableSignalBox);
}

/*!
    Set the SPI mode to \a mode.
*/
void UiSpiAnalyzerConfig::setMode(Types::SpiMode mode)
{
    InputHelper::setInt(mSpiModeBox, (int)mode);
}

/*!
    Returns the SPI mode.
*/
Types::SpiMode UiSpiAnalyzerConfig::mode()
{
    int f = InputHelper::intValue(mSpiModeBox);
    return (Types::SpiMode)f;
}

/*!
    Set the Enable mode to \a mode.
*/
void UiSpiAnalyzerConfig::setEnableMode(Types::SpiEnable mode)
{
    InputHelper::setInt(mSpiEnableModeBox, (int)mode);
}

/*!
    Returns the enable mode.
*/
Types::SpiEnable UiSpiAnalyzerConfig::enableMode()
{
    int f = InputHelper::intValue(mSpiEnableModeBox);
    return (Types::SpiEnable)f;
}

/*!
    Set number of data bits to \a bits.
*/
void UiSpiAnalyzerConfig::setDataBits(int bits)
{
    InputHelper::setInt(mSpiDataBitsBox, bits);
}

/*!
    Returns the number of data bits.
*/
int UiSpiAnalyzerConfig::dataBits()
{
    return InputHelper::intValue(mSpiDataBitsBox);
}

/*!
    Returns the data format.
*/
Types::DataFormat UiSpiAnalyzerConfig::dataFormat()
{
    int f = InputHelper::intValue(mFormatBox);
    return (Types::DataFormat)f;
}

/*!
    Set the data format to \a format.
*/
void UiSpiAnalyzerConfig::setDataFormat(Types::DataFormat format)
{
    InputHelper::setInt(mFormatBox, (int)format);
}

/*!
    Verify the choices.
*/
void UiSpiAnalyzerConfig::verifyChoice()
{
    bool unique = true;
    int s[4];

    s[0] = InputHelper::intValue(mSpiSckSignalBox);
    s[1] = InputHelper::intValue(mSpiMosiSignalBox);
    s[2] = InputHelper::intValue(mSpiMisoSignalBox);
    s[3] = InputHelper::intValue(mSpiEnableSignalBox);

    for (int i = 0; i < 4; i++) {
        for (int j= 0; j < 4; j++) {
            if (i == j) continue;

            if (s[i] == s[j]) {
                unique = false;
                break;
            }

        }
    }


    if (unique) {
        accept();
    }
    else {
        QMessageBox::warning(
                    this,
                    tr("Invalid choice"),
                    tr("Signals must be unique"));
    }

}

/*!
    Returns the cursor used for synchronization.
*/
UiCursor::CursorId UiSpiAnalyzerConfig::syncCursor()
{
    return (UiCursor::CursorId)InputHelper::intValue(mCursorBox);
}

/*!
    Set the cursor to use for synchronization.
*/
void UiSpiAnalyzerConfig::setSyncCursor(UiCursor::CursorId id)
{
    InputHelper::setInt(mCursorBox, id);
}
