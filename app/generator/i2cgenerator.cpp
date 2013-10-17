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
#include "i2cgenerator.h"

#include <QDebug>
#include <QStringList>

/*!
    \class I2CGenerator
    \brief This is a helper class that can generate valid digital data
    for the I2C protocol.

    \ingroup Generator

*/

/*!
    Constructs the I2CGenerator with the given \a parent.
*/
I2CGenerator::I2CGenerator(QObject *parent) :
    QObject(parent)
{
    mAddressType = Types::I2CAddress_7bit;
    mI2CRate = 100000; // 100 KHz
    mTransfer = false;
}

/*!
    Sets the I2C address type to \a type.
*/
void I2CGenerator::setAddressType(Types::I2CAddress type)
{
    mAddressType = type;
}

/*!
    Sets the I2C rate/frequency to \a rate.
*/
void I2CGenerator::setI2CRate(int rate)
{
    if (rate <= 0 || rate > 400000) return;

    mI2CRate = rate;
}

/*!
    Get sample rate for the current configuration.
*/
int I2CGenerator::sampleRate()
{
    // two samples for each clock cycle (low-high transition)
    return mI2CRate*2;
}

/*!
    Generate I2C signal data based on the provided string \a s. The string
    should be formatted as below

    S    = Start
    P    = Stop
    A    = ACK
    N    = NACK
    Wddd = Address (7 or 10 bit) - Write
    Rddd = Address (7 or 10 bit) - Read
    Xdd  = transmitted data (8-bits)
    Ddd  = Delay in number of clock cycles
    d    = hexadecimal digit (0-F)

    Example:
    D04,S,W060,A,X16,A,X00,A,X00,A,X00,A,X40,A,P,S,W060,A,X00,A,P,S,R060,A,X3F,N,P,S,W060,A,X01,A,P,S,R060,A,X7F,N,P

*/
bool I2CGenerator::generateFromString(QString s)
{
    bool success = true;

    // reset data
    mSclData.clear();
    mSdaData.clear();

    mTransfer = false;

    QStringList list = s.split(',', QString::SkipEmptyParts);
    for (int i = 0; i < list.size(); i++) {
        QString tok = list.at(i);

        switch(tok.at(0).toLatin1()) {
        case 'S':
            mTransfer = true;
            addStart();
            break;
        case 'P':
            mTransfer = false;
            addStop();
            break;
        case 'A':
            addAck();
            break;
        case 'N':
            addNack();
            break;
        case 'W':
            success = addAddressWrite(tok.mid(1));
            break;
        case 'R':
            success = addAddressRead(tok.mid(1));
            break;
        case 'X':
            success = addData(tok.mid(1));
            break;
        case 'D':
            success = addDelay(tok.mid(1));
            break;
        }

        if (!success) break;
    }

    return success;
}

/*!
    Get I2C SCL (clock) signal data.
*/
QVector<int> I2CGenerator::sclData()
{
    return mSclData;
}

/*!
    Get I2C SDA (data) signal data.
*/
QVector<int> I2CGenerator::sdaData()
{
    return mSdaData;
}

/*!
    Add a start condition
*/
bool I2CGenerator::addStart()
{
    // start condition: SDA high-low transition while SCL is high

    // SDA is low, must set it to high so a transition can take place
    if (mSdaData.size() > 1 && mSdaData.at(mSdaData.size()-1) == 0) {


        if (mSclData.at(mSclData.size()-1) == 1) {
            mSclData.append(0);
            mSdaData.append(1);

            mSclData.append(1);
            mSdaData.append(1);
        }
        else {
            mSclData.append(1);
            mSdaData.append(1);
        }
    }


    mSdaData.append(0);
    mSclData.append(1);

    return true;
}

