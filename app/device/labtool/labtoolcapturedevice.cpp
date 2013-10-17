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
#include "labtoolcapturedevice.h"

#include <QDebug>
#include <QFile>
#include <QTimer>

#include "labtoolcalibrationwizard.h"


/*! @brief Configuration for digital signal capture.
 * This is part of the \ref capture_cfg_t structure that the client software
 * must send to configure capture of analog and/or digital signals.
 * \private
 */
typedef struct
{
  /*! @brief Which digital signals should be sampled.
   *
   * Bit assignment (0 = not sampled, 1 = sampled):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for \a DIO_0
   *    1   | Setting for \a DIO_1
   *    2   | Setting for \a DIO_2
   *    3   | Setting for \a DIO_3
   *    4   | Setting for \a DIO_4
   *    5   | Setting for \a DIO_5
   *    6   | Setting for \a DIO_6
   *    7   | Setting for \a DIO_7
   *    9   | Setting for \a DIO_9
   *   10   | Setting for \a DIO_CLK
   *  11-31 | Reserved
   */
  uint32_t enabledChannels;

  /*! @brief Which digital signals have triggering conditions.
   *
   * Bit assignment (0 = not triggering, 1 = have trigger condition):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for \a DIO_0
   *    1   | Setting for \a DIO_1
   *    2   | Setting for \a DIO_2
   *    3   | Setting for \a DIO_3
   *    4   | Setting for \a DIO_4
   *    5   | Setting for \a DIO_5
   *    6   | Setting for \a DIO_6
   *    7   | Setting for \a DIO_7
   *    9   | Setting for \a DIO_9
   *   10   | Setting for \a DIO_CLK
   *  11-31 | Reserved
   */
  uint32_t enabledTriggers;

  /*! @brief Trigger information.
   *
   * Two bits are used per channel:
   * - 00 = falling edge
   * - 01 = rising edge
   * - 10 = high level
   * - 11 = low level
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-1  | Setting for \a DIO_0
   *   2-3  | Setting for \a DIO_1
   *   4-5  | Setting for \a DIO_2
   *   6-7  | Setting for \a DIO_3
   *   8-9  | Setting for \a DIO_4
   *  10-11 | Setting for \a DIO_5
   *  12-13 | Setting for \a DIO_6
   *  14-15 | Setting for \a DIO_7
   *  16-17 | Setting for \a DIO_8
   *  18-19 | Setting for \a DIO_9
   *  20-21 | Setting for \a DIO_CLK
   *  22-31 | Reserved
   */
  uint32_t triggerSetup;
} cap_sgpio_cfg_t;

/*! \brief Configuration for analog signal capture.
 * This is part of the \ref capture_cfg_t structure that the client software
 * must send to configure capture of analog and/or digital signals.
 *
 * \private
 */
typedef struct
{
  /*! @brief Which analog signals should be sampled.
   *
   * Bit assignment (0 = not sampled, 1 = sampled):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t enabledChannels;

  /*! @brief Which analog signals have triggering conditions.
   *
   * Bit assignment (0 = not triggering, 1 = have trigger condition):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t enabledTriggers;

  /*! @brief Trigger information (ignored if the trigger is not enabled).
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-11 | Trigger level for ch0
   *  12-13 | Reserved
   *  14-15 | 00 = rising edge, 01 = falling edge
   *  16-27 | Trigger level for ch1
   *  28-29 | Reserved
   *  30-31 | 00 = rising edge, 01 = falling edge
   */
  uint32_t triggerSetup;

  /*! @brief Volts/div configuration.
   * Values are indices in the \a VDIV_CONFIG table* found in \ref capture_vadc.c.
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-3  | Index for ch0
   *   4-7  | Index for ch1
   *   8-31 | Reserved
   */
  uint32_t voltPerDiv;

  /*! @brief AC/DC coupling information.
   *
   * Bit assignment (0 = DC, 1 = AC):
   *
   *  Bits  | Description
   *  :---: | -----------
   *    0   | Setting for ch0
   *    1   | Setting for ch1
   *   2-31 | Reserved
   */
  uint32_t couplings;

  /*! @brief Noise suppression.
   * The same filter is applied to both channels.
   *
   * Bit assignment:
   *
   *  Bits  | Description
   *  :---: | -----------
   *   0-3  | Index for ch0
   *   4-7  | Index for ch1
   *   8-31 | Reserved
   */
  uint32_t noiseReduction;
} cap_vadc_cfg_t;

/*! @brief Configuration for signal capture.
 * This is the structure that the client software must send to configure
 * capture of analog and/or digital signals.
 * \private
 */
typedef struct
{
  uint32_t         numEnabledSGPIO; /*!< Number of enabled digital signals */
  uint32_t         numEnabledVADC;  /*!< Number of enabled analog signals */
  uint32_t         sampleRate;      /*!< Wanted sample rate */

  /*! Post fill configuration. The lower 8 bits specify the percent of the
   * maximum buffer size that will be used for samples taken AFTER the trigger.
   * The upper 24 bits specifies the maximum number of samples to gather after
   * a trigger has been found.*/
  uint32_t         postFill;

  cap_sgpio_cfg_t  sgpio;  /*!< Configuration of digital signals */
  cap_vadc_cfg_t   vadc;   /*!< Configuration of analog signals */
} capture_cfg_t;



/*!
    \class LabToolCaptureDevice
    \brief Opens up the Capture functionality of the LabTool Hardware to this application.

    \ingroup Device

    The LabToolCaptureDevice class provides the interface to the Capture
    functionality of the LabTool Hardware. Capture functionality means being
    able to sample digital and/or analog signals at a given sample rate.
*/

/*!
    Constructs a capture device with the given \a parent.
*/
LabToolCaptureDevice::LabToolCaptureDevice(QObject *parent) :
    CaptureDevice(parent)
{
    // Deallocation: Destructor is responsible
    mData = (uchar*)malloc(sizeof(capture_cfg_t));

    // Deallocation: Destructor is responsible
    mTriggerConfig = new UiLabToolTriggerConfig();
    mConfigMustBeUpdated = true;

    mDeviceComm = NULL;
    mEndSampleIdx = 0;
    mTriggerIndex = 0;
    mReconfigTimer = NULL;
    mRunningCapture = false;
    mReconfigurationRequested = false;
    mWarnUncalibrated = true;
    mRequestedSampleRate = -1;
    mLastUsedSampleRate = -2;

    for (int i = 0; i < MaxDigitalSignals; i++) {
        mDigitalSignals[i] = NULL;
        mDigitalSignalTransitions[i] = NULL;
    }

    for (int i = 0; i < MaxAnalogSignals; i++) {
        mAnalogSignals[i] = NULL;
        mAnalogSignalData[i] = NULL;
    }
}

/*!
    Frees up resources
*/
LabToolCaptureDevice::~LabToolCaptureDevice()
{
    free(mData);
    mData = NULL;

    if (mReconfigTimer != NULL) {
        mReconfigTimer->stop();
        delete mReconfigTimer;
    }

    for (int i = 0; i < MaxAnalogSignals; i++) {
        if (mAnalogSignals[i] != NULL) {
            delete mAnalogSignals[i];
        }
        if (mAnalogSignalData[i] != NULL) {
            delete mAnalogSignalData[i];
        }
    }

    for (int i = 0; i < MaxDigitalSignals; i++) {
        if (mDigitalSignals[i] != NULL) {
            delete mDigitalSignals[i];
        }
        if (mDigitalSignalTransitions[i] != NULL) {
            delete mDigitalSignalTransitions[i];
        }
    }

    delete mTriggerConfig;
}

QList<int> LabToolCaptureDevice::supportedSampleRates()
{
    // supported sample rates in Hz when SystemCoreClock is 200MHz
    return QList<int>()
            << 100000000
            <<  90000000
            <<  80000000
            <<  70000000
            <<  60000000
            <<  50000000
            <<  40000000
            <<  30000000
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
//            <<     10000
//            <<      5000
//            <<      2000
//            <<      1000
//            <<       500
//            <<       200
//            <<       100
//            <<        50
                 ;
}

