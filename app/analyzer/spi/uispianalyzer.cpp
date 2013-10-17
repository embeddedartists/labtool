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
#include "uispianalyzer.h"

#include <QPainter>

#include "uispianalyzerconfig.h"
#include "common/configuration.h"
#include "capture/cursormanager.h"
#include "device/devicemanager.h"

/*!
    Counter used when creating the editable name.
*/
int UiSpiAnalyzer::spiAnalyzerCounter = 0;

/*!
    Name of this analyzer.
*/
const QString UiSpiAnalyzer::signalName = "SPI Analyzer";

/*!
    \class UiSpiAnalyzer
    \brief This class is an SPI protocol analyzer.

    \ingroup Analyzer

    The class will analyze specified digital signals and visualize the
    interpretation as SPI protocol data.

*/


/*!
    Constructs the UiSpiAnalyzer with the given \a parent.
*/
UiSpiAnalyzer::UiSpiAnalyzer(QWidget *parent) :
    UiAnalyzer(parent)
{
    mSckSignalId = -1;
    mMosiSignalId = -1;
    mMisoSignalId = -1;
    mEnableSignalId = -1;

    mRate = 1000000;
    mDataBits = 8;
    mMode = Types::SpiMode_0;
    mEnableMode = Types::SpiEnableLow;
    mFormat = Types::DataFormatHex;
    mSyncCursor = UiCursor::NoCursor;

    mIdLbl->setText("SPI");
    mNameLbl->setText(QString("SPI %1").arg(spiAnalyzerCounter++));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mSckLbl = new QLabel(this);
    mSckLbl->setAlignment(Qt::AlignRight);
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mMosiLbl = new QLabel(this);
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mMisoLbl = new QLabel(this);
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mEnableLbl = new QLabel(this);
    mEnableLbl->setAlignment(Qt::AlignRight);

    QPalette palette= mSckLbl->palette();
    palette.setColor(QPalette::Text, Qt::gray);
    mSckLbl->setPalette(palette);
    mMosiLbl->setPalette(palette);
    mMisoLbl->setPalette(palette);
    mEnableLbl->setPalette(palette);

    setFixedHeight(60);
}

/*!
    Set the SCK signal ID to \a id.
*/
void UiSpiAnalyzer::setSckSignal(int id)
{
    mSckSignalId = id;
    mSckLbl->setText(QString("SCK: D%1").arg(id));
}


/*!
    \fn int UiSpiAnalyzer::sckSignal() const

    Returns SCK signal ID
*/


/*!
    Set the MOSI signal ID to \a id.
*/
void UiSpiAnalyzer::setMosiSignal(int id)
{
    mMosiSignalId = id;
    mMosiLbl->setText(QString("MOSI: D%1").arg(id));
}


/*!
    \fn int UiSpiAnalyzer::mosiSignal() const

    Returns MOSI signal ID
*/


/*!
    Set the MISO signal ID to \a id.
*/
void UiSpiAnalyzer::setMisoSignal(int id)
{
    mMisoSignalId = id;
    mMisoLbl->setText(QString("MISO: D%1").arg(id));
}


/*!
    \fn int UiSpiAnalyzer::misoSignal() const

    Returns MISO signal ID
*/


/*!
    Set the enable (CS) signal ID to \a id.
*/
void UiSpiAnalyzer::setEnableSignal(int id)
{
    mEnableSignalId = id;
    mEnableLbl->setText(QString("CS: D%1").arg(id));
}

/*!
    \fn int UiSpiAnalyzer::enableSignal() const

    Returns the Enable signal ID
*/

/*!
    \fn void UiSpiAnalyzer::setRate(int rate)

    Set SPI frequency to \a rate.
*/

/*!
    \fn int UiSpiAnalyzer::rate() const

    Returns the SPI frequency.
*/

/*!
    \fn void UiSpiAnalyzer::setDataBits(int bits)

    Set the number of data bits to \a bits.
*/

/*!
    \fn int UiSpiAnalyzer::dataBits() const

    Returns number of data bits.
*/

/*!
    \fn void UiSpiAnalyzer::setMode(Types::SpiMode mode)

    Set the SPI mode to \a mode.
*/