/*!
    Add a stop condition
*/
bool I2CGenerator::addStop()
{
    // stop condition: an SDA low-high transition while SCL is high

    // SDA is high, must set it low so a transition can take place
    if (mSdaData.size() > 1 && mSdaData.at(mSdaData.size()-1) == 1) {

        // the transition must take place when SCL is high (not during SCL
        // transition)


        if (mSclData.at(mSclData.size()-1) == 1) {

            // add one clock cycle where SDA is LOW

            mSclData.append(0);
            mSdaData.append(0);

            mSclData.append(1);
            mSdaData.append(0);
        }
        else {
            mSclData.append(1);
            mSdaData.append(0);
        }
    }

    mSdaData.append(1);
    mSclData.append(1);

    return true;
}

/*!
    Add an ACK
*/
bool I2CGenerator::addAck()
{
    // ACK: keep SDA low during a clock cycle
    mSclData.append(0);
    mSclData.append(1);

    mSdaData.append(0);
    mSdaData.append(0);

    return true;
}

/*!
    Add a NACK
*/
bool I2CGenerator::addNack()
{

    // ACK: keep SDA high during a clock cycle
    mSclData.append(0);
    mSclData.append(1);

    mSdaData.append(1);
    mSdaData.append(1);

    return true;
}

/*!
    Add address write request for address specified by \a slaveAddress
*/
bool I2CGenerator::addAddressWrite(QString slaveAddress)
{
    bool success = false;

    do {
        if (slaveAddress.size() != 3) break;

        int value = slaveAddress.toInt(&success, 16);

        if (!success) break;

        if (mAddressType == Types::I2CAddress_7bit) {
            // Write -> R/W bit = 0
            add8Bits((value << 1) & 0xFE);
        }
        else {

            // 10-bit address

            /*
                    - The 7 first bits of the first byte are the combination 1111 0XX
                      of which the last two bits are the two most-significant bits of
                      the 10-bit address; the eight bit of the first byte is the R/W
                      bit.
                    - As always a byte is followed an Acknowledge bit
                    - The second byte is the 8 least-significant bits of the 10-bit
                      address.
            */

            int v = 0x78;
            v |= ((value & 0x300) >> 8);
            add8Bits((v << 1) & 0xFE);
            addAck();
            add8Bits(value & 0xff);

        }



        success = true;

    } while(0);


    return success;
}

/*!
    Add address read request for address specified by \a slaveAddress
*/
bool I2CGenerator::addAddressRead(QString slaveAddress)
{
    bool success = false;

    do {
        if (slaveAddress.size() != 3) break;

        int value = slaveAddress.toInt(&success, 16);

        if (!success) break;

        // Read -> R/W bit = 0
        add8Bits((value << 1) | 0x01);

        success = true;

    } while(0);


    return success;
}

/*!
    Add data as specified by \a data.
*/
bool I2CGenerator::addData(QString data)
{
    bool success = false;

    do {
        if (data.size() != 2) break;

        int value = data.toInt(&success, 16);

        if (!success) break;

        add8Bits(value);

        success = true;

    } while(0);


    return success;
}

/*!
    Add a delay
*/
bool I2CGenerator::addDelay(QString value)
{
    bool success = true;

    do {

        if (value.size() != 2) break;

        int samples = value.toInt(&success, 16);

        if (!success) break;

        for (int i = 0; i < samples; i++) {

            // SCL is high when there isn't any active transfer; otherwise 0
            if (mTransfer) {
                mSclData.append(0);
            } else {
                mSclData.append(1);
            }

            // always keep SDA high
            mSdaData.append(1);
        }
    } while(0);

    return success;
}

/*!
    Add an 8-bit data value
*/
bool I2CGenerator::add8Bits(int value)
{
    int level;
    int mask = 0x80;
    for (int i = 0; i < 8; i++) {

        level = 0;
        if ((value & mask) != 0) {
            level = 1;
        }

        // clock cycle
        mSclData.append(0);
        mSclData.append(1);

        mSdaData.append(level);
        mSdaData.append(level);

        mask >>= 1;
    }

    return true;
}
