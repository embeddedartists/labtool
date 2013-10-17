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
#ifndef TYPES_H
#define TYPES_H

class Types
{
public:

    enum DataFormat {
        DataFormatHex,
        DataFormatDecimal,
        DataFormatAscii,
        DataFormatNum
    };


    /*
     * U A R T ##############################
     */

    enum UartParity {
        ParityNone,
        ParityOdd,
        ParityEven,
        ParityMark,
        ParitySpace,
        ParityNum // must be last
    };

    /*
     * I 2 C ##############################
     */

    enum I2CAddress {
        I2CAddress_7bit,
        I2CAddress_10bit
    };

    /*
     * S P I ##############################
     */

    enum SpiMode {
        SpiMode_0, //CPOL=0,CPHA=0
        SpiMode_1, //CPOL=0,CPHA=1
        SpiMode_2, //CPOL=1,CPHA=0
        SpiMode_3, //CPOL=1,CPHA=2
        SpiMode_Num // must be last
    };

    enum SpiEnable {
        SpiEnableLow, // active low
        SpiEnableHigh, // active high
        SpiEnableNum // must be last
    };

    Types();
};

#endif // TYPES_H
