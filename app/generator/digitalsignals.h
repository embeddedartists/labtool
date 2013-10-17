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
#ifndef DIGITALSIGNALS_H
#define DIGITALSIGNALS_H

#include <QDebug>
#include <QAbstractTableModel>
#include <QList>

#include "device/digitalsignal.h"

class DigitalSignals : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit DigitalSignals(QObject *parent = 0);

    int numStates() const {return mNumStates;}
    void setNumStates(int numStates);

    DigitalSignal* addSignal(int id);
    void syncSignalsWithDevice();
    void removeSignal(DigitalSignal *s);
    void removeAllSignals();

    /*
        Methods overriden from the abstract table model
     */

    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    //
    // http://qt-project.org/doc/qt-4.8/model-view-programming.html#model-subclassing-reference
    //

signals:
    
public slots:

private:
    int mNumStates;

};

#endif // DIGITALSIGNALS_H