int LabToolCaptureDevice::maxNumDigitalSignals()
{
    return MaxDigitalSignals;
}

int LabToolCaptureDevice::maxNumAnalogSignals()
{
    return MaxAnalogSignals;
}

QList<double> LabToolCaptureDevice::supportedVPerDiv()
{
    if (mSupportedVPerDiv.size() == 0) {
        mSupportedVPerDiv << 0.02
                          << 0.05
                          << 0.1
                          << 0.2
                          << 0.5
                          << 1
                          << 2
                          << 5;
    }

    return mSupportedVPerDiv;
}

/*!
    Opens the \ref UiLabToolTriggerConfig dialog to allow configuration
    of the additional trigger settings supported by the LabTool Hardware.
*/
void LabToolCaptureDevice::configureTrigger(QWidget* parent)
{
    (void)parent; // To avoid warning about unused parameter

    int result = mTriggerConfig->exec();

    if (result == QDialog::Accepted) {
        mConfigMustBeUpdated = true;
    }
}

/*!
    Opens the \ref LabToolCalibrationWizard to guid the user through the
    steps of calibrating the hardware. If the dialog is successfully completed
    then the calibration data is reloaded from the hardware.
*/
void LabToolCaptureDevice::calibrate(QWidget *parent)
{
    if (mDeviceComm == NULL) {
        QMessageBox msgBox(parent);
        msgBox.setText("Lost connection to Hardware.");
        msgBox.setInformativeText("There is no connected hardware to calibrate.");
        msgBox.exec();
    } else {
        LabToolCalibrationWizard wiz(parent);
        wiz.setComm(mDeviceComm);

        QObject::connect(mDeviceComm, SIGNAL(connectionStatus(bool)),
                &wiz, SLOT(handleConnectedStatus(bool)));

        int result = wiz.exec();

        if (result == QDialog::Accepted) {
            // User completed the calibration wizard and new values
            // were stored in hardware. Force a reload of those values.
            mConfigMustBeUpdated = true;
            mDeviceComm->storedCalibrationData(true);
        }

        mDeviceComm->disconnect(&wiz);
    }
}

/*!
    Scans the list of digital samples and locates the first entry with the correct
    level and returns it's index. The parameter \a s is the list of digital
    samples, parameter \a level is either one or zero. The \a offset parameter
    specifies where in the list to start looking.
*/
int LabToolCaptureDevice::locateFirstLevel(QVector<int> *s, int level, int offset)
{
    int start = offset;
    if (offset < 0) {
        start = 0;
    }
    for (int i = start; i < s->size(); i++) {
        if (s->at(i) == level) {
            return i;
        }
    }
    return -1;
}

/*!
    Scans the list of digital samples backwards and locates the first entry with the
    correct level and returns it's index. The parameter \a s is the list of digital
    samples, parameter \a level is either one or zero. The \a offset parameter
    specifies where in the list to start looking.
*/
int LabToolCaptureDevice::locatePreviousLevel(QVector<int> *s, int level, int offset)
{
    int start = offset;
    if (offset > s->size()) {
        start = s->size()-1;
    }
    for (int i = start; i >= 0; i--) {
        if (s->at(i) == level) {
            return i;
        }
    }
    return -1;
}


/*!
    Scans the list of calibrated (double format) analog samples specified
    by the \a s parameter starting at \a offset, looking
    for the position where the value goes from above \a highLevel to below
    \a lowLevel.

    In cases where the sample rate is much higher than the frequency of
    the sampled signal a lot of samples will have about the same value
    and the returned index is calculated as the middle point between the
    last value above \a highLevel and the first value below \a lowLevel.
*/
int LabToolCaptureDevice::locateAnalogHighLowTransition(QVector<double> *s, double lowLevel, double highLevel, int offset)
{
    int numSamples = s->size();

    if (highLevel != lowLevel) {

//        qDebug("dlocateAnalogHighLowTransition(trigLevel %f, offset %d", trigLevel, offset);
//        qDebug("lowLevel %f, highLevel %f, looking for High->Low", lowLevel, highLevel);

        for (int i = (offset < 0 ? 0 : offset); i < numSamples; i++) {
            if (s->at(i) > highLevel) {
LocateAnalogHighLowTransition_restart_comp:
//                qDebug("s[%d]: %f above %f, restart", i, s->at(i), highLevel);
                int lastAbove = i;
                while ((lastAbove < numSamples) && (s->at(lastAbove) > highLevel)) {
//                    qDebug("s[%d]: %f still above %f", lastAbove, s->at(lastAbove), highLevel);
                    lastAbove++;
                }
                if (lastAbove < numSamples) {
//                    qDebug("s[%d]: %f broke while with > %f", lastAbove, s->at(lastAbove), highLevel);
                } else {
//                    qDebug("s[%d]: broke while as \"out of bounds\"", lastAbove);
                    break;
                }

                // found first value above level, now find equal to or below level
                for (i = lastAbove; i < numSamples; i++) {
                    if (s->at(i) <= lowLevel) {
//                        qDebug("s[%d]: %f <= %f, done returning %d", i, s->at(i), lowLevel, (i+lastAbove)/2);
                        // found transition
                        return (i+lastAbove)/2;
                    }
                    if (s->at(i) > highLevel) {
                        goto LocateAnalogHighLowTransition_restart_comp;
                    }
//                    qDebug("s[%d]: %f between %f and %f", i, s->at(i), lowLevel, highLevel);
                }
                break;
            }
            else {
//                qDebug("s[%d]: %f  not above %f", i, s->at(i), highLevel);
            }
        }
    } else {
        for (int i = (offset < 0 ? 0 : offset); i < numSamples; i++) {
            if (s->at(i) > highLevel) {
                // found first value above level, now find equal to or below level
                for (i = i+1; i < numSamples; i++) {
                    if (s->at(i) <= lowLevel) {
                        // found transition
                        return i;
                    }
                }
                break;
            }
        }
    }

    return -1;
}

/*!
    Scans the list of calibrated (double format) analog samples specified
    by the \a s parameter backwards, starting at \a offset, looking
    for the position where the value goes from above \a highLevel to below
    \a lowLevel.

    As the list is searched backwards the search starts with the low level
    and then the high level - the opposite of the forward search.

    In cases where the sample rate is much higher than the frequency of
    the sampled signal a lot of samples will have about the same value
    and the returned index is calculated as the middle point between the
    last value above \a highLevel and the first value below \a lowLevel.
*/
int LabToolCaptureDevice::locatePreviousAnalogHighLowTransition(QVector<double> *s, double lowLevel, double highLevel, int offset)
{
    int numSamples = s->size();

    if (highLevel != lowLevel) {

//        qDebug("dlocatePreviousAnalogHighLowTransition(trigLevel %f, offset %d", trigLevel, offset);
//        qDebug("lowLevel %f, highLevel %f, looking for High->Low", lowLevel, highLevel);

        for (int i = (offset >= numSamples ? numSamples-1 : offset); i >= 0; i--) {
            if (s->at(i) < lowLevel) {
LocatePreviousAnalogHighLowTransition_restart_comp:
//                qDebug("s[%d]: %f below %f, restart", i, s->at(i), lowLevel);
                int lastBelow = i;
                while ((lastBelow >= 0) && (s->at(lastBelow) < lowLevel)) {
//                    qDebug("s[%d]: %f still below %f", lastBelow, s->at(lastBelow), lowLevel);
                    lastBelow--;
                }

                if (lastBelow >= 0) {
//                    qDebug("s[%d]: %f broke while with < %f", lastBelow, s->at(lastBelow), lowLevel);
                } else {
//                    qDebug("s[%d]: broke while as \"out of bounds\"", lastBelow);
                    break;
                }

                // found first value above low level, now find equal to or above high level
                for (i = lastBelow; i >= 0; i--) {
                    if (s->at(i) >= highLevel) {
//                        qDebug("s[%d]: %f >= %f, done returning %d", i, s->at(i), highLevel, (i+lastBelow)/2);
                        // found transition
                        return (i+lastBelow)/2;
                    }
                    if (s->at(i) < lowLevel) {
                        goto LocatePreviousAnalogHighLowTransition_restart_comp;
                    }
//                    qDebug("s[%d]: %f between %f and %f", i, s->at(i), lowLevel, highLevel);
                }
                break;
            }
            else {
//                qDebug("s[%d]: %f  not below %f", i, s->at(i), lowLevel);
            }
        }
    } else {
        for (int i = (offset >= numSamples ? numSamples-1 : offset); i >= 0; i--) {
            if (s->at(i) < lowLevel) {
                // found first value below level, now find equal to or above level
                for (i = i-1; i >= 0; i--) {
                    if (s->at(i) >= highLevel) {
                        // found transition
                        return i+1;
                    }
                }
                break;
            }
        }
    }

    return -1;
}

