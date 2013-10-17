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
#include "simulatorcapturedevice.h"

#include <QDebug>

#include <QtGlobal>
#include <qmath.h>

#include "generator/i2cgenerator.h"
#include "generator/uartgenerator.h"
#include "generator/spigenerator.h"

/*!
    \class SimulatorCaptureDevice
    \brief Allows the user to test the Capture functionality of this application.

    \ingroup Device

*/

/*!
    Constructs a capture device with the given \a parent.
*/
SimulatorCaptureDevice::SimulatorCaptureDevice(QObject *parent) :
    CaptureDevice(parent)
{       
    mConfigDialog = NULL;

    mEndSampleIdx = 0;
    mUsedSampleRate = 1;
    mTriggerIdx = 0;

    for (int i = 0; i < MaxDigitalSignals; i++) {
        mDigitalSignals[i] = NULL;
        mDigitalSignalTransitions[i] = NULL;
    }

    for (int i = 0; i < MaxAnalogSignals; i++) {
        mAnalogSignals[i] = NULL;
    }

}

/*!
    Deletes generated signal data.
*/
SimulatorCaptureDevice::~SimulatorCaptureDevice()
{
    deleteSignalData();
}


QList<int> SimulatorCaptureDevice::supportedSampleRates()
{
    // supported sample rates in Hz
    return QList<int>()
            << 100000000
            <<  50000000
            <<  20000000
            <<  10000000
            <<   5000000
            <<   2000000
            <<   1000000
            <<    500000
            <<    200000
            <<    100000
            <<     50000
            <<     20000
            <<     10000
            <<      5000
            <<      2000
            <<      1000;
}

int SimulatorCaptureDevice::maxNumDigitalSignals()
{
    return MaxDigitalSignals;
}

int SimulatorCaptureDevice::maxNumAnalogSignals()
{
    return MaxAnalogSignals;
}

QList<double> SimulatorCaptureDevice::supportedVPerDiv()
{
    if (mSupportedVPerDiv.size() == 0) {
        for(double i = 0.1; i < 5.0; i+= 0.1) {
            mSupportedVPerDiv.append(i);
        }
    }

    return mSupportedVPerDiv;
}

void SimulatorCaptureDevice::configureBeforeStart(QWidget* parent)
{
    if (mConfigDialog == NULL) {
        // Deallocation: "Qt Object trees" (See UiMainWindow)
        mConfigDialog = new UiSimulatorConfigDialog(parent);
    }

    mConfigDialog->exec();
}

void SimulatorCaptureDevice::start(int sampleRate)
{
    mEndSampleIdx = 0;

    if (mConfigDialog != NULL) {

        mEndSampleIdx = numberOfSamples() - 1;
        mUsedSampleRate = sampleRate;


        switch(mConfigDialog->digitalFunction()) {
        case UiSimulatorConfigDialog::DigitalFunction_Random:
            generateRandomDigitalSignals();
            break;
        case UiSimulatorConfigDialog::DigitalFunction_I2C:
            generateI2CDigitalSignals();
            break;
        case UiSimulatorConfigDialog::DigitalFunction_UART:
            generateUartDigitalSignals();
            break;
        case UiSimulatorConfigDialog::DigitalFunction_SPI:
            generateSpiDigitalSignals();
            break;

        }

        switch(mConfigDialog->analogFunction()) {
        case UiSimulatorConfigDialog::AnalogFunction_Random:
            generateRandomAnalogSignals();
            break;
        case UiSimulatorConfigDialog::AnalogFunction_Sine:
            generateSineAnalogSignals();
            break;

        }


    }

#if 0
    int r = qrand();
    int scale = 1;
    if (mEndSampleIdx > RAND_MAX) {
        scale = mEndSampleIdx/RAND_MAX;
    }
    int ret = (r*scale % mEndSampleIdx);

//    qDebug() <<  "getTrigger: pos="<<ret << " r="<< r << " endIdx="<<mEndSampleIdx << " max=" <<RAND_MAX;

    mTriggerIdx = ret;
#else
    mTriggerIdx = 0;
#endif

    emit captureFinished(true, "");
}

void SimulatorCaptureDevice::stop()
{
    emit captureFinished(true, "");
}

int SimulatorCaptureDevice::lastSampleIndex()
{
    return mEndSampleIdx;
}

QVector<int>* SimulatorCaptureDevice::digitalData(int signalId)
{
    QVector<int>* data = NULL;

    if (signalId < MaxDigitalSignals) {
        data = mDigitalSignals[signalId];
    }

    return data;
}