/*!
    \fn Types::SpiMode UiSpiAnalyzer::mode() const

    Returns the SPI mode.
*/

/*!
    \fn void UiSpiAnalyzer::setEnableMode(Types::SpiEnable mode)

    Set the Enable mode to \a mode.
*/

/*!
    \fn Types::SpiEnable UiSpiAnalyzer::enableMode()

    Returns the Enable mode.
*/

/*!
    \fn void UiSpiAnalyzer::setDataFormat(Types::DataFormat format)

    Set the data format to \a format.
*/

/*!
    \fn Types::DataFormat UiSpiAnalyzer::dataFormat() const

    Returns the data format.
*/

/*!
    \fn void UiSpiAnalyzer::setSyncCursor(UiCursor::CursorId id)

    Set the cursor to use for synchronization.
*/

/*!
    \fn UiCursor::CursorId UiSpiAnalyzer::syncCursor() const

    Returns the cursor used for synchronization.
*/


/*!
    Start to analyze the signal data.
*/
void UiSpiAnalyzer::analyze()
{
    mSpiItems.clear();

    if (mSckSignalId == -1 || mMosiSignalId == -1
            ||  mMisoSignalId == -1 ||  mEnableSignalId == -1) return;

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    QVector<int>* sckData = device->digitalData(mSckSignalId);
    QVector<int>* mosiData = device->digitalData(mMosiSignalId);
    QVector<int>* misoData = device->digitalData(mMisoSignalId);
    QVector<int>* enableData = device->digitalData(mEnableSignalId);

    if (sckData == NULL || mosiData == NULL
            || misoData == NULL || enableData == NULL) return;
    if (sckData->size() == 0 || mosiData->size() == 0
            || misoData->size() == 0 || misoData->size() == 0) return;


    bool done = false;
    bool findCsOn = true;
    int pos = 0;

    if (mSyncCursor != UiCursor::NoCursor) {
        double t = CursorManager::instance().cursorPosition(mSyncCursor);
        if (t > 0 && CursorManager::instance().isCursorOn(mSyncCursor)) {
            pos = device->usedSampleRate()*t;
        }

        if (pos >= sckData->size()) {
            pos = 0;
        }

    }


    int prevCs = enableData->at(pos);
    int currCs = 0;
    bool csChanged = false;
    bool csOff = false;


    int prevSck = sckData->at(pos);
    int currSck = 0;
    bool sckChanged = false;
    int sckChangeNum = 0;

    int mosi = 0;
    int miso = 0;
    int mosiValue = 0;
    int misoValue = 0;
    int dataBitCnt = mDataBits;

    int startIdx = -1;

    // CPHA = 0 -> capture data on first clock transition (otherwise second)
    bool captureOnFirst = (mMode == Types::SpiMode_0
                           || mMode == Types::SpiMode_2);



    while (!done) {

        // reached end of data
        if (pos >= sckData->size()) break;

        currCs  = enableData->at(pos);
        csChanged = (prevCs != currCs);

        currSck = sckData->at(pos);
        sckChanged = (prevSck != currSck);
        if (sckChanged) {
            sckChangeNum++;
        }

        mosi = mosiData->at(pos);
        miso = misoData->at(pos);


        do {

            /*
             * Look for Enable on
             */

            if (findCsOn) {

                if (csChanged &&
                        ( ((currCs == 0 && mEnableMode == Types::SpiEnableLow) ||
                          (currCs == 1 && mEnableMode == Types::SpiEnableHigh))))
                {
                    findCsOn = false;
                }

                else {
                    // we've not found enable yet -> get next sample
                    break;
                }
            }

            /*
             * Check if Enable is set to off
             */

            csOff = (csChanged && ((currCs == 1 && mEnableMode == Types::SpiEnableLow)
                                   || (currCs == 0 && mEnableMode == Types::SpiEnableHigh)));

            if (csOff) {
                findCsOn = true;


                // enable signal has been set to off, but we haven't received a complete value
                if (dataBitCnt > 0 && dataBitCnt < 8) {
                    done = true;

                    SpiItem item(SpiItem::TYPE_FRAME_ERROR, 0, 0, startIdx, -1);
                    mSpiItems.append(item);
                }


            }

            // capture data when SCK changes
            if (sckChanged && ((captureOnFirst && (sckChangeNum % 2) != 0)
                    || (!captureOnFirst && (sckChangeNum % 2) == 0))) {

                if (startIdx == -1) {
                    startIdx = pos;
                }

                mosiValue |= (mosi << (--dataBitCnt));
                misoValue |= (miso << (dataBitCnt));



                // captured a complete value
                if (dataBitCnt == 0) {
                    SpiItem item(SpiItem::TYPE_DATA, mosiValue, misoValue,
                                 startIdx, pos);
                    mSpiItems.append(item);

                    startIdx = -1;
                    mosiValue = 0;
                    misoValue = 0;
                    dataBitCnt = mDataBits;
                }



                //sckChangeNum = 0;
            }




        } while (false);

        pos++;
        prevCs = currCs;
        prevSck = currSck;
    }

}

