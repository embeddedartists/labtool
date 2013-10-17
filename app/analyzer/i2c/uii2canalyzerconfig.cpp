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
#include "uii2canalyzerconfig.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QMessageBox>

#include "common/inputhelper.h"
#include "device/devicemanager.h"

/*!
    \class UiI2CAnalyzerConfig
    \brief Dialog window used to configure the I2C analyzer.

    \ingroup Analyzer

*/


/*!
    Constructs the UiI2CAnalyzerConfig with the given \a parent.
*/
UiI2CAnalyzerConfig::UiI2CAnalyzerConfig(QWidget *parent) :
    UiAnalyzerConfig(parent)
{
    setWindowTitle(tr("I2C Analyzer"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    // Deallocation: Re-parented when calling verticalLayout->addLayout
    QFormLayout* formLayout = new QFormLayout;


    mSclBox = InputHelper::createSignalBox(this, 0);
    formLayout->addRow(tr("Clock (SCL): "), mSclBox);

    mSdaBox = InputHelper::createSignalBox(this, 1);
    formLayout->addRow(tr("Data (SDA): "), mSdaBox);

    mFormatBox = InputHelper::createFormatBox(this, Types::DataFormatHex);
    formLayout->addRow(tr("Data format: "), mFormatBox);

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
    Returns the SCL signal ID.
*/
int UiI2CAnalyzerConfig::sclSignalId()
{
    return InputHelper::intValue(mSclBox);
}

/*!
    Returns the SDA signal ID.
*/
int UiI2CAnalyzerConfig::sdaSignalId()
{
    return InputHelper::intValue(mSdaBox);
}

/*!
    Set the SCL signal ID to \a id.
*/
void UiI2CAnalyzerConfig::setSclSignalId(int id)
{
    InputHelper::setInt(mSclBox, id);
}

/*!
    Set the SDA signal ID to \a id.
*/
void UiI2CAnalyzerConfig::setSdaSignalId(int id)
{
    InputHelper::setInt(mSdaBox, id);
}

/*!
    Set the data format to \a format.
*/
void UiI2CAnalyzerConfig::setDataFormat(Types::DataFormat format)
{   
    InputHelper::setInt(mFormatBox, (int)format);
}

/*!
    Returns the data format.
*/
Types::DataFormat UiI2CAnalyzerConfig::dataFormat()
{
    int f = InputHelper::intValue(mFormatBox);
    return (Types::DataFormat)f;
}

/*!
    Verify that the choices are valid.
*/
void UiI2CAnalyzerConfig::verifyChoice()
{
    int scl = InputHelper::intValue(mSclBox);
    int sda = InputHelper::intValue(mSdaBox);

    if (scl != sda) {
        accept();
    }
    else {
        QMessageBox::warning(
                    this,
                    tr("Invalid choice"),
                    tr("SCL and SDA can't use the same signal"));
    }

}

/*!
    Returns the cursor to use for synchronization.
*/
UiCursor::CursorId UiI2CAnalyzerConfig::syncCursor()
{
    return (UiCursor::CursorId)InputHelper::intValue(mCursorBox);
}

/*!
    Set the cursor to use for synchronization.
*/
void UiI2CAnalyzerConfig::setSyncCursor(UiCursor::CursorId id)
{
    InputHelper::setInt(mCursorBox, id);
}


