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
#include "digitalsignals.h"

#include <QDebug>
#include <QList>

#include "device/devicemanager.h"

/*!
    \class DigitalSignals
    \brief This class provides interface between the view of digital signals
    and the source of the signals.

    \ingroup Generator

    Digital signals are typically visualized in a table as rows and columns.
    This class provides the table model which the table view is using to get
    access to the digital signal data.
*/

/*!
    Constructs a DigitalSignals with the given \a parent.
*/
DigitalSignals::DigitalSignals(QObject *parent) :
    QAbstractTableModel(parent)
{
    mNumStates = 32;
}

/*!
    \fn int DigitalSignals::numStates()

    Returns the number of states set for digital signals.
*/


/*!
    Sets the number of valid states for all signals to \a numStates.
*/
void DigitalSignals::setNumStates(int numStates)
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (numStates > 0 && numStates != mNumStates && device != NULL) {
        bool add = true;
        if (numStates < mNumStates) {
            add = false;
        }

        // the first column contains the signal name which means that number of
        // columns are number of states + 1

        if (add) {
            beginInsertColumns(QModelIndex(), mNumStates+1, numStates);
        }
        else {
            beginRemoveColumns(QModelIndex(), numStates+1, mNumStates);
        }


        mNumStates = numStates;

        foreach(DigitalSignal* s, device->digitalSignals()) {
            s->setNumStates(numStates);
        }


        if (add) {
            endInsertColumns();
        }
        else {
            endRemoveColumns();
        }
    }
}

/*!
    Adds a new digital signal with ID \a id.
*/
DigitalSignal *DigitalSignals::addSignal(int id)
{

    DigitalSignal* signal = NULL;

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return NULL;

    int idx = device->digitalSignals().size();
    beginInsertRows(QModelIndex(), idx, idx);

    signal = device->addDigitalSignal(id);
    if (signal != NULL) {
        signal->setNumStates(mNumStates);
    }

    endInsertRows();

    return signal;
}

/*!
    Syncs this model with the device to make sure the correct signals are
    shown in the view.
*/
void DigitalSignals::syncSignalsWithDevice()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    // it's enough to notify the view that the data is invalid
    // the view will then reload the data
    beginResetModel();
    endResetModel();
}

/*!
    Remove the digital signal \a s from the available signals.
*/
void DigitalSignals::removeSignal(DigitalSignal* s)
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;

    QList<DigitalSignal*> signalList = device->digitalSignals();
    int idx = signalList.indexOf(s);
    if (idx == -1) return;

    beginRemoveRows(QModelIndex(), idx, idx);    
    device->removeDigitalSignal(s);
    endRemoveRows();
}

/*!
    Remove all digital signals.
*/
void DigitalSignals::removeAllSignals()
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();
    if (device == NULL) return;
    QList<DigitalSignal*> digitalSignals = device->digitalSignals();

    if (digitalSignals.isEmpty()) return;

    beginRemoveRows(QModelIndex(), 0, digitalSignals.size()-1);
    device->removeAllDigitalSignals();
    endRemoveRows();
}


/*
    ---------------------------------------------------------------------------
    #### Methods overriden from the abstract table model ######################
    ---------------------------------------------------------------------------
*/

/*!
    Reimplemented from QAbstractTableModel::flags
*/
Qt::ItemFlags DigitalSignals::flags(const QModelIndex &index) const
{
    if (!index.isValid()) {
        return Qt::ItemIsEnabled;
    }

    return Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled;
}

/*!
    Reimplemented from QAbstractTableModel::data
*/
QVariant DigitalSignals::data(const QModelIndex &index, int role) const
{
    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();


    if (!index.isValid() || device == NULL) {
        return QVariant();
    }

    // the first column contains the signal name which means that number of
    // columns are number of states + 1

    QList<DigitalSignal*> list = device->digitalSignals();

    if (index.row() >= list.size() || index.column() >= (mNumStates+1)) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        DigitalSignal* s = list.at(index.row());


        return qVariantFromValue<DigitalSignal*>(s);
    }
    else if (role == Qt::ToolTipRole && index.column() == 0) {
        return tr("Double-click to configure");
    }

    return QVariant();
}

/*!
    Reimplemented from QAbstractTableModel::headerData
*/
QVariant DigitalSignals::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (role == Qt::DisplayRole && device != NULL) {
        switch(orientation) {
        case Qt::Horizontal:
            if (section == 0) {
                // The spaces below are needed to get an initial column width
                return tr("Signal              ");
            }
            else if (section <= mNumStates) {
                // return state number
                return QString("%1").arg(section-1);
            }
            break;
        case Qt::Vertical:
            QList<DigitalSignal*> list = device->digitalSignals();

            if (section < list.size()) {
                int id = list.at(section)->id();
                return QString("D%1").arg(id);
            }
            break;
        }
    }

    return QVariant();
}

/*!
    Reimplemented from QAbstractTableModel::rowCount
*/
int DigitalSignals::rowCount(const QModelIndex &parent) const
{
    (void)parent;

    int cnt = 0;

    GeneratorDevice* device = DeviceManager::instance().activeDevice()
            ->generatorDevice();

    if (device != NULL) {
        cnt = device->digitalSignals().size();
    }


    return cnt;
}

/*!
    Reimplemented from QAbstractTableModel::columnCount
*/
int DigitalSignals::columnCount(const QModelIndex &parent) const
{
    (void)parent;

    // the first column contains the signal name, the rest are the signal
    // states
    return numStates() + 1;
}