/*!
    Scans the list of calibrated (double format) analog samples specified
    by the \a s parameter starting at \a offset, looking
    for the position where the value goes from below \a lowLevel to above
    \a highLevel.

    In cases where the sample rate is much higher than the frequency of
    the sampled signal a lot of samples will have about the same value
    and the returned index is calculated as the middle point between the
    last value below \a lowLevel and the first value above \a highLevel.
*/
int LabToolCaptureDevice::locateAnalogLowHighTransition(QVector<double> *s, double lowLevel, double highLevel, int offset)
{
    int numSamples = s->size();

    if (highLevel != lowLevel) {

//        qDebug("dlocateAnalogLowHighTransition(trigLevel %f, offset %d", trigLevel, offset);
//        qDebug("lowLevel %f, highLevel %f, looking for High->Low", lowLevel, highLevel);

        for (int i = (offset < 0 ? 0 : offset); i < numSamples; i++) {
            if (s->at(i) < lowLevel) {
LocateAnalogLowHighTransition_restart_comp:
//                qDebug("s[%d]: %f below %f, restart", i, s->at(i), lowLevel);
                int lastBelow = i;
                while ((lastBelow < numSamples) && (s->at(lastBelow) < lowLevel)) {
//                    qDebug("s[%d]: %f still below %f", lastBelow, s->at(lastBelow), lowLevel);
                    lastBelow++;
                }
                if (lastBelow < numSamples) {
//                    qDebug("s[%d]: %f broke while with < %f", lastBelow, s->at(lastBelow), lowLevel);
                } else {
//                    qDebug("s[%d]: broke while as \"out of bounds\"", lastBelow);
                    break;
                }

                // found first value below level, now find equal to or above level
                for (i = lastBelow; i < numSamples; i++) {
                    if (s->at(i) >= highLevel) {
//                        qDebug("s[%d]: %f <= %f, done returning %d", i, s->at(i), highLevel, (i+lastBelow)/2);
                        // found transition
                        return (i+lastBelow)/2;
                    }
                    if (s->at(i) < lowLevel) {
                        goto LocateAnalogLowHighTransition_restart_comp;
                    }
//                    qDebug("s[%d]: %f between %f and %f", i, s->at(i), lowLevel, highLevel);
                }
                break;
            }
            else {
//                qDebug("s[%d]: %f  not below %f", i, s->at(i), lowLevel);
            }
        }
    } else {
        for (int i = (offset < 0 ? 0 : offset); i < numSamples; i++) {
            if (s->at(i) < lowLevel) {
                // found first value below level, now find equal to or above level
                for (i = i+1; i < numSamples; i++) {
                    if (s->at(i) >= highLevel) {
                        // found transition
                        return i;
                    }
                }
                break;
            }
        }
    }

    return -1;
}

/*!
    Scans the list of calibrated (double format) analog samples specified
    by the \a s parameter backwards, starting at \a offset, looking
    for the position where the value goes from below \a lowLevel to above
    \a highLevel.

    In cases where the sample rate is much higher than the frequency of
    the sampled signal a lot of samples will have about the same value
    and the returned index is calculated as the middle point between the
    last value below \a lowLevel and the first value above \a highLevel.
*/
int LabToolCaptureDevice::locatePreviousAnalogLowHighTransition(QVector<double> *s, double lowLevel, double highLevel, int offset)
{
    int numSamples = s->size();

    if (highLevel != lowLevel) {

//        qDebug("dlocatePreviousAnalogLowHighTransition(trigLevel %f, offset %d", trigLevel, offset);
//        qDebug("lowLevel %f, highLevel %f, looking for High->Low", lowLevel, highLevel);

        for (int i = (offset >= numSamples ? numSamples-1 : offset); i >= 0; i--) {
            if (s->at(i) > highLevel) {
LocatePreviousAnalogLowHighTransition_restart_comp:
//                qDebug("s[%d]: %f above %f, restart", i, s->at(i), highLevel);
                int lastAbove = i;
                while ((lastAbove >= 0) && (s->at(lastAbove) > highLevel)) {
//                    qDebug("s[%d]: %f still above %f", lastAbove, s->at(lastAbove), highLevel);
                    lastAbove--;
                }

                if (lastAbove >= 0) {
//                    qDebug("s[%d]: %f broke while with > %f", lastAbove, s->at(lastAbove), highLevel);
                } else {
//                    qDebug("s[%d]: broke while as \"out of bounds\"", lastAbove);
                    break;
                }

                // found first value below high level, now find equal to or below low level
                for (i = lastAbove; i >= 0; i--) {
                    if (s->at(i) <= lowLevel) {
//                        qDebug("s[%d]: %f <= %f, done returning %d", i, s->at(i), lowLevel, (i+lastAbove)/2);
                        // found transition
                        return (i+lastAbove)/2;
                    }
                    if (s->at(i) > highLevel) {
                        goto LocatePreviousAnalogLowHighTransition_restart_comp;
                    }
//                    qDebug("s[%d]: %f between %f and %f", i, s->at(i), lowLevel, highLevel);
                }
                break;
            }
            else {
//                qDebug("s[%d]: %f  not above %f", i, s->at(i), highLevel);
            }
        }
    } else {
        for (int i = (offset >= numSamples ? numSamples-1 : offset); i >= 0; i--) {
            if (s->at(i) > highLevel) {
                // found first value above level, now find equal to or below level
                for (i = i-1; i >= 0; i--) {
                    if (s->at(i) <= lowLevel) {
                        // found transition
                        return i+1;
                    }
                }
                break;
            }
        }
    }

    return -1;
}

