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
#include "inputhelper.h"

#include "device/devicemanager.h"
#include "capture/cursormanager.h"

/*!
    \class InputHelper
    \brief This is a helper class used to create input widgets for several
    parts of this application.

    \ingroup Common

*/


/*!
    Constructs the InputHelper.
*/
InputHelper::InputHelper()
{
}

/*!
    Create an input box for selecting digital signals.
*/
QComboBox* InputHelper::createSignalBox(QWidget* parent, int selected)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    CaptureDevice* device = DeviceManager::instance().activeDevice()->captureDevice();

    if (device != NULL) {

        for (int i = 0; i < device->maxNumDigitalSignals(); i++) {
            QString name = device->digitalSignalName(i);

            box->addItem(QString("D%1 - %2").arg(i).arg(name), QVariant(i));
            if (i == selected) {
                box->setCurrentIndex(i);
            }
        }
    }

    return box;
}

/*!
    Create an input box for selecting a cursor (only active cursors are
    shown in the box).
*/
QComboBox* InputHelper::createActiveCursorsBox(QWidget* parent, int selected)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    QMap<UiCursor::CursorId, QString> map = CursorManager::instance().activeCursors();
    QList<UiCursor::CursorId> keys = map.keys();
    box->addItem("", QVariant(UiCursor::NoCursor));
    for (int i = 0; i < keys.size(); i++) {
        QString name = map.value(keys.at(i));
        box->addItem(name, QVariant(keys.at(i)));
        if (keys.at(i) == selected) {
            box->setCurrentIndex(i);
        }
    }

    return box;
}

/*!
    Return integer value associated with specified \a box.
*/
int InputHelper::intValue(QComboBox* box)
{
    return box->itemData(box->currentIndex()).toInt();
}

/*!
    Return integer value associated with specified \a box.
*/
int InputHelper::intValue(QLineEdit* box)
{
    return box->text().toInt();
}

/*!
    Associate an integer \a value with specified \a box.
*/
void InputHelper::setInt(QComboBox* box, int value)
{
    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == value) {
            box->setCurrentIndex(i);
        }
    }
}

/*!
    Create an input box for selecting between data formats.
*/
QComboBox* InputHelper::createFormatBox(QWidget* parent, Types::DataFormat selectedFormat)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("Hex", QVariant(Types::DataFormatHex));
    box->addItem("Decimal", QVariant(Types::DataFormatDecimal));
    box->addItem("Ascii", QVariant(Types::DataFormatAscii));

    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == selectedFormat) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying UART baud rate.
*/
QLineEdit* InputHelper::createUartBaudRateBox(QWidget* parent, int rate)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QLineEdit* box = new QLineEdit(parent);
    box->setText(QString("%1").arg(rate));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QIntValidator* intValidator = new QIntValidator(1, 10000000, box);
    box->setValidator(intValidator);

    return box;
}

/*!
    Create an input box for specifying UART parity.
*/
QComboBox* InputHelper::createUartParityBox(QWidget* parent, Types::UartParity parity)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("None", QVariant(Types::ParityNone));
    box->addItem("Odd", QVariant(Types::ParityOdd));
    box->addItem("Even", QVariant(Types::ParityEven));
    box->addItem("Mark", QVariant(Types::ParityMark));
    box->addItem("Space", QVariant(Types::ParitySpace));

    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == parity) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying UART stop bits.
*/
QComboBox* InputHelper::createUartStopBitsBox(QWidget* parent, int selectedBits)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("1", QVariant(1));
    box->addItem("2", QVariant(2));

    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == selectedBits) {
            box->setCurrentIndex(i);
        }
    }

    return box;
}

/*!
    Create an input box for specifying UART data bits.
*/
QComboBox* InputHelper::createUartDataBitsBox(QWidget* parent, int selectedBits)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("5", QVariant(5));
    box->addItem("6", QVariant(6));
    box->addItem("7", QVariant(7));
    box->addItem("8", QVariant(8));
    box->addItem("9", QVariant(9));

    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == selectedBits) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying I2C rate/frequency.
*/
QLineEdit* InputHelper::createI2cRateBox(QWidget* parent, int rate)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QLineEdit* box = new QLineEdit(parent);
    box->setText(QString("%1").arg(rate));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QIntValidator* intValidator = new QIntValidator(1, 400000, box);
    box->setValidator(intValidator);

    return box;
}

/*!
    Create an input box for specifying I2C address length.
*/
QComboBox* InputHelper::createI2cAddressLengthBox(QWidget* parent, Types::I2CAddress address)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("7-bit", QVariant(Types::I2CAddress_7bit));
    box->addItem("10-bit", QVariant(Types::I2CAddress_10bit));

    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == address) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying SPI rate/frequency.
*/
QLineEdit* InputHelper::createSpiRateBox(QWidget* parent, int rate)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QLineEdit* box = new QLineEdit(parent);
    box->setText(QString("%1").arg(rate));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QIntValidator* intValidator = new QIntValidator(1, 10000000, box);
    box->setValidator(intValidator);

    return box;
}

/*!
    Create an input box for specifying SPI mode.
*/
QComboBox* InputHelper::createSpiModeBox(QWidget* parent, Types::SpiMode mode)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("Mode 0 (CPOL=0, CPHA=0)", QVariant(Types::SpiMode_0));
    box->addItem("Mode 1 (CPOL=0, CPHA=1)", QVariant(Types::SpiMode_1));
    box->addItem("Mode 2 (CPOL=1, CPHA=0)", QVariant(Types::SpiMode_2));
    box->addItem("Mode 3 (CPOL=1, CPHA=1)", QVariant(Types::SpiMode_3));


    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == mode) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying SPI data bits.
*/
QComboBox* InputHelper::createSpiDataBitsBox(QWidget* parent, int selectedBits)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("4", QVariant(4));
    box->addItem("5", QVariant(5));
    box->addItem("6", QVariant(6));
    box->addItem("7", QVariant(7));
    box->addItem("8", QVariant(8));
    box->addItem("9", QVariant(9));
    box->addItem("10", QVariant(10));
    box->addItem("11", QVariant(11));
    box->addItem("12", QVariant(12));
    box->addItem("13", QVariant(13));
    box->addItem("14", QVariant(14));
    box->addItem("15", QVariant(15));
    box->addItem("16", QVariant(16));


    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == selectedBits) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}

/*!
    Create an input box for specifying SPI enable mode.
*/
QComboBox* InputHelper::createSpiEnableModeBox(QWidget* parent, Types::SpiEnable mode)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    QComboBox* box = new QComboBox(parent);

    box->addItem("Active Low", QVariant(Types::SpiEnableLow));
    box->addItem("Active High", QVariant(Types::SpiEnableHigh));


    for (int i = 0; i < box->count(); i++) {

        if (box->itemData(i).toInt() == mode) {
            box->setCurrentIndex(i);
        }
    }


    return box;
}
