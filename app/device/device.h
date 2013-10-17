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
#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>

#include "capturedevice.h"
#include "generatordevice.h"

class Device : public QObject
{
    Q_OBJECT
public:
    explicit Device(QObject *parent = 0);

    virtual QString name() const = 0;
    virtual bool isAvailable() const = 0;

    virtual bool supportsCaptureDevice() const {return (captureDevice() != NULL);}
    virtual CaptureDevice* captureDevice() const {return NULL;}

    virtual bool supportsGeneratorDevice() const {
        return (generatorDevice() != NULL);}
    virtual GeneratorDevice* generatorDevice() const {return NULL;}
    
signals:
    void availableStatusChanged(Device* device);
    
public slots:
    
};

#endif // DEVICE_H