void SimulatorCaptureDevice::setDigitalData(int signalId, QVector<int> data)
{
    if (signalId < MaxDigitalSignals) {

        if (mDigitalSignals[signalId] != NULL) {
            delete mDigitalSignals[signalId];
            mDigitalSignals[signalId] = NULL;
        }

        if (data.size() > 0) {
            mEndSampleIdx = data.size();

            // Deallocation:
            //    Deleted by deleteSignalData() which is called by destructor
            //    or clearSignalData()
            mDigitalSignals[signalId] = new QVector<int>(data);
        }

    }
}

QVector<double>* SimulatorCaptureDevice::analogData(int signalId)
{
    QVector<double>* data = NULL;

    if (signalId < MaxAnalogSignals) {
        data = mAnalogSignals[signalId];
    }

    return data;
}

void SimulatorCaptureDevice::setAnalogData(int signalId, QVector<double> data)
{
    if (signalId < MaxAnalogSignals) {

        if (mAnalogSignals[signalId] != NULL) {
            delete mAnalogSignals[signalId];
            mAnalogSignals[signalId] = NULL;
        }

        if (data.size() > 0) {
            mEndSampleIdx = data.size();

            // Deallocation:
            //    Deleted by deleteSignalData() which is called by destructor
            //    or clearSignalData()
            mAnalogSignals[signalId] = new QVector<double>(data);
        }

    }
}

void SimulatorCaptureDevice::clearSignalData()
{
    deleteSignalData();
}

int SimulatorCaptureDevice::digitalTriggerIndex()
{
    return mTriggerIdx;
}

void SimulatorCaptureDevice::setDigitalTriggerIndex(int idx)
{
    mTriggerIdx = idx;
}

void SimulatorCaptureDevice::digitalTransitions(int signalId, QList<int> &list)
{

    if (signalId >= MaxDigitalSignals) return;
    if (mDigitalSignals[signalId] == NULL) return;

    // Not in cache. Create the list
    if (mDigitalSignalTransitions[signalId] == NULL) {

        // Deallocation:
        //    Deleted by deleteSignalData() which is called by destructor
        //    or clearSignalData()
        QList<int>* l = new QList<int>();

        CaptureDevice::digitalTransitions(signalId, *l);
        mDigitalSignalTransitions[signalId] = l;
    }

    list = *mDigitalSignalTransitions[signalId];
}

void SimulatorCaptureDevice::reconfigure(int sampleRate)
{
    (void)sampleRate;
}


/*!
    Returns number of samples to use when generating signals.
*/
int SimulatorCaptureDevice::numberOfSamples()
{
    // calculate number of samples based on number of digital signals
    // and a buffer of 256kB. Each digital signal requires 1 bit
    int sz = mDigitalSignalList.size();
    if (sz <= 0) sz = 1;

    return ((262144*8)/sz);
}

/*!
    Generate random digital signal data
*/
void SimulatorCaptureDevice::generateRandomDigitalSignals()
{

    foreach(DigitalSignal* signal, mDigitalSignalList) {
        int id = signal->id();

        if (id >= MaxDigitalSignals) continue;

        int maxNumSamples = numberOfSamples();


        // Deallocation:
        //    Assigned to mDigitalSignals below which is deleted by
        //    deleteSignalData() which is called by destructor or
        //    clearSignalData()
        QVector<int> *s = new QVector<int>();
        bool fast = ((qrand() % 2) == 1);

        if (fast) {
            for(int j = 0; j < maxNumSamples; ++j) {
                s->append(qrand() % 2);
            }
        }
        else {

            for(int j = 0; j < maxNumSamples;) {
                int level = qrand() % 2;
                int parts = (qrand() % 1020) + 4;
                int duration = qrand() % (maxNumSamples/parts);
                duration = j+duration;
                if (duration>maxNumSamples) {
                    duration = maxNumSamples;
                }

                while(j < duration) {
                    s->append(level);
                    j++;
                }

            }

        }

        int skips = qrand() % 5478;
        for (int j = 0; j < skips; j++) qrand();

        if (mDigitalSignals[id] != NULL) {
            delete mDigitalSignals[id];
        }
        if (mDigitalSignalTransitions[id] != NULL) {
            delete mDigitalSignalTransitions[id];
            mDigitalSignalTransitions[id] = NULL;
        }

        mDigitalSignals[id] = s;

    }
}

