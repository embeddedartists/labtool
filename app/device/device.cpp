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
#include "device.h"

/*!
    \class Device
    \brief Device is the abstract base class of all devices.

    \ingroup Device

    A device that should be supported by this application must be a subclass
    of this abstract base class.
*/

/*!
    Constructs a device with the given \a parent. This class will never
    be instantiated directly. Instead a subclass will inherit from this class.
*/
Device::Device(QObject *parent) :
    QObject(parent)
{
}

/*!
    \fn virtual void Device::name() const = 0

    Returns the name of the device.
*/

/*!
    \fn virtual bool Device::isAvailable() const = 0

    Returns the availability state of the device, that is, if the device
    is considered to be accessible or not.

    \sa availableStatusChanged(Device* device)
*/

/*!
    \fn virtual bool Device::supportsCaptureDevice() const

    Returns true if this device supports the Capture functionality.

    \sa captureDevice()
*/

/*!
    \fn virtual CaptureDevice* Device::captureDevice() const

    Returns the CaptureDevice interface for this device or NULL if the Capture
    functionality isn't supported.

    \sa supportsCaptureDevice()
*/

/*!
    \fn virtual bool Device::supportsGeneratorDevice() const

    Returns true if this device supports the Generator functionality.

    \sa generatorDevice()
*/

/*!
    \fn virtual GeneratorDevice* Device::generatorDevice() const

    Returns the GeneratorDevice interface for this device or NULL if the
    Generator functionality isn't supported.

    \sa supportsCaptureDevice()
*/


/*!
    \fn void Device::availableStatusChanged(Device* device)

    This signal is emitted when the availability status of the device
    changes, for example, if the device is disconnected from the computer.

    \sa isAvailable()
*/
