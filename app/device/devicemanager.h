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
#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include <QObject>
#include <QList>

#include "device.h"

class DeviceManager : public QObject
{
    Q_OBJECT
public:


    static DeviceManager& instance()
    {
        static DeviceManager singleton;
        return singleton;
    }

    QList<Device*> devices() const;
    Device *activeDevice() const;
    void setActiveDevice(Device* device);
    
signals:
    
public slots:

private:
    explicit DeviceManager(QObject *parent = 0);
    // hide copy constructor
    DeviceManager(const DeviceManager&);
    // hide assign operator
    DeviceManager& operator=(const DeviceManager &);

    QList<Device*> mDevices;
    Device* mActiveDevice;

};

#endif // DEVICEMANAGER_H
