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
#ifndef UIUARTANALYZER_H
#define UIUARTANALYZER_H

#include <QWidget>

#include "analyzer/uianalyzer.h"
#include "capture/uicursor.h"

/*!
    \class UartItem
    \brief Container class for UART items.

    \ingroup Analyzer

    \internal

*/
class UartItem {
public:

    /*!
        UART item type
    */
    enum ItemType {
        TYPE_DATA,
        TYPE_FRAME_ERROR,
        TYPE_PARITY_ERROR
    };

    // default constructor needed in order to add this to QVector
    /*! Default constructor */
    UartItem() {
    }

    /*! Constructs a new container */
    UartItem(ItemType type, int value, int startIdx, int stopIdx) {
        this->type = type;
        this->value = value;
        this->startIdx = startIdx;
        this->stopIdx = stopIdx;
    }

    /*! type */
    ItemType type;
    /*! value */
    int value;
    /*! item start index */
    int startIdx;
    /*! item stop index */
    int stopIdx;    

};

class UiUartAnalyzer : public UiAnalyzer
{
    Q_OBJECT
public:
    static const QString name;


    explicit UiUartAnalyzer(QWidget *parent = 0);

    void setSignalId(int signalId);
    int signalId() const {return mSignalId;}

    void setDataFormat(Types::DataFormat format);
    Types::DataFormat dataFormat() const {return mFormat;}

    void setBaudRate(int rate) {if (rate > 0) mBaudRate = rate;}
    int baudRate() const {return mBaudRate;}

    void setStopBits(int bits) {if (bits > 0) mStopBits = bits;}
    int stopBits() const {return mStopBits;}

    void setParity(Types::UartParity parity) {mParity = parity;}
    Types::UartParity parity() const {return mParity;}

    void setDataBits(int bits) {if (bits > 0) mDataBits = bits;}
    int dataBits() const {return mDataBits;}

    void setSyncCursor(UiCursor::CursorId id) {mSyncCursor = id;}
    UiCursor::CursorId syncCursor() const {return mSyncCursor;}

    void analyze();
    void configure(QWidget* parent);

    QString toSettingsString() const;
    static UiUartAnalyzer* fromSettingsString(const QString &settings);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent* event);

private:

    enum {
        SignalIdMarginRight = 10
    };

    enum UartState {
        STATE_START,
        STATE_DATA,
        STATE_PARITY,
        STATE_STOP
    };

    static int uartAnalyzerCounter;
    int mSignalId;
    int mBaudRate;
    int mDataBits;
    int mStopBits;
    Types::UartParity mParity;
    Types::DataFormat mFormat;
    UiCursor::CursorId mSyncCursor;

    QLabel* mSignalLbl;

    QVector<UartItem> mUartItems;

    void infoWidthChanged();
    void doLayout();
    int calcMinimumWidth();

    void typeAndValueAsString(UartItem::ItemType type,
                                 int value,
                                 QString &shortTxt,
                                 QString &longTxt);
    
};

#endif // UIUARTANALYZER_H