/*!
    Converts the signal data received for digital signals from the LabTool Hardware
    into the format used by this application.

    Input format:

    \dot
     digraph structs {
         node [shape=record];
         start [label="DIO0 | DIO1 | ... | DIOn | DIO0 | ..."];
     }
     \enddot

    Each box is a 32-bit value containing 32 digital samples for that channel.
    the \a n value is the highest enabled channel number. If only DIO4 is
    enabled then the positions for DIO0, DIO1, DIO2 and DIO3 will still be
    present but with invalid data.

    The \a pData parameter is a pointer to the data, \a size is the number of
    bytes of data.

    The \a activeChannels parameter has two parts: The 16 MSB holds
    the number of channels with values in the data, the 16 LSB holds a bitmask
    where each channel with valid data has a bit set. In the previous example
    with only DIO4 enabled \a activeChannels would have the value \a 0x00050020.

    The \a trig parameter holds the id of the channel that caused the trigger.

    The \a digitalTrigSample and \a analogTrigSample parameters holds the current
    sample index at the time of triggering. They exist regardless of what caused
    the trigger (analog or digital) and are used to synchronize the signals in
    time. Example: \a digitalTrigSample is 500 and \a analogTrigSample is 600.
    The set of digital signals will be truncated and the last 100 samples removed.
    The set of analog signals will have the first 100 samples removed. The result
    is that both sets will be better aligned.
*/
void LabToolCaptureDevice::convertDigitalInput(const quint8 *pData, quint32 size, quint32 activeChannels, quint32 trig, int digitalTrigSample, int analogTrigSample)
{
    quint32* samples = (quint32*)pData;
    int signalsInInput = activeChannels >> 16;

    int samplePointDiff = analogTrigSample - digitalTrigSample;
    if (analogTrigSample == 0) {
        // no analog signals to adjust to. data only contains digital signals
        samplePointDiff = 0;
    }
    if (samplePointDiff > 0) {
        // need to remove samples from the start of the analog data
        // and remove samples from the end of the digital data
        // to align the two
    } else if (samplePointDiff < 0){
        // need to remove samples from the start of the digital data
        // and remove samples from the end of the analog data
        // to align the two
        digitalTrigSample += samplePointDiff; // move trigger point
        //qDebug("D: Diff %d, reduced digitalTrigSample to %d", samplePointDiff, digitalTrigSample);
    }

    foreach(DigitalSignal* signal, mDigitalSignalList) {
        int id = signal->id();

        if (id >= MaxDigitalSignals) continue;
        int slice = id;//GetSliceForId(id, activeChannels);
        if ((activeChannels & (1<<slice)) == 0) continue; // got no data for this channel from target

        int sampleGroups = (size/(signalsInInput*4));

        // Deallocation:
        //   QVector will be deallocated either by this function or the destructor
        //   as a part of deallocating mDigitalSignals
        QVector<int> *s = new QVector<int>();

        for(int j = 0; j < sampleGroups; ++j) {
            quint32 val = samples[j*signalsInInput + slice];
            for (int k = 0; k < 32; k++) {
                s->append(val & 1);
                val = val >> 1;
            }
        }

        if (samplePointDiff > 0) {
            // need to remove samples from the start of the analog data
            // and remove samples from the end of the digital data
            // to align the two
            if (s->size() >= samplePointDiff) {
                s->remove(s->size()-samplePointDiff-1, samplePointDiff);
                //qDebug("D: Diff %d, removed %d samples from the end of the digital data", samplePointDiff, samplePointDiff);
            } else {
                //qDebug("D: Diff %d, removed all %d samples from the digital data", samplePointDiff, s->size());
                s->clear();
            }
        } else if (samplePointDiff < 0){
            // need to remove samples from the start of the digital data
            // and remove samples from the end of the analog data
            // to align the two
            if (s->size() >= -samplePointDiff) {
                s->remove(0, -samplePointDiff);
                //qDebug("D: Diff %d, removed %d samples from the start of the digital data", samplePointDiff, -samplePointDiff);
            } else {
                //qDebug("D: Diff %d, removed all %d samples from the digital data", samplePointDiff, s->size());
                s->clear();
            }
        }


        if (((int)trig) == id)
        {
            // this signal was the trigger
            DigitalSignal::DigitalTriggerState trigger = signal->triggerState();

            int pos = 0;
            switch (trigger) {
            // Falling edge
            case DigitalSignal::DigitalTriggerHighLow:
                pos = locateFirstLevel(s, 1, digitalTrigSample-20);
                if (pos != -1) {
                    pos = locateFirstLevel(s, 0, pos);
                    if (pos != -1) {
                        // found first possible trigger past the digitalTrigSample location
                        mTriggerIndex = pos;
                        //qDebug("Found High->Low at %d, (+%d from %d)", pos, pos - digitalTrigSample, digitalTrigSample);
                    }
                }
                pos = locatePreviousLevel(s, 0, digitalTrigSample+20);
                if (pos != -1) {
                    pos = locatePreviousLevel(s, 1, pos);
                    if (pos != -1) {
                        pos++;
                        //qDebug("Found High->Low at %d, (%d from %d)", pos, pos - digitalTrigSample, digitalTrigSample);

                        // found last trigger before the digitalTrigSample location
                        if (abs(pos-digitalTrigSample) < abs(mTriggerIndex-digitalTrigSample)) {
                            // this trigger is the closest one to the digitalTrigSample location
                            mTriggerIndex = pos;
                        }
                    }
                }
                break;

                // Rising edge
            case DigitalSignal::DigitalTriggerLowHigh:
                pos = locateFirstLevel(s, 0, digitalTrigSample-20);
                if (pos != -1) {
                    pos = locateFirstLevel(s, 1, pos);
                    if (pos != -1) {
                        // found first possible trigger past the digitalTrigSample location
                        mTriggerIndex = pos;
                        //qDebug("Found Low->High at %d, (+%d from %d)", pos, pos - digitalTrigSample, digitalTrigSample);
                    }
                }
                pos = locatePreviousLevel(s, 1, digitalTrigSample+20);
                if (pos != -1) {
                    pos = locatePreviousLevel(s, 0, pos);
                    if (pos != -1) {
                        pos++;
                        //qDebug("Found Low->High at %d, (%d from %d)", pos, pos - digitalTrigSample, digitalTrigSample);

                        // found last trigger before the digitalTrigSample location
                        if (abs(pos-digitalTrigSample) < abs(mTriggerIndex-digitalTrigSample)) {
                            // this trigger is the closest one to the digitalTrigSample location
                            mTriggerIndex = pos;
                        }
                    }
                }
                break;

                // High level
#if 0 // disabling high level and low level as trigger levels
            case DigitalSignal::DigitalTriggerHigh:
                pos = locateFirstLevel(s, 1, digitalTrigSample-20);
                if (pos != -1) {
                    // found first possible trigger past the digitalTrigSample location
                    mTriggerIndex = pos;
                }
                pos = locatePreviousLevel(s, 1, digitalTrigSample+20);
                if (pos != -1) {
                    // found last trigger before the digitalTrigSample location
                    if (abs(pos-digitalTrigSample) < abs(mTriggerIndex-digitalTrigSample)) {
                        // this trigger is the closest one to the digitalTrigSample location
                        mTriggerIndex = pos;
                    }
                }
                break;

                // Low level
            case DigitalSignal::DigitalTriggerLow:
                pos = locateFirstLevel(s, 0, digitalTrigSample-20);
                if (pos != -1) {
                    // found first possible trigger past the digitalTrigSample location
                    mTriggerIndex = pos;
                }
                pos = locatePreviousLevel(s, 0, digitalTrigSample+20);
                if (pos != -1) {
                    // found last trigger before the digitalTrigSample location
                    if (abs(pos-digitalTrigSample) < abs(mTriggerIndex-digitalTrigSample)) {
                        // this trigger is the closest one to the digitalTrigSample location
                        mTriggerIndex = pos;
                    }
                }
                break;
#endif
                // Not a trigger
            default:
                break;
            }

        }


        if (mDigitalSignals[id] != NULL) {
            delete mDigitalSignals[id];
        }

        mDigitalSignals[id] = s;
        mEndSampleIdx = s->size()-1;
        //qDebug("D%d: %d samples", id, s->size());
    }
}

