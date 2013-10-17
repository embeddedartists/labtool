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
#include "uilabtooltriggerconfig.h"

#include <QFormLayout>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QIntValidator>
#include <QLabel>

// Noise reduction is disabled for now as it causes problems and is only
// stable for very low sample rates.
//#define ENABLE_NOISE_FILTER

/*!
    \class UiLabToolTriggerConfig
    \brief A dialog with trigger settings for the LabTool Hardware.

    \ingroup Device

    The UiLabToolTriggerConfig dialog provides configuration options for:

    - Post-fill Percentage

        Specifies how much of the capture buffer should be used after a trigger has been found.

        Example: If the sample buffer can hold 1000 samples and the post fill percentage is
                 set to 30% then after a trigger the sampling will continue for an additional 300 samples
                 before the data is sent to the PC.
    - Post-fill Max Time

        A maximum time limit can be set to avoid the long delays that might occur for low sample rates.

        Example: Assuming the same settings as in the example above, with a sample rate of 50Hz that will result in a
                 15 second delay before the result is sent. By setting the post fill time limit to
                 1000ms the hardware will only take an additional 20 (instead of 300) samples after the trigger and
                 return one second after the trigger.
    - Noise Reduction

        Enable the noise reduction filter to reduce the risk of finding incorrect trigger points.
        The filter level will dictate how much is filtered out. Setting the level too high or too low can result in
        missed trigger points.
*/

