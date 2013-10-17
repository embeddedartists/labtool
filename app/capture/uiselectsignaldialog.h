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
#ifndef UISELECTSIGNALDIALOG_H
#define UISELECTSIGNALDIALOG_H

#include <QWidget>
#include <QDialog>
#include <QList>
#include <QComboBox>
#include <QCheckBox>
#include <QMap>

#include "uiabstractsignal.h"

class UiSelectSignalDialog : public QDialog
{
    Q_OBJECT
public:
    explicit UiSelectSignalDialog(QWidget *parent = 0);
    QList<int> selectedDigitalSignals();
    QList<int> selectedAnalogSignals();
    QString selectedAnalyzer();

signals:
    
public slots:

private:

    QMap<int, QCheckBox*> mDigitalSignalsMap;
    QMap<int, QCheckBox*> mAnalogSignalsMap;
    QComboBox* mAnalyzersBox;

    QWidget *createDigitalSignalBox(QList<int> &list);
    QWidget *createAnalogSignalBox(QList<int> &list);
    QComboBox* createAnalyzerBox();
    
};

#endif // UISELECTSIGNALDIALOG_H