/*!
    Converts the signal data received for analog signals from the LabTool Hardware
    into two lists of integer values, one for each channel.

    Input format:

    \dot
     digraph structs {
         node [shape=record];
         start [label="A0 | A1 | A0 | ..."];
     }
     \enddot

    Each box is a 16-bit value containing one analog samples for that channel.
    If only one channel is enabled then only that channel's data will be present.
    Each 16-bit value is also marked with information about which channel
    the data is for.

    The \a pData parameter is a pointer to the data, \a size is the number of
    bytes of data.

    The \a activeChannels parameter has two parts: The 16 MSB holds
    the number of channels with values in the data, the 16 LSB holds a bitmask
    where each channel with valid data has a bit set.

    \todo Remove the \a activeChannels part of the protocol for analog signals?

    At high sample rates the analog signal data can get corrupted. This is only
    visible in the data when both analog channels are enabled and it will look
    like this:

    \dot
     digraph structs {
         node [shape=record];
         start [label="A0 | A1 | A0 | A0 | A1 | ..."];
     }
     \enddot

     This function detects the double values and inserts a value for the
     missing channel. In the example above channel A1 would get an extra value
     inserted. The reason for inserting extra value(s) is to at least keep the
     signals identical in length.

     One problem is that the double A0 could hide either one missing A1 value
     or one A1 and any number of A0+A1 samples. It is impossible to know.
*/
void LabToolCaptureDevice::unpackAnalogInput(const quint8 *pData, quint32 size, quint32 activeChannels)
{
    quint16* samples = (quint16*)pData;//PACKED

    for (int i = 0; i < MaxAnalogSignals; i++) {
        if (mAnalogSignalData[i] != NULL) {
            delete mAnalogSignalData[i];
        }
        mAnalogSignalData[i] = NULL;
    }

#define A0_CH_ID  (0)  // Mapping of A0 to the VADC channel number in fw
#define A1_CH_ID  (1)  // Mapping of A1 to the VADC channel number in fw

    // This is adjusted unpacking of the data. If an EMPTY marker is found, it is not treated
    // as data and is instead discarded. If two channels are sampled and two consecutive
    // samples for the same channel is found then an additional sample is inserted in the other
    // channel to make up for the missing one. The id bits of each sample is also read to make
    // sure that the samples end up on the correct signal's vector (prevents signal swapping).

    // Deallocation:
    //   QVector will be deallocated either by this function or by deleteSignals,
    //   unpackAnalogInput or the destructor as a part of deallocating mAnalogSignalData
    QVector<quint16> *s0 = new QVector<quint16>();
    // Deallocation:
    //   QVector will be deallocated either by this function or by deleteSignals,
    //   unpackAnalogInput or the destructor as a part of deallocating mAnalogSignalData
    QVector<quint16> *s1 = new QVector<quint16>();
    int numChannels = mAnalogSignalList.size();
    int lastId = -1;
    int numSamples = size/2;
    (void)activeChannels; // To avoid warning

    // Sometimes the absolute first sample is invalid (channel id 0). If that is the
    // case then discard that sample
//    if (((*samples & 0x7000)>>12) == 0) {
//        numSamples--;
//        samples++;
//    }

    for (int i = 0; i < numSamples; i++)
    {
        int id = (*samples & 0x7000)>>12;
        int empty = (*samples & 0x8000)>>15;
        if (empty)
        {
            qDebug("Empty marker for i=%d", i);
        }
        else
        {
            if (id == lastId && numChannels>1)
            {
                // found a skip i.e. two consecutive samples for the same channel, add an extra sample for the other channel
                if (id == A1_CH_ID)
                {
                    s0->append(s0->isEmpty() ? 0 : s0->last());
                }
                else
                {
                    s1->append(s1->isEmpty() ? 0 : s1->last());
                }
                qDebug("Skip at i=%d", i);
            }

            if (id == A1_CH_ID)
            {
                quint16 val = *samples;//PACKED
                val = val & 0xfff;
                s1->append(val);
            }
            else
            {
                quint16 val = *samples;//PACKED
                val = val & 0xfff;
                s0->append(val);
            }
            lastId = id;
        }
        samples++;
    }

    // Make sure that the same amount of samples have been received for both channels.
    // This difference can only happen when two channels have been sampled and the
    // difference in size can only be one element.
    if (numChannels > 1)
    {
        if (s0->size() > s1->size())
        {
            s0->pop_back();
        }
        else if (s1->size() > s0->size())
        {
            s1->pop_back();
        }
    }

    // Compensate for crosstalk between channels. The compensation is needed at sample
    // rates >=30MHz and only when sampling both channels.
    if (numChannels > 1 && (mUsedSampleRate == 30000000 || mUsedSampleRate == 40000000))
    {
        // Deallocation:
        //   QVector will be deallocated by deleteSignals, unpackAnalogInput or
        //   the destructor as a part of deallocating mAnalogSignalData
        QVector<quint16> *s0prim = new QVector<quint16>();
        // Deallocation:
        //   QVector will be deallocated by deleteSignals, unpackAnalogInput or
        //   the destructor as a part of deallocating mAnalogSignalData
        QVector<quint16> *s1prim = new QVector<quint16>();

        // 40MHz needs 8%, 30MHz needs 5%
        int percent = 8;
        if (mUsedSampleRate == 30000000)
        {
            percent = 5;
        }

        s0prim->append(s0->at(0));
        s1prim->append(s1->at(0) - (percent*(s0->at(0) - 2048))/100);
        for (int i = 1; i < s1->size(); i++)
        {
            s0prim->append(s0->at(i) - (percent*(s1->at(i-1) - 2048))/100);
            s1prim->append(s1->at(i) - (percent*(s0->at(i) - 2048))/100);
        }

        delete s0;
        delete s1;
        s0 = s0prim;
        s1 = s1prim;
    }

    mAnalogSignalData[0] = s0;
    mAnalogSignalData[1] = s1;
}

