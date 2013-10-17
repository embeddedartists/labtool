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
#ifndef UII2CANALYZER_H
#define UII2CANALYZER_H


#include "analyzer/uianalyzer.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVector>

#include "capture/uicursor.h"

/*!
    \class I2CItem
    \brief Container class for I2C items.

    \ingroup Analyzer

    \internal

*/
class I2CItem {
public:

    /*!
        I2C protocol types
    */
    enum I2CType {
        I2C_START,
        I2C_STOP,
        I2C_ACK,
        I2C_NACK,
        I2C_DATA,
        I2C_7_ADDRESS_WRITE,
        I2C_7_ADDRESS_READ,
        I2C_10_ADDRESS_WRITE,
        I2C_10_ADDRESS_READ,
        I2C_ERROR
    };

    // default constructor needed in order to add this to QVector
    /*!
        Default constructor
    */
    I2CItem() {
    }

    /*!
        Creates an I2C container item
    */
    I2CItem(I2CType type, int value, int startIdx, int stopIdx) {
        this->type = type;
        this->value = value;
        this->startIdx = startIdx;
        this->stopIdx = stopIdx;
    }

    /*! type */
    I2CType type;
    /*! value */
    int value;
    /*! sample index where item starts */
    int startIdx;
    /*! sample index where item stop */
    int stopIdx;
};



class UiI2CAnalyzer : public UiAnalyzer
{
    Q_OBJECT
public:

    static const QString signalName;

    explicit UiI2CAnalyzer(QWidget *parent = 0);

    void setSclSignalId(int sclSignalId);
    void setSdaSignalId(int sdaSignalId);
    void setDataFormat(Types::DataFormat format);
    int sclSignalId() {return mSclSignalId;}
    int sdaSignalId() {return mSdaSignalId;}
    Types::DataFormat dataFormat() {return mFormat;}

    void setSyncCursor(UiCursor::CursorId id) {mSyncCursor = id;}
    UiCursor::CursorId syncCursor() {return mSyncCursor;}

    void analyze();
    void configure(QWidget* parent);

    QString toSettingsString() const;
    static UiI2CAnalyzer* fromSettingsString(const QString &settings);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent* event);


private:

    enum {
        MaxNumBusErrors = 5,
        SignalIdMarginRight = 10
    };

    int mSclSignalId;
    int mSdaSignalId;
    Types::DataFormat mFormat;

    QLabel* mSclLbl;
    QLabel* mSdaLbl;
    UiCursor::CursorId mSyncCursor;


    static int i2cAnalyzerCounter;


    QVector<I2CItem> mI2cItems;

    void typeAndValueAsString(I2CItem::I2CType type, int value, QString &shortTxt, QString &longTxt);

    void infoWidthChanged();
    void doLayout();
    int calcMinimumWidth();

};

#endif // UII2CANALYZER_H
