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
#include "devicemanager.h"

#include "simulator/simulatordevice.h"
#include "labtool/labtooldevice.h"

/*!
    \class DeviceManager
    \brief The DeviceManager class is responsible for providing access to
           supported and active devices.

    \ingroup Device

    Supported devices are created and registered with the device manager.
    Whenever the supported devices or the currently active device must
    be retrieved the device manager must be used.
*/

/*!
    Constructs the device manager with the given \a parent. All supported
    devices are created at this time.
*/
DeviceManager::DeviceManager(QObject *parent) :
    QObject(parent)
{

    //
    // Create list with supported devices
    //
    // Note: If an additional device should be supported it must be created
    //       here and added to the device list.
    //
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    //
    mDevices = QList<Device *>()
            << new SimulatorDevice(this)
            << new LabToolDevice(this);

    // the Simulator device (at index 0) is considered the default device
    mActiveDevice = mDevices.at(0);
}

/*!
    \fn DeviceManager& DeviceManager::instance()

    This function returns an instance of the DeviceManager which is a
    singleton class.
*/

/*!
    Returns a list with supported devices.
*/
QList<Device *> DeviceManager::devices() const
{
    return mDevices;
}

/*!
    Returns the active (selected) device. There can only be one active
    device at a time.
*/
Device* DeviceManager::activeDevice() const
{
    return mActiveDevice;
}

/*!
    Sets the active device to \a device.
*/
void DeviceManager::setActiveDevice(Device *device)
{
    mActiveDevice = device;
}