/*!
    Converts the signal data received for analog signals from the LabTool Hardware
    into the format used by this application.

    The conversion is done in three steps:
    -# Use \ref unpackAnalogInput to creates one list of integer values per channel.
    -# Convert the integer values in to double values used by this application
    -# Scale the values according to each channel's Volts/div setting

    The \a pData parameter is a pointer to the data, \a size is the number of
    bytes of data.

    The \a activeChannels parameter has two parts: The 16 MSB holds
    the number of channels with values in the data, the 16 LSB holds a bitmask
    where each channel with valid data has a bit set. In the previous example
    with only DIO4 enabled \a activeChannels would have the value \a 0x00050020.

    The \a trig parameter holds the id of the channel that caused the trigger.

    The \a digitalTrigSample and \a analogTrigSample parameters holds the current
    sample index at the time of triggering. They exist regardless of what caused
    the trigger (analog or digital) and are used to synchronize the signals in
    time. Example: \a digitalTrigSample is 500 and \a analogTrigSample is 600.
    The set of digital signals will be truncated and the last 100 samples removed.
    The set of analog signals will have the first 100 samples removed. The result
    is that both sets will be better aligned.
*/
void LabToolCaptureDevice::convertAnalogInput(const quint8 *pData, quint32 size, quint32 activeChannels, quint32 trig, int analogTrigSample, int digitalTrigSample)
{
    (void)trig; // To avoid warning
    if (mAnalogSignalList.isEmpty()) {
        // nothing to do
        return;
    }
    unpackAnalogInput(pData, size, activeChannels);

    int samplePointDiff = analogTrigSample - digitalTrigSample;
    if (digitalTrigSample == 0) {
        // no digital signals to adjust to. data only contains analog signals
        samplePointDiff = 0;
    }
    if (samplePointDiff > 0) {
        // need to remove samples from the start of the analog data
        // and remove samples from the end of the digital data
        // to align the two
        analogTrigSample -= samplePointDiff; // move trigger point
        //qDebug("A: Diff %d, reduced analogTrigSample to %d", samplePointDiff, analogTrigSample);
    } else if (samplePointDiff < 0){
        // need to remove samples from the start of the digital data
        // and remove samples from the end of the analog data
        // to align the two
    }

    LabToolCalibrationData* calib = mDeviceComm->storedCalibrationData();

    foreach(AnalogSignal* signal, mAnalogSignalList) {
        int id = signal->id();
        int voltsPerDivIndex = supportedVPerDiv().indexOf(signal->vPerDiv());
        double a = calib->analogFactorA(id, voltsPerDivIndex);
        double b = calib->analogFactorB(id, voltsPerDivIndex);

        if (mAnalogSignalData[id] == NULL) continue;

        if (samplePointDiff > 0) {
            // need to remove samples from the start of the analog data
            // and remove samples from the end of the digital data
            // to align the two
            analogTrigSample -= samplePointDiff; // move trigger point
            if (mAnalogSignalData[id]->size() >= samplePointDiff) {
                mAnalogSignalData[id]->remove(0, samplePointDiff);
                //qDebug("A: Diff %d, removed %d samples from the start of the analog data", samplePointDiff, samplePointDiff);
            } else {
                //qDebug("A: Diff %d, removed all %d samples from the analog data", samplePointDiff, mAnalogSignalData[id]->size());
                mAnalogSignalData[id]->clear();
            }
        } else if (samplePointDiff < 0){
            // need to remove samples from the start of the digital data
            // and remove samples from the end of the analog data
            // to align the two
            if (mAnalogSignalData[id]->size() >= -samplePointDiff) {
                mAnalogSignalData[id]->remove(mAnalogSignalData[id]->size()+samplePointDiff-1, -samplePointDiff);
                //qDebug("A: Diff %d, removed %d samples from the end of the analog data", samplePointDiff, -samplePointDiff);
            } else {
                //qDebug("A: Diff %d, removed all %d samples from the analog data", samplePointDiff, mAnalogSignalData[id]->size());
                mAnalogSignalData[id]->clear();
            }
        }

        // Deallocation:
        //   QVector will be deallocated either by this function or the destructor
        //   as a part of deallocating mAnalogSignals
        QVector<double> *s = new QVector<double>();

        for (int j = 0; j < mAnalogSignalData[id]->size(); j++)
        {
            double dval = a + b * mAnalogSignalData[id]->at(j);
            s->append(dval);
        }

        if (signal->triggerState() != AnalogSignal::AnalogTriggerNone)
        {
            double trigLevel = signal->triggerLevel();
            double lowLevel = trigLevel;
            double highLevel = trigLevel;
            bool forceNoiseFilter = true; // have to apply some filtering

            if (forceNoiseFilter) {
                lowLevel = trigLevel - b * (1<<5);
                highLevel = trigLevel + b * (1<<5);
            } else if (mTriggerConfig->isNoiseFilterEnabled()) {
                lowLevel = trigLevel - b * mTriggerConfig->noiseFilter12BitLevel();
                highLevel = trigLevel + b * mTriggerConfig->noiseFilter12BitLevel();
            }

            int pos;
            switch(signal->triggerState()) {
            // Falling edge
            case AnalogSignal::AnalogTriggerHighLow:
                pos = locateAnalogHighLowTransition(s, lowLevel, highLevel, analogTrigSample-20);
                if (pos != -1) {
                    // found first possible trigger past the analogTrigSample location
                    //qDebug("Found High->Low at %d, (+%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                    mTriggerIndex = pos;
                }
                pos = locatePreviousAnalogHighLowTransition(s, lowLevel, highLevel, analogTrigSample+20);
                if (pos != -1) {
                    // found last trigger before the analogTrigSample location
                    //qDebug("Found High->Low at %d, (%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                    if (abs(pos-analogTrigSample) < 2*abs(mTriggerIndex-analogTrigSample)) { //*2 as we prefer to find the one prior to the analogTrigSample
                        // this trigger is the closest one to the analogTrigSample location
                        mTriggerIndex = pos;
                    }
                }
                if (mTriggerIndex == 0) {
                    // Could not find any trigger point after filtering. Try with the unfiltered search.
                    pos = locateAnalogHighLowTransition(s, trigLevel, trigLevel, analogTrigSample-20);
                    if (pos != -1) {
                        // found first possible trigger past the analogTrigSample location
                        //qDebug("Found unfiltered High->Low at %d, (+%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                        mTriggerIndex = pos;
                    }
                    pos = locatePreviousAnalogHighLowTransition(s, trigLevel, trigLevel, analogTrigSample+20);
                    if (pos != -1) {
                        // found last trigger before the analogTrigSample location
                        //qDebug("Found unfiltered High->Low at %d, (%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                        if (abs(pos-analogTrigSample) < 2*abs(mTriggerIndex-analogTrigSample)) { //*2 as we prefer to find the one prior to the analogTrigSample
                            // this trigger is the closest one to the analogTrigSample location
                            mTriggerIndex = pos;
                        }
                    }
                }
                break;

                // Rising edge
            case AnalogSignal::AnalogTriggerLowHigh:
                pos = locateAnalogLowHighTransition(s, lowLevel, highLevel, analogTrigSample-20);
                if (pos != -1) {
                    // found first possible trigger past the analogTrigSample location
                    //qDebug("Found Low->High at %d, (+%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                    mTriggerIndex = pos;
                }
                pos = locatePreviousAnalogLowHighTransition(s, lowLevel, highLevel, analogTrigSample+20);
                if (pos != -1) {
                    // found last trigger before the analogTrigSample location
                    //qDebug("Found Low->High at %d, (%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                    if (abs(pos-analogTrigSample) < 2*abs(mTriggerIndex-analogTrigSample)) { //*2 as we prefer to find the one prior to the analogTrigSample
                        // this trigger is the closest one to the analogTrigSample location
                        mTriggerIndex = pos;
                    }
                }
                if (mTriggerIndex == 0) {
                    // Could not find any trigger point after filtering. Try with the unfiltered search.
                    pos = locateAnalogLowHighTransition(s, trigLevel, trigLevel, analogTrigSample-20);
                    if (pos != -1) {
                        // found first possible trigger past the analogTrigSample location
                        //qDebug("Found unfiltered Low->High at %d, (+%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                        mTriggerIndex = pos;
                    }
                    pos = locatePreviousAnalogLowHighTransition(s, trigLevel, trigLevel, analogTrigSample+20);
                    if (pos != -1) {
                        // found last trigger before the analogTrigSample location
                        //qDebug("Found unfiltered Low->High at %d, (%d from %d)", pos, pos - analogTrigSample, analogTrigSample);
                        if (abs(pos-analogTrigSample) < 2*abs(mTriggerIndex-analogTrigSample)) { //*2 as we prefer to find the one prior to the analogTrigSample
                            // this trigger is the closest one to the analogTrigSample location
                            mTriggerIndex = pos;
                        }
                    }
                }
                break;

                // Not a trigger
            default:
                break;
            }
        }

        if (mAnalogSignals[id] != NULL) {
            delete mAnalogSignals[id];
        }

        mAnalogSignals[id] = s;
        mEndSampleIdx = s->size()-1;
        //qDebug("A%d: %d samples", id, s->size());
    }
}

void LabToolCaptureDevice::start(int sampleRate)
{
    if (mWarnUncalibrated) {
        mWarnUncalibrated = false;

        LabToolCalibrationData* mCalib = mDeviceComm->storedCalibrationData();
        if (mCalib == NULL || mCalib->isDefaultData()) {
            captureFinished(false, "The connected LabTool Device hardware has not been calibrated "
                            "and is running with default parameters. "
                            "Run the Calibration Wizard to correct it.\n"
                            "This capture has been aborted!");
            return;
        }
    }
    if (mRunningCapture) {
        // preventing double starts
        return;
    }
//    mEndSampleIdx = -1;
    mRequestedSampleRate = sampleRate;
    mReconfigurationRequested = false;

    qDebug() << "LabToolCaptureDevice::start";

    mRunningCapture = true;
    if (hasConfigChanged()) {
        qDebug("Configuration has changed and will be pushed to target");
        mDeviceComm->configureCapture(configSize(), configData());
    } else {
        //qDebug("Configuration same as last time");
        mDeviceComm->runCapture();
    }
}

void LabToolCaptureDevice::stop()
{
    qDebug() << "LabToolCaptureDevice::stop";
    mReconfigurationRequested = false;
    mRunningCapture = false;
    mDeviceComm->stopCapture();
}

