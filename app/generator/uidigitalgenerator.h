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
#ifndef UIDIGITALGENERATOR_H
#define UIDIGITALGENERATOR_H

#include <QWidget>
#include <QTableView>
#include <QComboBox>
#include <QSpinBox>
#include <QToolBar>
#include <QLineEdit>
#include <QAction>

#include "digitalsignals.h"


class UiDigitalGenerator : public QWidget
{
    Q_OBJECT
public:
    explicit UiDigitalGenerator(DigitalSignals* digitalSignals, QWidget *parent = 0);
    int rate();
    void setRate(int rate);

    void removeAllSignals();
    
signals:
    
public slots:
    void setNumStates(int states);
    void handleDeviceChanged();

private:

    QTableView *mTable;
    DigitalSignals* mSignals;

    QLineEdit* mRate;
    QString mLastRateText;
    QSpinBox* mStatesBox;

    QAction* mAddAction;
    QAction* mRemoveAction;

    QToolBar* createToolBar();
    QTableView* createTable();
    QLineEdit* createRateBox();
    QSpinBox* createStatesBox();

    QList<DigitalSignal*> selectedSignals();

private slots:
    void addSignal();
    void removeSelectedSignals();
    void handleSelectionChanged(QItemSelection selected, QItemSelection deselected);
    void updateRate();
    
};

#endif // UIDIGITALGENERATOR_H