/*!
    Configure the analyzer.
*/
void UiSpiAnalyzer::configure(QWidget *parent)
{
    UiSpiAnalyzerConfig dialog(parent);
    dialog.setSckSignal(mSckSignalId);
    dialog.setMosiSignal(mMosiSignalId);
    dialog.setMisoSignal(mMisoSignalId);
    dialog.setEnableSignal(mEnableSignalId);
    dialog.setDataBits(mDataBits);
    dialog.setDataFormat(mFormat);
    dialog.setEnableMode(mEnableMode);
    dialog.setMode(mMode);
    dialog.setSyncCursor(mSyncCursor);

    dialog.exec();

    setSckSignal(dialog.sckSignal());
    setMosiSignal(dialog.mosiSignal());
    setMisoSignal(dialog.misoSignal());
    setEnableSignal(dialog.enableSignal());
    setDataBits(dialog.dataBits());
    setDataFormat(dialog.dataFormat());
    setEnableMode(dialog.enableMode());
    setMode(dialog.mode());
    setSyncCursor(dialog.syncCursor());

    analyze();
    update();
}

/*!
    Returns a string representation of this analyzer.
*/
QString UiSpiAnalyzer::toSettingsString() const
{
    // type;name;SCK;MOSI;MISO;CS;Format;Mode;EnableMode;DataBits;Sync

    QString str;
    str.append(UiSpiAnalyzer::signalName);str.append(";");
    str.append(getName());str.append(";");
    str.append(QString("%1;").arg(sckSignal()));
    str.append(QString("%1;").arg(mosiSignal()));
    str.append(QString("%1;").arg(misoSignal()));
    str.append(QString("%1;").arg(enableSignal()));
    str.append(QString("%1;").arg(dataFormat()));
    str.append(QString("%1;").arg(mode()));
    str.append(QString("%1;").arg(enableMode()));
    str.append(QString("%1;").arg(dataBits()));
    str.append(QString("%1").arg(syncCursor()));

    return str;
}