int LabToolCaptureDevice::lastSampleIndex()
{
    return mEndSampleIdx;
}

QVector<int>* LabToolCaptureDevice::digitalData(int signalId)
{
    QVector<int>* data = NULL;

    if (signalId < MaxDigitalSignals) {
        data = mDigitalSignals[signalId];
    }

    return data;
}

void LabToolCaptureDevice::setDigitalData(int signalId, QVector<int> data)
{
    if (signalId < MaxDigitalSignals) {

        if (mDigitalSignals[signalId] != NULL) {
            delete mDigitalSignals[signalId];
            mDigitalSignals[signalId] = NULL;
        }

        if (data.size() > 0) {
            mEndSampleIdx = data.size()-1;

            // Deallocation:
            //   QVector will be deallocated either by this function or the destructor
            //   as a part of deallocating mDigitalSignals
            mDigitalSignals[signalId] = new QVector<int>(data);
        }

    }
}

QVector<double>* LabToolCaptureDevice::analogData(int signalId)
{
    QVector<double>* data = NULL;

    if (signalId < MaxAnalogSignals) {
        data = mAnalogSignals[signalId];
    }

    return data;
}

void LabToolCaptureDevice::setAnalogData(int signalId, QVector<double> data)
{
    if (signalId < MaxAnalogSignals) {

        if (mAnalogSignals[signalId] != NULL) {
            delete mAnalogSignals[signalId];
            mAnalogSignals[signalId] = NULL;
        }

        if (data.size() > 0) {
            mEndSampleIdx = data.size()-1;

            // Deallocation:
            //   QVector will be deallocated either by this function or the destructor
            //   as a part of deallocating mAnalogSignalData
            mAnalogSignals[signalId] = new QVector<double>(data);
        }
    }
}

void LabToolCaptureDevice::clearSignalData()
{
    deleteSignals();
}

int LabToolCaptureDevice::digitalTriggerIndex()
{
    return mTriggerIndex;
}

void LabToolCaptureDevice::setDigitalTriggerIndex(int idx)
{
    mTriggerIndex = idx;
}

void LabToolCaptureDevice::digitalTransitions(int signalId, QList<int> &list)
{
    if (signalId >= MaxDigitalSignals) return;
    if (mDigitalSignals[signalId] == NULL) return;

    // Not in cache. Create the list
    if (mDigitalSignalTransitions[signalId] == NULL) {
        // Deallocation:
        //   QList will be deallocated by the destructor
        //   as a part of deallocating mDigitalSignalTransitions
        QList<int>* l = new QList<int>();
        CaptureDevice::digitalTransitions(signalId, *l);
        mDigitalSignalTransitions[signalId] = l;
    }

    list = *mDigitalSignalTransitions[signalId];
}

void LabToolCaptureDevice::reconfigure(int sampleRate)
{
    // Ignore if there is no ongoing capture as the reconfiguration
    // will take place the next time a capture is started
    if (!mRunningCapture) {
        return;
    }

    if (mReconfigTimer == NULL) {
        // Deallocation: Destructor is responsible
        mReconfigTimer = new QTimer();
        mReconfigTimer->setInterval(200);
        mReconfigTimer->setSingleShot(true);
        QObject::connect(mReconfigTimer, SIGNAL(timeout()), this, SLOT(handleReconfigurationTimer()));
    }

    if (sampleRate != -1) {
        mRequestedSampleRate = sampleRate;
    }

    // Start (or restart if already running) the timer to gather consecutive
    // changes (e.g. a slider will create events continuously as long as the
    // user moves it). This way we at least get only one event every 200ms
    mReconfigurationRequested = true;
    mReconfigTimer->start();
}

/*!
    Sets the communication interface to the LabTool Hardware. Should
    be called with \a comm as NULL when the connection to the LabTool Hardware
    is lost and then with a non-NULL \a comm when the connection has been
    restored.
*/
void LabToolCaptureDevice::setDeviceComm(LabToolDeviceComm* comm)
{
    if (comm == NULL)
    {
        // lost connection
        mConfigMustBeUpdated = true;
        mRunningCapture = false;
    }
    mDeviceComm = comm;
}

/*!
    Deletes all analog and digital signals and related data such as
    lists of transitions.
*/
void LabToolCaptureDevice::deleteSignals()
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

        if (mAnalogSignalData[i] != NULL) {
            delete mAnalogSignalData[i];
            mAnalogSignalData[i] = NULL;
        }
    }
}

/*!
    A report that the LabTool Hardware has stopped as requested.
    Sends the \ref captureFinished signal to indicate success.
*/
void LabToolCaptureDevice::handleStopped()
{
    qDebug("finished stopping");
    mRunningCapture = false;
    if (mReconfigurationRequested) {
        qDebug("Reconfiguration timer starting new capture");
        mReconfigurationRequested = false;
        start(mRequestedSampleRate);
    } else {
        emit captureFinished(true, "");
    }
}

/*!
    A report that the LabTool Hardware has completed the requested
    configuration update.
    The LabTool Hardware is only configured when the user has selected
    to start a capture (but had an old configuration) so now that the
    configuration is finished, the actual capturing will be started.
*/
void LabToolCaptureDevice::handleConfigurationDone()
{
    // now that the configuration has been applied, save current configuration
    saveConfig();

    // configuration only done immediately before running, so run now
    //qDebug("Configuration done, time to run");
    mDeviceComm->runCapture();
    mRunningCapture = true;
}

/*!
    A report that the LabTool Hardware has failed to complete the requested
    configuration update. A \ref captureFinished signal will be sent to
    indicate the failure with an explanation in \a msg.
*/
void LabToolCaptureDevice::handleConfigurationFailure(const char *msg)
{
    mRunningCapture = false;
    mConfigMustBeUpdated = true;
    emit captureFinished(false, msg);
}

/*!
    A report that the LabTool Hardware has successfully captured the requested
    signal data.
    The previously collected signals will be discarded and the new data will
    be unpacked. Finally a \ref captureFinished signal will be sent to
    indicate the successful end of the capturing.
*/
void LabToolCaptureDevice::handleReceivedSamples(LabToolDeviceTransfer* transfer, unsigned int size, unsigned int trigger, unsigned int digitalTrigSample, unsigned int analogTrigSample, unsigned int digitalChannelInfo, unsigned int analogChannelInfo)
{
    if (mReconfigurationRequested && hasConfigChanged()) {
        // will restart capture with the new data so discard this set
        qDebug("Discarding captured data as reconfiguration is in the pipe");
    } else {
        int analogOffset = transfer->analogDataOffset();
        int analogSize = transfer->analogDataSize();
        deleteSignals();

        mUsedSampleRate = mRequestedSampleRate;
        mTriggerIndex = 0;
        convertDigitalInput(transfer->data(), size-analogSize, digitalChannelInfo, trigger, digitalTrigSample, analogTrigSample);
        convertAnalogInput(transfer->data()+analogOffset, analogSize, analogChannelInfo, trigger, analogTrigSample, digitalTrigSample);
        qDebug() << "Got " << size << "bytes with samples";
        //qDebug() << "Digital trigger at " << digitalTrigSample << ", analog at " << analogTrigSample;

        mRunningCapture = false;
        emit captureFinished(true, "");
    }
    delete transfer;
}

/*!
    A report that the LabTool Hardware has failed to capture signal data
    as requested. A \ref captureFinished signal will be sent to
    indicate the failure with an explanation in \a msg.
*/
void LabToolCaptureDevice::handleFailedCapture(const char *msg)
{
    mRunningCapture = false;
    emit captureFinished(false, msg);
}