/*!
    Generate I2C digital signal data
*/
void SimulatorCaptureDevice::generateI2CDigitalSignals()
{

    if (mDigitalSignalList.size() < 2 || mConfigDialog == NULL) return;

    I2CGenerator i2cGen;
    i2cGen.setAddressType(mConfigDialog->i2cAddressType());
    i2cGen.setI2CRate(mConfigDialog->i2cRate());
    i2cGen.generateFromString("D04,S,W060,A,X16,A,X00,A,X00,A,X00,A,X40,A,P,S,W060,A,X00,A,P,S,R060,A,X3F,N,P,S,W060,A,X01,A,P,S,R060,A,X7F,N,P");

    double i2cSampleTime = (double)1/i2cGen.sampleRate();
    QVector<int> sclData = i2cGen.sclData();
    QVector<int> sdaData = i2cGen.sdaData();

    if (sclData.size() < 2) return;

    // Deallocation:
    //    Deleted by deleteSignalData() which is called by destructor or
    //    clearSignalData()
    QVector<int> *scl = new QVector<int>();
    QVector<int> *sda = new QVector<int>();


    int maxNumSamples = numberOfSamples();
    double sampleTime = (double)1/mUsedSampleRate;

    int i2cPos = 0;
    double nextI2CTime = i2cSampleTime;

    for (int i = 0; i < maxNumSamples; i++) {

        while (i*sampleTime >= nextI2CTime) {
            i2cPos++;
            nextI2CTime = (i2cPos+1)*i2cSampleTime;
        }

        if (i2cPos < sclData.size()) {
            scl->append(sclData.at(i2cPos));
            sda->append(sdaData.at(i2cPos));
        }
        else {
            scl->append(scl->at(i-1));
            sda->append(sda->at(i-1));
        }


    }

    setDigitalSignalData(mConfigDialog->i2cSclSignalId(), scl);
    setDigitalSignalData(mConfigDialog->i2cSdaSignalId(), sda);

}

/*!
    Generate UART digital signal data
*/
void SimulatorCaptureDevice::generateUartDigitalSignals()
{

    if (mDigitalSignalList.size() < 1 || mConfigDialog == NULL) return;

    UartGenerator uartGen;
    uartGen.setBaudRate(mConfigDialog->uartBaudRate());
    uartGen.setDataBits(mConfigDialog->uartDataBits());
    uartGen.setStopBits(mConfigDialog->uartStopBits());
    uartGen.setParity(mConfigDialog->uartParity());

    QByteArray dataToGen = QString("Hello World abcde fghij klmno pqrst uvwxy z0123 45678 9").toLatin1();
    uartGen.generate(dataToGen);

    double uartSampleTime = (double)1/uartGen.sampleRate();
    QVector<int> uartData = uartGen.uartData();

    if (uartData.size() < 2) return;

    // Deallocation:
    //    Deleted by deleteSignalData() which is called by destructor or
    //    clearSignalData()
    QVector<int> *data = new QVector<int>();


    int maxNumSamples = numberOfSamples();
    double sampleTime = (double)1/mUsedSampleRate;

    int pos = 0;
    double nextTime = uartSampleTime;

    for (int i = 0; i < maxNumSamples; i++) {

        while (i*sampleTime >= nextTime) {
            pos++;
            nextTime = (pos+1)*uartSampleTime;
        }

        if (pos < uartData.size()) {
            data->append(uartData.at(pos));
        }
        else {
            data->append(data->at(i-1));
        }


    }

    setDigitalSignalData(mConfigDialog->uartSignalId(), data);
}

