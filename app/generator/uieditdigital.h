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
#ifndef UIEDITDIGITAL_H
#define UIEDITDIGITAL_H

#include <QWidget>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QComboBox>

#include "device/digitalsignal.h"

class UiEditDigital : public QWidget
{
    Q_OBJECT
public:
    explicit UiEditDigital(DigitalSignal* signal, QWidget *parent = 0);
    
signals:
    
public slots:

private:
    DigitalSignal* mSignal;
    QVBoxLayout* mMainLayout;
    QComboBox* mGenTypeBox;
    QWidget* mTypeWidget;
    QLineEdit* mNameEdit;

    QStringList generateTypes();
    QWidget* createType(QString type);
    bool generateOutput(QString type, QWidget *w, QVector<bool> &data, QString &warnMsg);


    QWidget* createTypeConstant();
    bool generateConstantOutput(QWidget *w, QVector<bool> &data, QString &warnMsg);
    QWidget* createTypeClock();
    bool generateClockOutput(QWidget *w, QVector<bool> &data, QString &warnMsg);

    void setStates(QVector<bool> &data, bool high, int from, int to) const;

private slots:
    void handleNameEdited();
    void handleTypeChanged(QString type);
    void generateOutput();


    
};

#endif // UIEDITDIGITAL_H