/*!
    Constructs a new trigger dialog for the given \a comm.
*/
UiLabToolTriggerConfig::UiLabToolTriggerConfig(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("Trigger Settings"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QFormLayout* formLayout = new QFormLayout;


    // post-fill percentage
    mPostFillPercent = new QSlider(Qt::Horizontal, this);
    mPostFillPercent->setToolTip(tr("Percent of capture buffer reserved for samples after the trigger"));
    mPostFillPercent->setRange(0, 100);
    mPostFillPercent->setSingleStep(5);
    mPostFillPercent->setTickPosition(QSlider::TicksAbove);
    mPostFillPercent->setValue(50); // 50% of capture buffer filled after trigger point

    QLabel* infoLbl = new QLabel(tr("Specifies how much of the capture buffer should be used after a trigger has been found."
                                 "\nExample: If the sample buffer can hold 1000 samples and the post fill percentage is "
                                 "set to 30% then after a trigger the sampling will continue for an additional 300 samples "
                                 "before the data is sent to the PC."), this);
    infoLbl->setWordWrap(true);
    formLayout->addRow(infoLbl);

    QLabel* percLbl = new QLabel("50", this);
    percLbl->setMinimumWidth(18);
    QHBoxLayout* pfhLayout = new QHBoxLayout();
    pfhLayout->addWidget(percLbl);
    pfhLayout->addWidget(mPostFillPercent);

    connect(mPostFillPercent, SIGNAL(valueChanged(int)), percLbl, SLOT(setNum(int)));

    formLayout->addRow(tr("Post-Fill (%): "), pfhLayout);

    QLabel* infoLbl2 = new QLabel(tr("A maximum time limit can be set to avoid the long delays that might occur for low sample rates."
                                  "\nExample: Assuming the same settings as in the example above, with a sample rate of 50Hz that will result in a "
                                  "15 second delay before the result is sent. By setting the post fill time limit to "
                                  "1000ms the hardware will only take an additional 20 (instead of 300) samples after the trigger and "
                                  "return one second after the trigger."), this);
    infoLbl2->setWordWrap(true);
    formLayout->addRow(infoLbl2);

    // max time for post-fill
    mPostFillTimeLimit = new QLineEdit(this);
    mPostFillTimeLimit->setToolTip(tr("Maximum time spent on post-fill (in ms)"));
    QIntValidator* intValidator = new QIntValidator(0, 600000, this);
    mPostFillTimeLimit->setValidator(intValidator);
    mPostFillTimeLimit->setText(QString("%1").arg(1000)); // 1 second limit

    formLayout->addRow(tr("Time limit (ms): "), mPostFillTimeLimit);

#ifdef ENABLE_NOISE_FILTER
    QLabel* infoLbl3 = new QLabel(tr("Enable the noise reduction filter to reduce the risk of finding incorrect trigger points."
                                  "The filter level will dictate how much is filtered out. Setting the level too high or too low can result in "
                                  "missed trigger points."), this);
    infoLbl3->setWordWrap(true);
    formLayout->addRow(infoLbl3);

    // noise reduction
    mNoiseFilterEnabled = new QCheckBox(this);
    mNoiseFilterEnabled->setTristate(false);
    mNoiseFilterEnabled->setCheckState(Qt::Unchecked);
    mNoiseLevel = new QSlider(Qt::Horizontal, this);
    mNoiseLevel->setToolTip(tr("How much noise to filter out"));
    mNoiseLevel->setRange(1, 10);
    mNoiseLevel->setEnabled(false);
    mNoiseLevel->setSingleStep(1);
    mNoiseLevel->setTickPosition(QSlider::TicksAbove);
    mNoiseLevel->setTickInterval(1);
    mNoiseLevel->setValue(5);
    QLabel* noiseLbl = new QLabel("5", this);
    noiseLbl->setMinimumWidth(18);
    QHBoxLayout* pfhLayout2 = new QHBoxLayout();
    pfhLayout2->addWidget(mNoiseFilterEnabled);
    pfhLayout2->addWidget(noiseLbl);
    pfhLayout2->addWidget(mNoiseLevel);

    connect(mNoiseLevel, SIGNAL(valueChanged(int)), noiseLbl, SLOT(setNum(int)));
    connect(mNoiseFilterEnabled, SIGNAL(stateChanged(int)), this, SLOT(noiseFilterStateChanged(int)));

    formLayout->addRow(tr("Noise Filter: "), pfhLayout2);
#endif






    QVBoxLayout* verticalLayout = new QVBoxLayout();

    QDialogButtonBox* bottonBox = new QDialogButtonBox(
                QDialogButtonBox::Ok,
                Qt::Horizontal,
                this);
    bottonBox->setCenterButtons(true);

    connect(bottonBox, SIGNAL(accepted()), this, SLOT(accept()));

    verticalLayout->addLayout(formLayout);
    verticalLayout->addWidget(bottonBox);


    setLayout(verticalLayout);
}

/*!
    Sets the post-fill time limit.
*/
void UiLabToolTriggerConfig::setPostFillTimeLimit(int ms)
{
    mPostFillTimeLimit->setText(QString("%1").arg(ms));
}

/*!
    Returns the the post-fill time limit in milliseconds.
*/
int UiLabToolTriggerConfig::postFillTimeLimit()
{
    return mPostFillTimeLimit->text().toInt();
}

/*!
    Sets the post-fill percent.
*/
void UiLabToolTriggerConfig::setPostFillPercent(int percent)
{
    mPostFillPercent->setValue(percent);
}

/*!
    Returns the the post-fill percent.
*/
int UiLabToolTriggerConfig::postFillPercent()
{
    return mPostFillPercent->value();
}

/*!
    Sets up the noisefilter. The \a level parameter is in bits (1..10)
*/
void UiLabToolTriggerConfig::setNoiseFilter(bool enabled, int level)
{
    // Noise reduction is disabled for now as it causes problems and is only
    // stable for very low sample rates.
#ifdef ENABLE_NOISE_FILTER
    if (enabled) {
        mNoiseFilterEnabled->setCheckState(Qt::Checked);
    } else {
        mNoiseFilterEnabled->setCheckState(Qt::Unchecked);
    }
    mNoiseLevel->setValue(level);
#else
    (void)enabled;
    (void)level;
#endif
}

/*!
    Returns true if the noise filter is enabled
*/
bool UiLabToolTriggerConfig::isNoiseFilterEnabled()
{
    // Noise reduction is disabled for now as it causes problems and is only
    // stable for very low sample rates.
#ifdef ENABLE_NOISE_FILTER
    return (mNoiseFilterEnabled->checkState() == Qt::Checked);
#else
    return false;
#endif
}

/*!
    Returns the noise filter level (in bits 1..10)
*/
int UiLabToolTriggerConfig::noiseFilterLevel()
{
#ifdef ENABLE_NOISE_FILTER
    return mNoiseLevel->value();
#else
    return 5;
#endif
}

/*!
    \fn qint16 UiLabToolTriggerConfig::noiseFilter12BitLevel()

    Returns the noise filter level as an integer in the 0..4096 range
*/

/*!
    Acts on the enabling/disabling of the noise filter and
    enables/disables the noise filter level slider accordingly.
*/
void UiLabToolTriggerConfig::noiseFilterStateChanged(int state)
{
#ifdef ENABLE_NOISE_FILTER
    if (state == Qt::Unchecked)
    {
        mNoiseLevel->setEnabled(false);
    }
    if (state == Qt::Checked)
    {
        mNoiseLevel->setEnabled(true);
    }
#else
    (void)state;
#endif
}
