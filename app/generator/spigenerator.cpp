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
#include "spigenerator.h"

#include <QStringList>

/*!
    \class SpiGenerator
    \brief This is a helper class that can generate valid digital data
    for the SPI protocol.

    \ingroup Generator

*/

/*!
    Constructs the SpiGenerator with the given \a parent.
*/
SpiGenerator::SpiGenerator(QObject *parent) :
    QObject(parent)
{
    mRate = 1000000;
    mDataBits = 8;
    mMode = Types::SpiMode_0;
    mEnable = Types::SpiEnableLow;
    mEnableOn = false;
}

/*!
    Set the SPI rate to \a rate.
*/
void SpiGenerator::setSpiRate(int rate)
{
    mRate = rate;
}

/*!
    Set the number of data bits to \a numBits.
*/
void SpiGenerator::setDataBits(int numBits)
{
    mDataBits = numBits;
}

/*!
    Set the SPI mode to \a mode.
*/
void SpiGenerator::setSpiMode(Types::SpiMode mode)
{
    mMode = mode;
}

/*!
    Set the enable mode to \a mode.
*/
void SpiGenerator::setEnableMode(Types::SpiEnable mode)
{
    mEnable = mode;
}

/*!
    Generate SPI signal data based on the provided string \a s. The string
    should be formatted as below

    E1   = Enable signal on
    E0   = Enable signal off
    Xdd:dd = data -> MOSI:MISO
    Ddd  = Delay in number of clock cycles
    d    = hexadecimal digit (0-F)

    Example:
    D04,E1,D03,XD1:00,XFF:19,XFF:00,D02,E0,D03,E1,D02,X91:00,XFF:64,XFF:18,D02,E0

*/
bool SpiGenerator::generateFromString(QString s)
{
    bool success = true;

    // reset data
    mSckData.clear();
    mMosiData.clear();
    mMisoData.clear();
    mCsData.clear();
    mEnableOn = false;


    QStringList list = s.split(',', QString::SkipEmptyParts);
    for (int i = 0; i < list.size(); i++) {
        QString tok = list.at(i);

        switch(tok.at(0).toLatin1()) {
        case 'E':
            success = addEnable(tok.mid(1));
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
    \fn int SpiGenerator::sampleRate()

    Returns the sample rate for this configuration.
*/

/*!
    \fn QVector<int> SpiGenerator::sckData()

    Returns the SCK signal data.
*/

/*!
    \fn QVector<int> SpiGenerator::mosiData()

    Returns the MOSI signal data.
*/

/*!
    \fn QVector<int> SpiGenerator::misoData()

    Returns the MISO signal data.
*/

/*!
    \fn QVector<int> SpiGenerator::enableData()

    Returns the SPI signal data.
*/


/*!
    Add enable state with \a value. The parameter \a onlyEnable is
    set to true of only the enable signal should be affected.
*/
bool SpiGenerator::addEnable(QString value, bool onlyEnable)
{
    bool success = true;

    do {

        if (value.size() != 1) break;

        int e = value.toInt(&success, 10);

        if (!success) break;
        if (!(e == 0 || e == 1)) break;

        mEnableOn = (e == 1);

        // active low
        if ((mEnableOn && mEnable == Types::SpiEnableLow)
                || (!mEnableOn && mEnable == Types::SpiEnableHigh)) {
            mCsData.append(0);
        }

        // active high
        else {
            mCsData.append(1);
        }

        if (!onlyEnable) {
            mMosiData.append(0);
            mMisoData.append(0);

            if (mMode == Types::SpiMode_0 || mMode == Types::SpiMode_1) {
                // CPOL == 0
                mSckData.append(0);
            }
            else {
                // CPOL == 1
                mSckData.append(1);
            }

        }


    } while(0);

    return success;
}

/*!
    Add data
*/
bool SpiGenerator::addData(QString value)
{
    bool success = true;

    do {

        QStringList parts = value.split(':');

        if (parts.size() != 2) break;

        int mosi = 0xff;
        if (parts.at(0).size() > 0) {
            mosi = parts.at(0).toInt(&success, 16);
        }
        if (!success) break;

        int miso = 0;
        if (parts.at(1).size() > 0) {
            miso = parts.at(1).toInt(&success, 16);
        }
        if (!success) break;

        addBits(mosi, miso);

    } while(0);

    return success;
}

/*!
    Add a delay.
*/
bool SpiGenerator::addDelay(QString value)
{
    bool success = true;

    do {

        if (value.size() != 2) break;

        int samples = value.toInt(&success, 16);

        if (!success) break;

        for (int i = 0; i < samples; i++) {


            if (mMode == Types::SpiMode_0 || mMode == Types::SpiMode_1) {
                mSckData.append(0);
            }
            else {
                mSckData.append(1);
            }


            mMosiData.append(0);
            mMisoData.append(0);

            if (mEnableOn) {
                addEnable("1", true);
            }
            else {
                addEnable("0", true);
            }

        }
    } while(0);

    return success;
}

/*!
    Add MOSI and MISO data according to \a mosi and \a miso.
*/
void SpiGenerator::addBits(int mosi, int miso)
{

    int mosiLevel;
    int misoLevel;
    int mask = (1<<(mDataBits-1));

    for (int i = 0; i < mDataBits; i++) {

        mosiLevel = 0;
        if ((mosi & mask) != 0) {
            mosiLevel = 1;
        }
        misoLevel = 0;
        if ((miso & mask) != 0) {
            misoLevel = 1;
        }
#if 0
        // clock cycle
        if (mMode == Types::SpiMode_0 || mMode == Types::SpiMode_1) {
            mSckData.append(1);
            mSckData.append(0);
        }
        else {
            mSckData.append(0);
            mSckData.append(1);
        }

        mMosiData.append(mosiLevel);
        mMosiData.append(mosiLevel);

        mMisoData.append(misoLevel);
        mMisoData.append(misoLevel);
#endif

        switch(mMode) {
        case Types::SpiMode_0: // CPOL=0, CPHA=0
            mSckData.append(0);
            mSckData.append(1);
            break;
        case Types::SpiMode_1: // CPOL=0, CPHA=1
            mSckData.append(1);
            mSckData.append(0);
            break;
        case Types::SpiMode_2: // CPOL=1, CPHA=0
            mSckData.append(1);
            mSckData.append(0);
            break;
        case Types::SpiMode_3: // CPOL=1, CPHA=1
            mSckData.append(0);
            mSckData.append(1);
            break;
        default:
            break;
        }


        mMosiData.append(mosiLevel);
        mMosiData.append(mosiLevel);

        mMisoData.append(misoLevel);
        mMisoData.append(misoLevel);

        // CS enable
        addEnable("1", true);
        addEnable("1", true);

        mask >>= 1;


    }
}
