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
#ifndef LABTOOLCAPTUREDEVICE_H
#define LABTOOLCAPTUREDEVICE_H

#include <QObject>
#include <QList>

#include "device/capturedevice.h"
#include "labtooldevicecomm.h"
#include "uilabtooltriggerconfig.h"

class LabToolCaptureDevice : public CaptureDevice
{
    Q_OBJECT
public:
    explicit LabToolCaptureDevice(QObject *parent = 0);
    ~LabToolCaptureDevice();

    QList<int> supportedSampleRates();
    int maxNumDigitalSignals();
    int maxNumAnalogSignals();
    QList<double> supportedVPerDiv();
    bool supportsContinuousCapture() {return true;}

    void configureTrigger(QWidget* parent);
    void calibrate(QWidget* parent);
    void start(int sampleRate);
    void stop();

    int lastSampleIndex();
    QVector<int>* digitalData(int signalId);
    void setDigitalData(int signalId, QVector<int> data);
    QVector<double>* analogData(int signalId);
    void setAnalogData(int signalId, QVector<double> data);

    void clearSignalData();

    int digitalTriggerIndex();
    void setDigitalTriggerIndex(int idx);
    void digitalTransitions(int signalId, QList<int> &list);

    void reconfigure(int sampleRate = -1);

    void setDeviceComm(LabToolDeviceComm* comm);

signals:

private slots:
    void handleStopped();
    void handleConfigurationDone();
    void handleConfigurationFailure(const char* msg);
    void handleReceivedSamples(LabToolDeviceTransfer* transfer, unsigned int size, unsigned int trigger, unsigned int digitalTrigSample, unsigned int analogTrigSample, unsigned int digitalChannelInfo, unsigned int analogChannelInfo);
    void handleFailedCapture(const char* msg);
    void handleReconfigurationTimer();

private:

    enum Constants {
        MaxDigitalSignals = 11,
        MaxAnalogSignals = 2
    };

    UiLabToolTriggerConfig* mTriggerConfig;
    LabToolDeviceComm*  mDeviceComm;

    int mEndSampleIdx;
    int mTriggerIndex;
    int mRequestedSampleRate;
    bool mConfigMustBeUpdated;
    bool mRunningCapture;
    bool mReconfigurationRequested;
    bool mWarnUncalibrated;
    quint8* mData;
    QList<DigitalSignal> mLastUsedDigtalSignals;
    QList<AnalogSignal> mLastUsedAnalogSignals;
    int mLastUsedSampleRate;

    QVector<int>* mDigitalSignals[MaxDigitalSignals];
    QVector<double>* mAnalogSignals[MaxAnalogSignals];
    QVector<quint16>* mAnalogSignalData[MaxAnalogSignals];
    QList<int>* mDigitalSignalTransitions[MaxDigitalSignals];

    QList<double> mSupportedVPerDiv;

    QTimer* mReconfigTimer;

    int locateFirstLevel(QVector<int> *s, int level, int offset);
    int locatePreviousLevel(QVector<int> *s, int level, int offset);

    int locateAnalogHighLowTransition(QVector<double> *s, double lowLevel, double highLevel, int offset);
    int locatePreviousAnalogHighLowTransition(QVector<double> *s, double lowLevel, double highLevel, int offset);
    int locateAnalogLowHighTransition(QVector<double> *s, double lowLevel, double highLevel, int offset);
    int locatePreviousAnalogLowHighTransition(QVector<double> *s, double lowLevel, double highLevel, int offset);

    bool detectAnalogSignalFrequency(int id, quint16 trigLevel, bool fallingEdge);
    void convertDigitalInput(const quint8* pData, quint32 size, quint32 activeChannels, quint32 trig, int digitalTrigSample, int analogTrigSample);
    void unpackAnalogInput(const quint8 *pData, quint32 size, quint32 activeChannels);
    void convertHiddenAnalogInput(const quint8 *pData, quint32 size);
    void convertAnalogInput(const quint8* pData, quint32 size, quint32 activeChannels, quint32 trig, int analogTrigSample, int digitalTrigSample);
    void saveData(const quint8* pData, quint32 size);
    void deleteSignals();

    qint16 analog12BitTriggerLevel(const AnalogSignal *signal);

    bool hasConfigChanged();
    void saveConfig();

    unsigned int configSize();
    quint8 *configData();
    void updateDigitalConfigData();
    void updateAnalogConfigData();

};

#endif // LABTOOLDEVICE_H
