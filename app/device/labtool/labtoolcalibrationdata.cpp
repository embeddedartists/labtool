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
#include "labtoolcalibrationdata.h"
#include <string.h>
#include "math.h"

/*!
    \class LabToolCalibrationData
    \brief Holds the calibration data loaded from the LabTool Hardware.

    \ingroup Device

    The LabToolCalibrationData class calculates the scaling factors based
    on the raw calibration data from the LabTool Hardware. The scaling factors
    are used to convert the captured data samples into correctly calibrated
    floating point values in Volts.
*/

/*!
    Constructs a new set of calibration data based on the raw \a data.
*/
LabToolCalibrationData::LabToolCalibrationData(const quint8 *data)
{
    memcpy(&mRawResult, data, sizeof(calib_result));
    mReasonableData = true;

    // Calculate calibration factors
    //
    //   B = (Vin1 - Vin2)/(hex1 - hex2)
    //   A = Vin1 - B*hex1
    //
    // Both A and B must be calculated for each channel and for each V/div setting

    for (int i = 0; i < 8; i++)
    {
        // convert mV to V
        double vin1 = mRawResult.voltsInLow[i] / 1000.0;
        double vin2 = mRawResult.voltsInHigh[i] / 1000.0;

        for (int ch = 0; ch < 2; ch++)
        {
            mCalibB[ch][i] = (vin1 - vin2) / (mRawResult.inLow[ch][i] - mRawResult.inHigh[ch][i]);
            mCalibA[ch][i] = vin1 - mCalibB[ch][i] * mRawResult.inLow[ch][i];

            if (mReasonableData)
            {
                if (!isfinite(mCalibA[ch][i]) || isnan(mCalibA[ch][i])) {
                    mReasonableData = false;
                }
                else if ((mCalibA[ch][i] < -1000) || (mCalibA[ch][i] > 1000))
                {
                    mReasonableData = false;
                }
                else if (!isfinite(mCalibB[ch][i]) || isnan(mCalibB[ch][i]))
                {
                    mReasonableData = false;
                }
                else if ((mCalibB[ch][i] < -1000) || (mCalibB[ch][i] > 1000))
                {
                    mReasonableData = false;
                }
            }
        }
    }
}

/*!
    Prints a table with the raw calibration data for each of the Volts/div levels.
*/
void LabToolCalibrationData::printRawInfo()
{
    qDebug("Got result:");
    qDebug("userOut { { %d, %d, %d }, { %d, %d, %d }",
           mRawResult.userOut[0][0],
           mRawResult.userOut[0][1],
           mRawResult.userOut[0][2],
           mRawResult.userOut[1][0],
           mRawResult.userOut[1][1],
           mRawResult.userOut[1][2]);

    qDebug("               Low               High");
    qDebug(" V/div     mV    A0   A1      mV    A0   A1");
    qDebug("-------  ------ ---- ----   ------ ---- ----");int i = 0;
    qDebug("   20mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("   50mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  100mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  200mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug("  500mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 1000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 2000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
    qDebug(" 5000mV   %5d %4u %4u    %5d %4u %4u", mRawResult.voltsInLow[i], mRawResult.inLow[0][i], mRawResult.inLow[1][i], mRawResult.voltsInHigh[i], mRawResult.inHigh[0][i], mRawResult.inHigh[1][i]);i++;
}

/*!
    Prints a table with the calculated calibration factors for each of the Volts/div levels.
*/
void LabToolCalibrationData::printCalibrationInfo()
{
    int i = 0;
    qDebug("Calibration data:");
    if (isDefaultData()) {
        qDebug("USING DEFAULT DATA - The EEPROM is either empty or contains invalid data!");
    }
    if (!isDataReasonable()) {
        qDebug("Data seems to contain strange values, consider recalibrating!");
    }

    qDebug(" V/div     A0  A      A0  B       A1  A      A1  B   ");
    qDebug("-------  ---------- ----------  ---------- ----------");
    qDebug("   20mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("   50mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  100mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  200mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug("  500mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 1000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 2000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
    qDebug(" 5000mV  %10.7f %10.7f  %10.7f %10.7f", mCalibA[0][i], mCalibB[0][i], mCalibA[1][i], mCalibB[1][i]);i++;
}

/*!
    \fn static int LabToolCalibrationData::rawDataByteSize()

    Returns the size of the raw data structure
*/

/*!
    \fn double LabToolCalibrationData::analogFactorA()

    Returns the A factor for analog input based on which analog channel \a ch and
    it's Volt/div setting \a voltsPerDivIndex
*/

/*!
    \fn double LabToolCalibrationData::analogFactorB()

    Returns the B factor for analog input based on which analog channel \a ch and
    it's Volt/div setting \a voltsPerDivIndex
*/

/*!
    \fn const quint8* LabToolCalibrationData::rawCalibrationData()

    Returns a pointer to the raw calibration data. Used when saving the data in
    the LabTool Hardware's persistant memory.
*/

/*!
    \fn bool LabToolCalibrationData::isDefaultData()

    Returns true if the raw data represents the default settings and not the
    data specific to the connected hardware.
*/

/*!
    \fn bool LabToolCalibrationData::isDataReasonable()

    Does a simple validation of the calibration parameters. Returns true if the
    data passes the validation.
*/