/*!
    Called by the reconfiguration timer. If a capture is still
    running and the configuration has changed then the capture
    will be stopped here and \ref handleStopped will start it
    again.
*/
void LabToolCaptureDevice::handleReconfigurationTimer()
{
    // Ignore if there is no ongoing capture as the reconfiguration
    // will take place the next time a capture is started
    if (!mRunningCapture || !mReconfigurationRequested) {
        return;
    }

    // If there still are changes to apply then stop the ongoing
    // capture (it will be started again from the handleStopped()
    // slot). Abort if at this time there is no changes in the configuration
    // anymore.
    if (hasConfigChanged()) {
        qDebug("Reconfiguration timer causes stop");
        mRunningCapture = false;
        mDeviceComm->stopCapture();
    } else {
        mReconfigurationRequested = false;
    }
}


/*!
    Converts the trigger level of the \a signal parameter which is in
    the -5..5 range into a integer value in the 0..4096 range suitable for
    comparisons with the analog sample data retrieved from the LabTool Hardware.
    The conversion is based on the calibration data from the hardware.
*/
qint16 LabToolCaptureDevice::analog12BitTriggerLevel(const AnalogSignal *signal)
{
    LabToolCalibrationData* calib = mDeviceComm->storedCalibrationData();

    int id = signal->id();
    int voltsPerDivIndex = supportedVPerDiv().indexOf(signal->vPerDiv());
    double a = calib->analogFactorA(id, voltsPerDivIndex);
    double b = calib->analogFactorB(id, voltsPerDivIndex);

    // Convert trigger level in volts to the 0..4096 range used by the hardware

    // Vout = A + B * hex  => hex = (Vout - A) / B
    return (signal->triggerLevel() - a) / b;
}

/*!
    Returns true if any change has been made to the configuration.
    Used to determine if the LabTool Hardware's configuration must
    be updated before a new capture can be started.
*/
bool LabToolCaptureDevice::hasConfigChanged()
{
    do {
        if (mConfigMustBeUpdated) break;

        if (mLastUsedSampleRate != mRequestedSampleRate) break;

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
void LabToolCaptureDevice::saveConfig()
{
    mConfigMustBeUpdated = false;
    mLastUsedSampleRate = mRequestedSampleRate;

    mLastUsedDigtalSignals.clear();
    foreach(DigitalSignal* signal, mDigitalSignalList) {
        mLastUsedDigtalSignals.append(*signal);
    }

    mLastUsedAnalogSignals.clear();
    foreach(AnalogSignal* signal, mAnalogSignalList) {
        mLastUsedAnalogSignals.append(*signal);
    }
}

/*!
    Number of bytes in the configuration data to send to the LabTool Hardware.
*/
unsigned int LabToolCaptureDevice::configSize()
{
    return sizeof(capture_cfg_t);
}

/*!
    Prepares and returns the configuration data to send to the LabTool Hardware.
    The function is only called when the configuration has changed so there
    is no need to cache the changes.

    The data is returned as a byte array, but is actually one \a capture_cfg_t
    structure. The signal independant information is filled in here and then
    \ref updateDigitalConfigData and \ref updateAnalogConfigData are called
    to fill in the signal specific parts.
*/
quint8 *LabToolCaptureDevice::configData()
{
    memset(mData, 0, sizeof(capture_cfg_t));

    // Configure the common parts
    capture_cfg_t* common_header = (capture_cfg_t*)mData;
    common_header->sampleRate = mRequestedSampleRate;

    common_header->postFill =  (mTriggerConfig->postFillPercent()&0xff);
    long tmp = ((mTriggerConfig->postFillTimeLimit() * mRequestedSampleRate)/1000);
    if (tmp > 0xffffff) {
        tmp = 0xffffff;
    }
    common_header->postFill |= (tmp&0xffffff)<<8;

    if (!mDigitalSignalList.isEmpty()) {
        updateDigitalConfigData();
    }
    if (!mAnalogSignalList.isEmpty()) {
        updateAnalogConfigData();
    }

    return mData;
}

/*!
    Fills in the configuration of the digital signals in the \a cap_sgpio_cfg_t
    part of the \a capture_cfg_t to send to the LabTool Hardware.
*/
void LabToolCaptureDevice::updateDigitalConfigData()
{
    int maxId = -1;
    capture_cfg_t* common_header = (capture_cfg_t*)mData;
    cap_sgpio_cfg_t* header = &common_header->sgpio;

    // Add signal information: which are enabled, which can trigger, type of trigger
    foreach(DigitalSignal* signal, mDigitalSignalList) {
        int id = signal->id();

        if (id > maxId) {
            maxId = id;
        }

        // Mark signal as enabled
        header->enabledChannels |= (1<<id);

        // Add any trigger information
        switch (signal->triggerState())
        {
        // Falling edge
        case DigitalSignal::DigitalTriggerHighLow:
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((1 & 0x3)<<(id*2));
            break;

        // Rising edge
        case DigitalSignal::DigitalTriggerLowHigh:
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((0 & 0x3)<<(id*2));
            break;

#if 0 // disabling high level and low level as trigger levels
        // High level
        case DigitalSignal::DigitalTriggerHigh:
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((3 & 0x3)<<(id*2));
            break;

        // Low level
        case DigitalSignal::DigitalTriggerLow:
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((2 & 0x3)<<(id*2));
            break;
#endif

        // Not a trigger
        default:
            break;
        }
    }

    // Specify how many digital signals are enabled
    common_header->numEnabledSGPIO = mDigitalSignalList.size();
}

/*!
    Fills in the configuration of the analog signals in the \a cap_vadc_cfg_t
    part of the \a capture_cfg_t to send to the LabTool Hardware.
*/
void LabToolCaptureDevice::updateAnalogConfigData()
{
    capture_cfg_t* common_header = (capture_cfg_t*)mData;
    cap_vadc_cfg_t* header = &common_header->vadc;

    // Add signal information: which are enabled, which can trigger, type of trigger
    foreach(AnalogSignal* signal, mAnalogSignalList) {
        int id = signal->id();

        // Mark signal as enabled
        header->enabledChannels |= (1<<id);

        // Convert trigger level from Volts to the 0..4095 range
        qint32 triggerLevel = analog12BitTriggerLevel(signal);

        // Add any trigger information
        switch (signal->triggerState())
        {
        // Falling edge
        case AnalogSignal::AnalogTriggerHighLow:
            //qDebug("Trigger ch %d high-low with level %f", id, getAnalogTriggerLevels()[id]);
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((1 & 0x3)<<(id*16 + 14));
            header->triggerSetup |= ((triggerLevel & 0xfff)<<(id*16));
            break;

        // Rising edge
        case AnalogSignal::AnalogTriggerLowHigh:
            //qDebug("Trigger ch %d low-high with level %f", id, getAnalogTriggerLevels()[id]);
            header->enabledTriggers |= (1<<id);
            header->triggerSetup |= ((0 & 0x3)<<(id*16 + 14));
            header->triggerSetup |= ((triggerLevel & 0xfff)<<(id*16));
            break;

        // Not a trigger
        default:
            break;
        }

        // Specify volt per div
        double vdiv = signal->vPerDiv();
        int idx = supportedVPerDiv().indexOf(vdiv);
        if (idx == -1) {
            qCritical("Volts per div %f is not one of the supported values", vdiv);
            header->voltPerDiv |= 0xf<<(id*4);//TODO: Report error as this case is invalid
        } else {
            header->voltPerDiv |= (idx & 0xf)<<(id*4);
        }

        // Specify couplings
        if (signal->coupling() == AnalogSignal::CouplingAc) {
            header->couplings |= (1<<id);
        }
    }

    // Specify if noise reduction should be enabled and how much
    if (mTriggerConfig->isNoiseFilterEnabled()) {
      header->noiseReduction = (1<<31) | ((1<<mTriggerConfig->noiseFilterLevel()) & 0xfff);
    }

    // Specify how many digital signals are enabled
    common_header->numEnabledVADC = mAnalogSignalList.size();
}
