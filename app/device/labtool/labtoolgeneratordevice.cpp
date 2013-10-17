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
#include "labtoolgeneratordevice.h"

#include <QDebug>

/*!
    @brief Configuration of the digital signal(s) to generate.

    The \a enabledChannels bit mask represents DIO0..DIO9 and DIO_CLK.
    \private
 */
typedef struct
{
  uint32_t enabledChannels; /*!< using bits 0-10, a 1 means enabled */
  uint32_t frequency;       /*!< Frequency of generated signal */
  uint32_t numStates;       /*!< Bits per channel 1..256 */
  uint32_t patterns[8][11];  /*!< Up to 8*32 states for up to 11 channels */
} gen_sgpio_cfg_t;


/*!
    @brief Configuration of one analog signal to generate.
    \private
 */
typedef struct
{
  /*! @brief Type of waveform to generate.
   *
   * Value | Waveform type
   * :---: | -------------
   *   0   | Sine
   *   1   | Square
   *   2   | Triangle
   *   3   | Sawtooth
   *   4   | Reverse (or inverse) Sawtooth
   *   5   | Level (outputs DC offset, ignores amplitude)
   */
  uint32_t waveform;
  uint32_t frequency; /*!< Frequency in Hz */
  uint32_t amplitude; /*!< Amplitude in mV, 0..5000 */
  int32_t  dcOffset;  /*!< DC offset in mV, -5000..5000 */
} gen_dac_one_ch_cfg_t;

/*!
    @brief Configuration of the analog signal(s) to generate.
    \private
*/
typedef struct
{
  uint32_t              available;  /*!< Bitmask, bit0=ch1, bit1=ch2 */
  gen_dac_one_ch_cfg_t  ch[2];      /*!< Configuration for A_OUT_1 and A_OUT_2 */
} gen_dac_cfg_t;

/*!
    \brief Configuration for signal generation.

    This is the structure that the application sends to
    the LabTool Hardware to configure generation of analog
    and/or digital signals.

    \private
 */
typedef struct
{
  uint32_t         available;  /*!< Bitmask, bit0=SGPIO, bit1=DAC */
  uint32_t         runCounter; /*!< 0=countinuous run, 1=run only once, >1 currently invalid */
  gen_sgpio_cfg_t  sgpio;      /*!< Configuration of digital signals */
  gen_dac_cfg_t    dac;        /*!< Configuration of analog signals */
} generator_cfg_t;

/*!
    \class LabToolGeneratorDevice
    \brief Opens up the Generator functionality of the LabTool Hardware to this application.

    \ingroup Device

    The LabToolGeneratorDevice class provides the interface to the Generator
    functionality of the LabTool Hardware. Generator functionality means
    being able to generate digital and/or analog output signals.
*/

/*!
    Constructs a transfer for the given \a comm.
*/
LabToolGeneratorDevice::LabToolGeneratorDevice(QObject *parent) :
    GeneratorDevice(parent)
{
    mDeviceComm = NULL;
    mData = NULL;
    mContinuousRun = false;
    mDigitalRate = 1;

    mConfigMustBeUpdated = true;
    mLastUsedDigitalRate = 1;
    mLastUsedContinuousRun = false;
}

/*!
    Frees up resources
*/
LabToolGeneratorDevice::~LabToolGeneratorDevice()
{
    if (mData != NULL)
    {
        free(mData);
        mData = NULL;
    }
}

void LabToolGeneratorDevice::start(int digitalRate, bool loop)
{

    mContinuousRun = loop;
    mDigitalRate = digitalRate;

    if (hasConfigChanged())
    {
        mDeviceComm->configureGenerator(configSize(),
                                        configData(digitalRate));
    }
    else
    {
        mDeviceComm->runGenerator();
    }
}

void LabToolGeneratorDevice::stop()
{
    mDeviceComm->stopGenerator();
    emit generateFinished(true, "");
}

