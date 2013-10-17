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
#ifndef UARTGENERATOR_H
#define UARTGENERATOR_H

#include <QObject>
#include <QVector>

#include "common/types.h"

class UartGenerator : public QObject
{
    Q_OBJECT
public:

    explicit UartGenerator(QObject *parent = 0);

    void setBaudRate(int rate);
    void setDataBits(int numBits);
    void setStopBits(int numBits);
    void setParity(Types::UartParity parity);

    bool generate(QByteArray &data);
    QVector<int> uartData();
    int sampleRate();

    
signals:
    
public slots:

private:
    int mBaudRate;
    int mNumDataBits;
    int mNumStopBits;
    Types::UartParity mParity;

    QVector<int> mUartData;

    void addParity(int numOnes);
    int addData(char data);
    
};

#endif // UARTGENERATOR_H