/*!
    Create an SPI analyzer from the string representation \a s.

    \sa toSettingsString
*/
UiSpiAnalyzer* UiSpiAnalyzer::fromSettingsString(const QString &s)
{
    UiSpiAnalyzer* analyzer = NULL;
    QString name;

    bool ok = false;

    do {
        // type;name;SCK;MOSI;MISO;CS;Format;Mode;EnableMode;DataBits;Sync
        QStringList list = s.split(';');
        if (list.size() != 11) break;

        // --- type
        if (list.at(0) != UiSpiAnalyzer::signalName) break;

        // --- name
        name = list.at(1);
        if (name.isNull()) break;

        // --- SCK signal ID
        int sckId = list.at(2).toInt(&ok);
        if (!ok) break;

        // --- MOSI signal ID
        int mosiId = list.at(3).toInt(&ok);
        if (!ok) break;

        // --- MISO signal ID
        int misoId = list.at(4).toInt(&ok);
        if (!ok) break;

        // --- CS signal ID
        int csId = list.at(5).toInt(&ok);
        if (!ok) break;

        // --- data format
        Types::DataFormat format;
        int f = list.at(6).toInt(&ok);
        if (!ok) break;
        if (f < 0 || f >= Types::DataFormatNum) break;
        format = (Types::DataFormat)f;

        // --- mode
        int m = list.at(7).toInt(&ok);
        if (!ok) break;
        if (m < 0 || m >= Types::SpiMode_Num) break;
        Types::SpiMode mode = (Types::SpiMode)m;

        // --- enable mode
        int em = list.at(8).toInt(&ok);
        if (!ok) break;
        if (em < 0 || em >= Types::SpiEnableNum) break;
        Types::SpiEnable enableMode = (Types::SpiEnable)em;

        // --- data bits
        int dataBits = list.at(9).toInt(&ok);
        if (!ok) break;

        // --- sync cursor
        int sc = list.at(10).toInt(&ok);
        if (sc < 0 || sc > UiCursor::NumCursors) break;
        UiCursor::CursorId syncCursor = (UiCursor::CursorId)sc;

        // Deallocation: caller of this function must delete the analyzer
        analyzer = new UiSpiAnalyzer();
        if (analyzer == NULL) break;

        analyzer->setSignalName(name);
        analyzer->setSckSignal(sckId);
        analyzer->setMosiSignal(mosiId);
        analyzer->setMisoSignal(misoId);
        analyzer->setEnableSignal(csId);
        analyzer->setDataFormat(format);
        analyzer->setMode(mode);
        analyzer->setEnableMode(enableMode);
        analyzer->setDataBits(dataBits);
        analyzer->setSyncCursor(syncCursor);

    } while (false);

    return analyzer;
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiSpiAnalyzer::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);

    int textMargin = 3;

    // -----------------
    // draw background
    // -----------------
    paintBackground(&painter);

    painter.setClipRect(plotX(), 0, width()-infoWidth(), height());

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    int sampleRate = device->usedSampleRate();


    double from = 0;
    double to = 0;
    int fromIdx = 0;
    int toIdx = 0;

    int h = height() / 6;

    QString mosiShortTxt;
    QString mosiLongTxt;
    QString misoShortTxt;
    QString misoLongTxt;

    if (mSelected) {
        QPen pen = painter.pen();
        pen.setColor(Qt::gray);
        painter.setPen(pen);
        QRectF mosiRect(plotX()+4, height()/4-h, 100, 2*h);
        painter.drawText(mosiRect, Qt::AlignLeft|Qt::AlignVCenter, "MOSI");
        QRectF misoRect(plotX()+4, 3*height()/4-h, 100, 2*h);
        painter.drawText(misoRect, Qt::AlignLeft|Qt::AlignVCenter, "MISO");
    }


    QPen pen = painter.pen();
    pen.setColor(Configuration::instance().analyzerColor());
    painter.setPen(pen);

    for (int i = 0; i < mSpiItems.size(); i++) {
        SpiItem item = mSpiItems.at(i);

        fromIdx = item.startIdx;
        toIdx = item.stopIdx;


        typeAndValueAsString(item.type, item.mosiValue, mosiShortTxt,
                             mosiLongTxt);
        typeAndValueAsString(item.type, item.misoValue, misoShortTxt,
                             misoLongTxt);

        int shortTextWidth = painter.fontMetrics().width(mosiShortTxt);
        int longTextWidth = painter.fontMetrics().width(mosiLongTxt);


        from = mTimeAxis->timeToPixelRelativeRef((double)fromIdx/sampleRate);

        // no need to draw when signal is out of plot area
        if (from > width()) break;

        if (toIdx != -1) {
            to = mTimeAxis->timeToPixelRelativeRef((double)toIdx/sampleRate);
        }
        else  {

            // see if the long text version fits
            to = from + longTextWidth+textMargin*2;

            if (i+1 < mSpiItems.size()) {

                // get position for the start of the next item
                double tmp = mTimeAxis->timeToPixelRelativeRef(
                            (double)mSpiItems.at(i+1).startIdx/sampleRate);


                // if 'to' overlaps check if short text fits
                if (to > tmp) {

                    to = from + shortTextWidth+textMargin*2;

                    // 'to' overlaps next item -> limit to start of next item
                    if (to > tmp) {
                        to = tmp;
                    }

                }


            }
        }


        painter.save();
        painter.translate(0, height()/4);
        paintSignal(&painter, from, to, h, mosiShortTxt, mosiLongTxt);
        painter.restore();

        painter.save();
        painter.translate(0, 3*height()/4);
        paintSignal(&painter, from, to, h, misoShortTxt, misoLongTxt);
        painter.restore();

    }

}