/*!
    Sets the communication interface to the LabTool Hardware. Should
    be called with \a comm as NULL when the connection to the LabTool Hardware
    is lost and then with a non-NULL \a comm when the connection has been
    restored.
*/
void LabToolGeneratorDevice::setDeviceComm(LabToolDeviceComm *comm)
{
    if (comm == NULL)
    {
        // lost connection, invalidate last used configuration to force
        // it to be resent to the device when it has been reconnected.
        mConfigMustBeUpdated = true;
    }
    mDeviceComm = comm;
}

/*!
    A report that the LabTool Hardware has stopped as requested.
    Sends the \ref generateFinished signal to indicate success.
*/
void LabToolGeneratorDevice::handleStopped()
{
    qDebug("Generator stopped");
    emit generateFinished(true, "");
}

/*!
    A report that the LabTool Hardware has completed the requested
    configuration update.
    The LabTool Hardware is only configured when the user has selected
    to start signal generation (but had an old configuration) so now that
    the configuration is finished, the actual generation will be started.
*/
void LabToolGeneratorDevice::handleConfigurationDone()
{
    // now that the configuration has been applied, save it
    saveConfig();

    // configuration only done immediately before running, so run now
    mDeviceComm->runGenerator();
}

/*!
    A report that the LabTool Hardware has failed to complete the requested
    configuration update. A \ref generateFinished signal will be sent to
    indicate the failure with an explanation in \a msg.
*/
void LabToolGeneratorDevice::handleConfigurationFailure(const char *msg)
{
    emit generateFinished(false, msg);
}

/*!
    A report that the LabTool Hardware is generating a signal.
    If \ref start was called with \a loop set to true then nothing happens.
    If \a loop was false then a \ref generateFinished signal will be sent to
    indicate that the one-shot generation was successfully completed.

    This is a bit tricky when only generating analog signals as they don't
    have any one-shot mode. Regardless if the analog signal generation was
    started with \a loop true or false, it will continue until \ref stop
    is called.
*/
void LabToolGeneratorDevice::handleRunning()
{
    if (mLastUsedContinuousRun) {
        qDebug("Generator running...");
    } else {
        emit generateFinished(true, "");
    }
}

/*!
    A report that the LabTool Hardware has failed to generate the signal
    as requested. A \ref generateFinished signal will be sent to
    indicate the failure with an explanation in \a msg.
*/
void LabToolGeneratorDevice::handleRunningFailure(const char *msg)
{
    emit generateFinished(false, msg);
}


/*!
    Number of bytes in the configuration data to send to the LabTool Hardware.
*/
unsigned int LabToolGeneratorDevice::configSize()
{
    return sizeof(generator_cfg_t);
}

/*!
    Prepares and returns the configuration data to send to the LabTool Hardware.
    The function is only called when the configuration has changed so there
    is no need to cache the changes.

    The data is returned as a byte array, but is actually one \a generator_cfg_t
    structure. The signal independant information is filled in here and then
    \ref updateDigitalConfigData and \ref updateAnalogConfigData are called
    to fill in the signal specific parts.
*/
quint8* LabToolGeneratorDevice::configData(int digitalRate)
{
    if (mData == NULL) {
        mData = (uchar*)malloc(sizeof(generator_cfg_t));
    }

    memset(mData, 0, sizeof(generator_cfg_t));

    // Configure common parts
    generator_cfg_t* common_header = (generator_cfg_t*)mData;
    if (!mContinuousRun) {
        common_header->runCounter = 1;
    }

    if (isDigitalGeneratorEnabled() && !digitalSignals().empty())
    {
        common_header->available |= (1<<0);
        updateDigitalConfigData(digitalRate);
    }
    if (isAnalogGeneratorEnabled() && !analogSignals().empty())
    {
        common_header->available |= (1<<1);
        updateAnalogConfigData();
    }

    return mData;
}

