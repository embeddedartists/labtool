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
#ifndef I2CGENERATOR_H
#define I2CGENERATOR_H

#include <QObject>
#include <QVector>

#include "common/types.h"

class I2CGenerator : public QObject
{
    Q_OBJECT
public:


    explicit I2CGenerator(QObject *parent = 0);
    void setAddressType(Types::I2CAddress type);
    void setI2CRate(int rate);
    int sampleRate();
    bool generateFromString(QString s);
    QVector<int> sclData();
    QVector<int> sdaData();

    
signals:
    
public slots:

private:
    Types::I2CAddress mAddressType;
    int mI2CRate;
    QVector<int> mSclData;
    QVector<int> mSdaData;
    bool mTransfer;

    bool addStart();
    bool addStop();
    bool addAck();
    bool addNack();
    bool addAddressWrite(QString slaveAddress);
    bool addAddressRead(QString slaveAddress);
    bool addData(QString data);
    bool addDelay(QString value);
    bool add8Bits(int value);
    
};

#endif // I2CGENERATOR_H