/*!
    Generate SPI digital signal data
*/
void SimulatorCaptureDevice::generateSpiDigitalSignals()
{
    if (mDigitalSignalList.size() < 4 || mConfigDialog == NULL) return;

    SpiGenerator spiGen;
    spiGen.setSpiMode(mConfigDialog->spiMode());
    spiGen.setSpiRate(mConfigDialog->spiRate());
    spiGen.setDataBits(mConfigDialog->spiDataBits());
    spiGen.setEnableMode(mConfigDialog->spiEnableMode());

    spiGen.generateFromString("D04,E1,D03,XD1:00,XFF:19,XFF:00,D02,E0,D03,E1,D02,X91:00,XFF:64,XFF:18,D02,E0");

    double spiSampleTime = (double)1/spiGen.sampleRate();
    QVector<int> sckData = spiGen.sckData();
    QVector<int> mosiData = spiGen.mosiData();
    QVector<int> misoData = spiGen.misoData();
    QVector<int> csData = spiGen.enableData();

    if (sckData.size() < 2) return;

    // Deallocation:
    //    Deleted by deleteSignalData() which is called by destructor or
    //    clearSignalData()
    QVector<int> *sck = new QVector<int>();
    QVector<int> *mosi = new QVector<int>();
    QVector<int> *miso = new QVector<int>();
    QVector<int> *cs = new QVector<int>();


    int maxNumSamples = numberOfSamples();
    double sampleTime = (double)1/mUsedSampleRate;

    int pos = 0;
    double nextTime = spiSampleTime;

    for (int i = 0; i < maxNumSamples; i++) {

        while (i*sampleTime >= nextTime) {
            pos++;
            nextTime = (pos+1)*spiSampleTime;
        }

        if (pos < sckData.size()) {
            sck->append(sckData.at(pos));
            mosi->append(mosiData.at(pos));
            miso->append(misoData.at(pos));
            cs->append(csData.at(pos));
        }
        else {
            sck->append(sck->at(i-1));
            mosi->append(mosi->at(i-1));
            miso->append(miso->at(i-1));
            cs->append(cs->at(i-1));

        }


    }

    setDigitalSignalData(mConfigDialog->spiSckSignalId(), sck);
    setDigitalSignalData(mConfigDialog->spiMosiSignalId(), mosi);
    setDigitalSignalData(mConfigDialog->spiMisoSignalId(), miso);
    setDigitalSignalData(mConfigDialog->spiEnableSignalId(), cs);
}

/*!
    Generate random analog signal data
*/
void SimulatorCaptureDevice::generateRandomAnalogSignals()
{

    foreach(AnalogSignal* signal, mAnalogSignalList) {
        int id = signal->id();
        if (id >= MaxAnalogSignals) continue;

        int maxNumSamples = numberOfSamples();

        // Deallocation:
        //    Deleted by deleteSignalData() which is called by destructor or
        //    clearSignalData()
        QVector<double> *s = new QVector<double>();

        for(int j = 0; j < maxNumSamples; ++j) {

            // random number between -5.0 and +5.0
            double val = qrand() % 1000;
            val -= 500;
            val /= 100.0;

            s->append(val);
        }

        int skips = qrand() % 5478;
        for (int j = 0; j < skips; j++) qrand();

        if (mAnalogSignals[id] != NULL) {
            delete mAnalogSignals[id];
        }

        mAnalogSignals[id] = s;
    }
}

/*!
    Generate analog signal data with sine waveform
*/
void SimulatorCaptureDevice::generateSineAnalogSignals()
{

    double pi = 3.14159265;
    int maxNumSamples = numberOfSamples();

    foreach(AnalogSignal* signal, mAnalogSignalList) {
        int id = signal->id();

        if (id >= MaxAnalogSignals) continue;

        // Deallocation:
        //    Deleted by deleteSignalData() which is called by destructor or
        //    clearSignalData()
        QVector<double> *s = new QVector<double>();

        double amp = qrand() % 1000;
        amp -= 500;
        amp /= 100.0;

        int per = (qrand() % (maxNumSamples/32));

        for(int j = 0; j < maxNumSamples; j++) {

            double val = amp*qSin(2*pi*j/per);

            s->append(val);
        }

        if (mAnalogSignals[id] != NULL) {
            delete mAnalogSignals[id];
        }

        mAnalogSignals[id] = s;
    }
}

/*!
    Delete generated signal data.
*/
void SimulatorCaptureDevice::deleteSignalData()
{
    for (int i = 0; i < MaxDigitalSignals; i++) {
        if (mDigitalSignals[i] != NULL) {
            delete mDigitalSignals[i];
            mDigitalSignals[i] = NULL;
        }

        if (mDigitalSignalTransitions[i] != NULL) {
            delete mDigitalSignalTransitions[i];
            mDigitalSignalTransitions[i] = NULL;
        }
    }

    for (int i = 0; i < MaxAnalogSignals; i++) {
        if (mAnalogSignals[i] != NULL) {
            delete mAnalogSignals[i];
            mAnalogSignals[i] = NULL;
        }
    }
}

/*!
    Set digital signal data to \a data for signal with given \a id.
*/
void SimulatorCaptureDevice::setDigitalSignalData(int id, QVector<int>* data)
{
    if (mDigitalSignals[id] != NULL) {
        delete mDigitalSignals[id];
    }
    if (mDigitalSignalTransitions[id] != NULL) {
        delete mDigitalSignalTransitions[id];
        mDigitalSignalTransitions[id] = NULL;
    }
    mDigitalSignals[id] = data;
}
