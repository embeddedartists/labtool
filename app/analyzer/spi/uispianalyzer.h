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
#ifndef UISPIANALYZER_H
#define UISPIANALYZER_H

#include <QWidget>

#include "analyzer/uianalyzer.h"
#include "capture/uicursor.h"

/*!
    \class SpiItem
    \brief Container class for SPi items.

    \ingroup Analyzer

    \internal

*/
class SpiItem {
public:

    /*!
        SPI item type
    */
    enum ItemType {
        TYPE_DATA,
        TYPE_FRAME_ERROR
    };

    // default constructor needed in order to add this to QVector
    /*! Default constructor */
    SpiItem() {
    }

    /*! Constructs a new container */
    SpiItem(ItemType type, int mosiValue, int misoValue, int startIdx, int stopIdx) {
        this->type = type;
        this->mosiValue = mosiValue;
        this->misoValue = misoValue;
        this->startIdx = startIdx;
        this->stopIdx = stopIdx;
    }

    /*! type */
    ItemType type;
    /*! mosi value */
    int mosiValue;
    /*! miso value */
    int misoValue;
    /*! item start index */
    int startIdx;
    /*! item stop index */
    int stopIdx;
};

class UiSpiAnalyzer : public UiAnalyzer
{
    Q_OBJECT
public:

    static const QString signalName;

    explicit UiSpiAnalyzer(QWidget *parent = 0);

    void setSckSignal(int id);
    int sckSignal() const {return mSckSignalId;}

    void setMosiSignal(int id);
    int mosiSignal() const {return mMosiSignalId;}

    void setMisoSignal(int id);
    int misoSignal() const {return mMisoSignalId;}

    void setEnableSignal(int id);
    int enableSignal() const {return mEnableSignalId;}

    void setRate(int rate) {mRate = rate;}
    int rate() const {return mRate;}

    void setDataBits(int bits) {mDataBits = bits;}
    int dataBits() const {return mDataBits;}

    void setMode(Types::SpiMode mode) {mMode = mode;}
    Types::SpiMode mode() const {return mMode;}

    void setEnableMode(Types::SpiEnable mode) {mEnableMode = mode;}
    Types::SpiEnable enableMode() const {return mEnableMode;}

    void setDataFormat(Types::DataFormat format) {mFormat = format;}
    Types::DataFormat dataFormat() const {return mFormat;}

    void setSyncCursor(UiCursor::CursorId id) {mSyncCursor = id;}
    UiCursor::CursorId syncCursor() const {return mSyncCursor;}

    void analyze();
    void configure(QWidget* parent);

    QString toSettingsString() const;
    static UiSpiAnalyzer* fromSettingsString(const QString &settings);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent* event);
    

private:

    enum {
        SignalIdMarginRight = 10
    };

    int mSckSignalId;
    int mMosiSignalId;
    int mMisoSignalId;
    int mEnableSignalId;
    int mRate;
    int mDataBits;
    Types::SpiMode mMode;
    Types::SpiEnable mEnableMode;

    Types::DataFormat mFormat;

    UiCursor::CursorId mSyncCursor;

    QLabel* mSckLbl;
    QLabel* mMosiLbl;
    QLabel* mMisoLbl;
    QLabel* mEnableLbl;

    QVector<SpiItem> mSpiItems;

    static int spiAnalyzerCounter;

    void infoWidthChanged();
    void doLayout();
    int calcMinimumWidth();

    void typeAndValueAsString(SpiItem::ItemType type,
                                 int value,
                                 QString &shortTxt,
                                 QString &longTxt);

    void paintSignal(QPainter* painter, double from, double to,
                     int h, QString &shortTxt, QString &longTxt);
};

#endif // UISPIANALYZER_H
