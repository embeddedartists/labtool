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
#include "uieditanalog.h"

#include <QHBoxLayout>
#include <QFormLayout>

#include "common/stringutil.h"
#include "device/devicemanager.h"

/*!
    \class UiEditAnalog
    \brief UI widget that is responsible for analog signal generation settings.

    \ingroup Generator

*/


/*!
    Constructs an UiEditAnalog with the given analog signal \a signal
    and \a parent.
*/
UiEditAnalog::UiEditAnalog(AnalogSignal *signal, QWidget *parent) :
    QWidget(parent)
{
    mSignal = signal;

    // Deallocation: ownership changed when calling setLayout
    QHBoxLayout* layout = new QHBoxLayout();

    // Deallocation: Re-parented when calling addLayout below
    QFormLayout* settingsLayout = new QFormLayout();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mNameEdit = new QLineEdit(mSignal->name(), this);
    settingsLayout->addRow(tr("Name:"), mNameEdit);
    connect(mNameEdit, SIGNAL(editingFinished()),
            this, SLOT(handleNameEdited()));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mShape = new UiAnalogShape(this);
    mShape->setWaveform(mSignal->waveform());

    mWaveBox = createWaveformBox(mSignal->waveform());
    settingsLayout->addRow(tr("Waveform:"), mWaveBox);

    mRate = createFrequencyBox();
    mRate->setText(StringUtil::frequencyToString(mSignal->frequency()));
    settingsLayout->addRow(tr("Frequency:"), mRate);

    mAmpBox = createAmplitudeBox();
    mAmpBox->setValue(mSignal->amplitude());
    settingsLayout->addRow(tr("Amplitude:"), mAmpBox);

    layout->addLayout(settingsLayout);
    layout->addWidget(mShape);

    setLayout(layout);    
}


/*!
    \fn AnalogSignal* UiEditAnalog::signal()

    Returns the analog signal associated with this widget.
*/


/*!
    Marks the signal as invalid and disconnects the editor. This is needed
    to prevent calls to handleNameEdited() after the AnalogSignal has been
    deleted elsewhere.
*/
void UiEditAnalog::invalidateSignal()
{
    disconnect(mNameEdit, SIGNAL(editingFinished()),
               this, SLOT(handleNameEdited()));
    mSignal = NULL;
}

/*!
    Creates and returns a box where the analog waveform can be chosen.
    The current waveform is set to \a selected.
*/
QComboBox* UiEditAnalog::createWaveformBox(
        AnalogSignal::AnalogWaveform selected)
{
    // Deallocation: ownership changed when calling setLayout
    QComboBox* box = new QComboBox(this);

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (device != NULL) {

        foreach(AnalogSignal::AnalogWaveform form,
                device->supportedAnalogWaveforms())
        {
            switch(form) {
            case AnalogSignal::WaveformSine:
                box->addItem("Sine", QVariant(AnalogSignal::WaveformSine));
                break;
            case AnalogSignal::WaveformSquare:
                box->addItem("Square", QVariant(AnalogSignal::WaveformSquare));
                break;
            case AnalogSignal::WaveformTriangle:
                box->addItem("Triangle",
                             QVariant(AnalogSignal::WaveformTriangle));
                break;
            default:
                break;
            }

            if (form == selected) {
                box->setCurrentIndex(box->count()-1);
            }
        }

    }

    connect(box, SIGNAL(currentIndexChanged(int)),
            this, SLOT(changeWaveform(int)));

    return box;
}

/*!
    Creates and returns a box where the analog frequency can be set.
*/
QLineEdit* UiEditAnalog::createFrequencyBox()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();


    // Deallocation: ownership changed when calling setLayout
    QLineEdit* rate = new QLineEdit(this);
    rate->setToolTip(tr("Frequency"));
//    rate->setMaximumWidth(80);

    mLastRateText = StringUtil::frequencyToString(device->maxAnalogRate());
    rate->setText(mLastRateText);

    connect(rate, SIGNAL(editingFinished()), this, SLOT(updateRate()));


    return rate;
}

/*!
    Creates and returns a box where the analog amplitude can be chosen.
*/
QDoubleSpinBox* UiEditAnalog::createAmplitudeBox()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    // Deallocation: ownership changed when calling setLayout
    QDoubleSpinBox* box = new QDoubleSpinBox(this);
    box->setMinimum(0);
    box->setMaximum(device->maxAnalogAmplitude());
    box->setSingleStep(0.1);
    box->setSuffix(" V");

    connect(box, SIGNAL(valueChanged(double)),
            this, SLOT(amplitudeChanged(double)));

    return box;
}

/*!
    This function is called when the name of the signal is changed.
*/
void UiEditAnalog::handleNameEdited()
{
    QString n = mNameEdit->text();
    if (n.isEmpty() || n.isNull()) {
        n = mSignal->name();
        mNameEdit->setText(n);
    } else if (mNameEdit->text() != mSignal->name()) {
        mSignal->setName(n);
    }
}

/*!
    This function is called when the frequency is changed.
*/
void UiEditAnalog::updateRate()
{

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    QString t = mRate->text().trimmed();

    do {

        if (!StringUtil::isFrequencyStringValid(t)) {
            mRate->setText(mLastRateText);
            break;
        }

        int freq = StringUtil::frequencyToInt(t);

        if (freq < device->minAnalogRate()) {
            freq = device->minAnalogRate();
        }

        if (freq > device->maxAnalogRate()) {
            freq = device->maxAnalogRate();
        }

        t = StringUtil::frequencyToString(freq);

        mRate->setText(t);

        mLastRateText = t;

        mSignal->setFrequency(freq);

    } while (false);

}

/*!
    This function is called when the waveform is changed.
*/
void UiEditAnalog::changeWaveform(int selectedIdx)
{
    int w = mWaveBox->itemData(selectedIdx).toInt();
    if (w < 0 || w >= AnalogSignal::WaveformNum) return;

    mShape->setWaveform(static_cast<AnalogSignal::AnalogWaveform>(w));
    mSignal->setWaveform(static_cast<AnalogSignal::AnalogWaveform>(w));
}

/*!
    This function is called when the amplitude is changed.
*/
void UiEditAnalog::amplitudeChanged(double v)
{
    mSignal->setAmplitude(v);
}