/*!
    Fills in the configuration of the digital signals in the \a gen_sgpio_cfg_t
    part of the \a generator_cfg_t to send to the LabTool Hardware.
*/
void LabToolGeneratorDevice::updateDigitalConfigData(int digitalRate)
{
    generator_cfg_t* common_header = (generator_cfg_t*)mData;
    gen_sgpio_cfg_t* digital_header = &common_header->sgpio;

    digital_header->frequency = digitalRate;

    QList<DigitalSignal*> signalList = digitalSignals();
    foreach(DigitalSignal* s, signalList)
    {
        int ch = s->id();
        digital_header->enabledChannels |= (1 << ch);
        QVector<bool> data = s->data();
        int numStates = s->numStates();
        quint32 tmp = 0;
        int pos = 0;
        int i;
        for (i = 0; i < numStates; i++)
        {
            tmp = (tmp >> 1);
            if (data.at(i)) {
                tmp |= 0x80000000;
            }
            if ((i % 32) == 31)
            {
                digital_header->patterns[pos][ch] = tmp;
                pos++;
                tmp = 0;
            }
        }
        if (tmp > 0)
        {
            // As data is added MSB first it has to be shifted down.
            // This only happens when the number of states isn't a
            // multiple of 32.
            for (int k = (i % 32); k < 32; k++) {
                tmp = (tmp >> 1);
            }
            digital_header->patterns[pos][ch] = tmp;
        }
        digital_header->numStates = numStates;
    }
}

/*!
    Fills in the configuration of the analog signals in the \a gen_dac_cfg_t
    part of the \a generator_cfg_t to send to the LabTool Hardware.
*/
void LabToolGeneratorDevice::updateAnalogConfigData()
{
    generator_cfg_t* common_header = (generator_cfg_t*)mData;
    gen_dac_cfg_t* analog_header = &common_header->dac;

    QList<AnalogSignal*> signalList = analogSignals();
    foreach(AnalogSignal* s, signalList)
    {
        int id = s->id();
        if (id < maxNumAnalogSignals()) {
            analog_header->available |= (1 << id);

            analog_header->ch[id].amplitude = s->amplitude()*1000;
            analog_header->ch[id].frequency = s->frequency();
            analog_header->ch[id].waveform = s->waveform();
            analog_header->ch[id].dcOffset = 0; /*! \todo Add DC Offset to GUI */
        }
    }
}

/*!
    Returns true if any change has been made to the configuration.
    Used to determine if the LabTool Hardware's configuration must
    be updated before a new generation can be started.
*/
bool LabToolGeneratorDevice::hasConfigChanged()
{
    do {
        if (mConfigMustBeUpdated) break;

        if (mLastUsedDigitalRate != mDigitalRate) break;
        if (mLastUsedContinuousRun != mContinuousRun) break;

        if (mLastUsedDigtalSignals.size() != mDigitalSignalList.size()) break;
        if (mLastUsedAnalogSignals.size() != mAnalogSignalList.size()) break;


        bool changed = false;
        foreach(DigitalSignal* signal, mDigitalSignalList) {
            if (!mLastUsedDigtalSignals.contains(*signal)) {
                changed = true;
                break;
            }
        }
        if (changed) break;


        foreach(AnalogSignal* signal, mAnalogSignalList) {
            if (!mLastUsedAnalogSignals.contains(*signal)) {
                changed = true;
                break;
            }
        }
        if (changed) break;


        return false;

    } while (false);

    return true;
}

/*!
    Saves a copy of the current configuration. The copy is used by
    \ref hasConfigChanged to determine when the LabTool Hardware's
    configuration is out-of-date.
*/
void LabToolGeneratorDevice::saveConfig()
{
    mLastUsedDigitalRate = mDigitalRate;
    mLastUsedContinuousRun = mContinuousRun;
    mConfigMustBeUpdated = false;

    mLastUsedDigtalSignals.clear();
    foreach(DigitalSignal* signal, mDigitalSignalList) {
        mLastUsedDigtalSignals.append(*signal);
    }

    mLastUsedAnalogSignals.clear();
    foreach(AnalogSignal* signal, mAnalogSignalList) {
        mLastUsedAnalogSignals.append(*signal);
    }
}

