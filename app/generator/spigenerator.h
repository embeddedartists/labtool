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
#ifndef SPIGENERATOR_H
#define SPIGENERATOR_H

#include <QObject>
#include <QVector>

#include "common/types.h"

class SpiGenerator : public QObject
{
    Q_OBJECT
public:
    explicit SpiGenerator(QObject *parent = 0);

    void setSpiRate(int rate);
    void setDataBits(int numBits);
    void setSpiMode(Types::SpiMode mode);
    void setEnableMode(Types::SpiEnable mode);

    bool generateFromString(QString s);

    int sampleRate() {return mRate*2;}
    QVector<int> sckData() {return mSckData;}
    QVector<int> mosiData() {return mMosiData;}
    QVector<int> misoData() {return mMisoData;}
    QVector<int> enableData() {return mCsData;}
    
signals:
    
public slots:

private:
    int mRate;
    int mDataBits;
    Types::SpiMode mMode;
    Types::SpiEnable mEnable;

    QVector<int> mSckData;
    QVector<int> mMosiData;
    QVector<int> mMisoData;
    QVector<int> mCsData;

    bool mEnableOn;

    bool addEnable(QString value, bool onlyEnable=false);
    bool addData(QString value);
    bool addDelay(QString value);
    void addBits(int mosi, int miso);
    
};

#endif // SPIGENERATOR_H
