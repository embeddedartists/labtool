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
#ifndef UIGENERATORSIGNALDIALOG_H
#define UIGENERATORSIGNALDIALOG_H

#include <QDialog>
#include <QList>
#include <QCheckBox>
#include <QMap>

class UiGeneratorSignalDialog : public QDialog
{
    Q_OBJECT
public:

    enum SignalType {
        SignalDigital = 0x01,
        SignalAnalog = 0x02
    };


    explicit UiGeneratorSignalDialog(QMap<SignalType, QList<int> > unused, QWidget *parent);
    QList<int> selectedSignals(SignalType type);
    
signals:
    
public slots:

private:
    QMap<int, QCheckBox*> mDigitalSignalsMap;
    QMap<int, QCheckBox*> mAnalogSignalsMap;

    QWidget *createSignalBox(SignalType type, QList<int> &list);
    
};

#endif // UIGENERATORSIGNALDIALOG_H
