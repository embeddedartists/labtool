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
#ifndef LABTOOLCALIBRATIONDATA_H
#define LABTOOLCALIBRATIONDATA_H

#include <qglobal.h>
#include <QString>

class LabToolCalibrationData
{
private:

    /*! \brief Raw calibration data.
     * This is part of the \ref calib_result_t structure that is read from
     * the LabTool Hardware.
     *
     * \private
     */
    struct calib_result
    {
      quint32 cmd;            /*!< Marker used by the protocol */
      quint32 checksum;       /*!< Checksum to assure correct read/write to EEPROM */
      quint32 version;        /*!< Future proof the data by adding a version number */
      quint32 dacValOut[3];   /*!< DAC values in 10-bit format used for calibration of analog out */
      int     userOut[2][3];  /*!< User's measured analog output in mV for dacValOut's values */

      int     voltsInLow[8];  /*!< Analog output values in mV used for calibration of analog in for each V/div */
      int     voltsInHigh[8]; /*!< Analog output values in mV used for calibration of analog in for each V/div */
      quint32 inLow[2][8];    /*!< Measured analog in for each channel and V/div combo at low output*/
      quint32 inHigh[2][8];   /*!< Measured analog in for each channel and V/div combo at high output*/
    };

    double mCalibA[2][8];
    double mCalibB[2][8];
    calib_result mRawResult;
    bool mReasonableData;

public:
    LabToolCalibrationData(const quint8* data);

    static int rawDataByteSize() { return sizeof(calib_result); }

    double analogFactorA(int ch, int voltsPerDivIndex) { return mCalibA[ch][voltsPerDivIndex]; }
    double analogFactorB(int ch, int voltsPerDivIndex) { return mCalibB[ch][voltsPerDivIndex]; }

    const quint8* rawCalibrationData() { return (const quint8*)&mRawResult; }

    bool isDefaultData() { return (mRawResult.checksum == 0x00dead00 || mRawResult.version == 0x00dead00); }
    bool isDataReasonable() { return mReasonableData; }

    void printRawInfo();
    void printCalibrationInfo();
};

#endif // LABTOOLCALIBRATIONDATA_H
