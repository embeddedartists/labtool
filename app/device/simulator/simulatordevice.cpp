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
#include "simulatordevice.h"

/*!
    \class SimulatorDevice
    \brief A device that allows a user to test the application capabilities
        without any hardware.

    \ingroup Device

    In some Logic Analyzer and Oscilloscope software it is known as Demo mode
    to be able to test the software without any hardware. This simulator device
    allows a user to test the application without having access to any
    hardware.
*/

/*!
    Constructs a simulator device with the given \a parent.
*/
SimulatorDevice::SimulatorDevice(QObject *parent) :
    Device(parent)
{
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mCaptureDevice = new SimulatorCaptureDevice(this);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mGeneratorDevice = new SimulatorGeneratorDevice(this);
}
