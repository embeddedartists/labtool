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
#ifndef UIANALYZER_H
#define UIANALYZER_H

#include <QObject>
#include <QWidget>

#include "common/types.h"
#include "capture/uisimpleabstractsignal.h"


class UiAnalyzer : public UiSimpleAbstractSignal
{
    Q_OBJECT
public:


    explicit UiAnalyzer(QWidget *parent = 0);

    virtual void analyze() = 0;
    virtual QString toSettingsString() const = 0;
    void handleSignalDataChanged();

    
signals:
    
public slots:
    virtual void configure(QWidget* parent) = 0;


protected:
    QString formatValue(Types::DataFormat format, int value);


    
};

#endif // UIANALYZER_H
