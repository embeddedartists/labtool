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
#include "uartgenerator.h"

/*!
    \class UartGenerator
    \brief This is a helper class that can generate valid digital data
    for the UART protocol.

    \ingroup Generator

*/

/*!
    Constructs the UartGenerator with the given \a parent.
*/
UartGenerator::UartGenerator(QObject *parent) :
    QObject(parent)
{
    mBaudRate = 115200;
    mNumDataBits = 8;
    mNumStopBits = 1;
    mParity = Types::ParityNone;
}

/*!
    Sets baud rate to \a rate.
*/
void UartGenerator::setBaudRate(int rate)
{
    mBaudRate = rate;
}

/*!
    Sets number of data bits to \a numBits.
*/
void UartGenerator::setDataBits(int numBits)
{
    mNumDataBits = numBits;
}

/*!
    Sets number of stop bits to \a numBits.
*/
void UartGenerator::setStopBits(int numBits)
{
    mNumStopBits = numBits;
}

/*!
    Sets parity to \a parity.
*/
void UartGenerator::setParity(Types::UartParity parity)
{
    mParity = parity;
}

/*!
    Generate UART signal using specified \a data.
*/
bool UartGenerator::generate(QByteArray &data)
{
    int numOnes = 0;

    mUartData.clear();

    // idle line -> high
    mUartData.append(1);

    for (int i = 0; i < data.size(); i++) {

        // start bit
        mUartData.append(0);

        // data
        numOnes = addData(data.at(i));

        // parity
        addParity(numOnes);

        // stop bit(s)
        for (int j = 0; j < mNumStopBits; j++) {
            mUartData.append(1);
        }

    }

    // idle line -> high
    mUartData.append(1);

    return true;
}

/*!
    Returns UART signal data.
*/
QVector<int> UartGenerator::uartData()
{
    return mUartData;
}

/*!
    Returns sample rate.
*/
int UartGenerator::sampleRate()
{
    // same as baud rate
    return mBaudRate;
}

/*!
    Add parity
*/
void UartGenerator::addParity(int numOnes)
{

    switch (mParity) {
    case Types::ParityNone:
        break;
    case Types::ParityOdd:
        if ((numOnes % 2) == 0) {
            mUartData.append(1);
        } else {
            mUartData.append(0);
        }
        break;
    case Types::ParityEven:
        if ((numOnes % 2) == 0) {
            mUartData.append(0);
        } else {
            mUartData.append(1);
        }
        break;
    case Types::ParityMark:
        mUartData.append(1);
        break;
    case Types::ParitySpace:
        mUartData.append(0);
        break;
    default:
        break;
    }

}

/*!
    Add data.
*/
int UartGenerator::addData(char data)
{
    int numOnes = 0;
    // LSB first (do we also need to support MSB first)
    for (int i = 0; i < mNumDataBits; i++) {
        if ( (data & (1<<i)) != 0) {
            mUartData.append(1);
            numOnes++;
        }
        else {
            mUartData.append(0);
        }
    }

    return numOnes;
}