/*!
    Event handler called when this widget is being shown
*/
void UiSpiAnalyzer::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Called when the info width has changed for this widget.
*/
void UiSpiAnalyzer::infoWidthChanged()
{
    doLayout();
}

/*!
    Position the child widgets.
*/
void UiSpiAnalyzer::doLayout()
{
    UiSimpleAbstractSignal::doLayout();

    QRect r = infoContentRect();
    int y = r.top();

    mIdLbl->move(r.left(), y);

    int x = mIdLbl->pos().x()+mIdLbl->width() + SignalIdMarginRight;
    mNameLbl->move(x, y);
    mEditName->move(x, y);

    mMosiLbl->move(r.left(), r.bottom()-mMosiLbl->height()-mSckLbl->height());
    mMisoLbl->move(r.left()+5+mMosiLbl->width(), r.bottom()-mMisoLbl->height()
                   -mSckLbl->height());

    mSckLbl->move(r.left(), r.bottom()-mSckLbl->height());
    mEnableLbl->move(r.left()+5+mMosiLbl->width(),
                     r.bottom()-mSckLbl->height());

    mEnableLbl->resize(mMisoLbl->width(),
                       mEnableLbl->minimumSizeHint().height());
    mSckLbl->resize(mMosiLbl->width(), mSckLbl->minimumSizeHint().height());
}

/*!
    Calculate and return the minimum width for this widget.
*/
int UiSpiAnalyzer::calcMinimumWidth()
{
    int w = mNameLbl->pos().x() + mNameLbl->minimumSizeHint().width();
    if (mEditName->isVisible()) {
        w = mEditName->pos().x() + mEditName->width();
    }

    int w2 = mEnableLbl->pos().x()+mEnableLbl->width();
    if (w2 > w) w = w2;

    w2 = mMisoLbl->pos().x()+mMisoLbl->width();
    if (w2 > w) w = w2;

    return w+infoContentMargin().right();
}

/*!
    Convert SPI \a type and data \a value to string representation. A short
    and long representation is returned in \a shortTxt and \a longTxt.
*/
void UiSpiAnalyzer::typeAndValueAsString(SpiItem::ItemType type,
                                         int value,
                                         QString &shortTxt,
                                         QString &longTxt)
{


    switch(type) {
    case SpiItem::TYPE_DATA:
        shortTxt = formatValue(mFormat, value);
        longTxt = formatValue(mFormat, value);
        break;
    case SpiItem::TYPE_FRAME_ERROR:
        shortTxt = "FE";
        longTxt = "Frame Error";
        break;
    }

}

/*!
    Paint signal data.
*/
void UiSpiAnalyzer::paintSignal(QPainter* painter, double from, double to,
                                int h, QString &shortTxt, QString &longTxt)
{

    int shortTextWidth = painter->fontMetrics().width(shortTxt);
    int longTextWidth = painter->fontMetrics().width(longTxt);

    if (to-from > 4) {
        painter->drawLine(from, 0, from+2, -h);
        painter->drawLine(from, 0, from+2, h);

        painter->drawLine(from+2, -h, to-2, -h);
        painter->drawLine(from+2, h, to-2, h);

        painter->drawLine(to, 0, to-2, -h);
        painter->drawLine(to, 0, to-2, h);
    }

    // drawing a vertical line when the allowed width is too small
    else {
        painter->drawLine(from, -h, from, h);
    }

    // only draw the text if it fits between 'from' and 'to'
    QRectF textRect(from+1, -h, (to-from), 2*h);
    if (longTextWidth < (to-from)) {
        painter->drawText(textRect, Qt::AlignCenter, longTxt);
    }
    else if (shortTextWidth < (to-from)) {
        painter->drawText(textRect, Qt::AlignCenter, shortTxt);
    }

}
