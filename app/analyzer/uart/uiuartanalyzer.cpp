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
#include "uiuartanalyzer.h"

#include <QDebug>

#include "uiuartanalyzerconfig.h"
#include "device/devicemanager.h"
#include "common/configuration.h"
#include "capture/cursormanager.h"

/*!
    Counter used when creating the editable name.
*/
int UiUartAnalyzer::uartAnalyzerCounter = 0;

/*!
    Name of this analyzer.
*/
const QString UiUartAnalyzer::name = "UART Analyzer";

/*!
    \class UiUartAnalyzer
    \brief This class is a UART protocol analyzer.

    \ingroup Analyzer

    The class will analyze specified digital signals and visualize the
    interpretation as UART protocol data.

*/


/*!
    Constructs the UiUartAnalyzer with the given \a parent.
*/
UiUartAnalyzer::UiUartAnalyzer(QWidget *parent) :
    UiAnalyzer(parent)
{
    mSignalId = -1;
    mBaudRate = 115200;
    mDataBits = 8;
    mStopBits = 1;
    mFormat = Types::DataFormatAscii;
    mParity = Types::ParityNone;
    mSyncCursor = UiCursor::NoCursor;

    mIdLbl->setText("UART");
    mNameLbl->setText(QString("UART %1").arg(uartAnalyzerCounter++));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mSignalLbl = new QLabel(this);

    QPalette palette= mSignalLbl->palette();
    palette.setColor(QPalette::Text, Qt::gray);
    mSignalLbl->setPalette(palette);

    setFixedHeight(50);
}

/*!
    Set the signal ID to \a id.
*/
void UiUartAnalyzer::setSignalId(int signalId)
{
    mSignalId = signalId;
    mSignalLbl->setText(QString("Signal: D%1").arg(signalId));
}


/*!
    \fn int UiUartAnalyzer::signalId() const

    Returns the signal ID.
*/


/*!
    Set data format to \a format.
*/
void UiUartAnalyzer::setDataFormat(Types::DataFormat format)
{
    mFormat = format;
}

/*!
    \fn Types::DataFormat UiUartAnalyzer::dataFormat() const

    Returns the data format.
*/

/*!
    \fn void UiUartAnalyzer::setBaudRate(int rate)

    Set the baud rate.
*/

/*!
    \fn int UiUartAnalyzer::baudRate() const

    Returns the baud rate.
*/

/*!
    \fn void UiUartAnalyzer::setStopBits(int bits)

    Set the number of stop bits.
*/

/*!
    \fn int UiUartAnalyzer::stopBits() const

    Returns the number of stop bits.
*/

/*!
    \fn void UiUartAnalyzer::setParity(Types::UartParity parity)

    Set the parity.
*/

/*!
    \fn Types::UartParity UiUartAnalyzer::parity() const

    Returns the parity.
*/

/*!
    \fn void UiUartAnalyzer::setDataBits(int bits)

    Set the number of data bits.
*/

/*!
    \fn int UiUartAnalyzer::dataBits() const

    Returns the number of data bits.
*/

/*!
    \fn void UiUartAnalyzer::setSyncCursor(UiCursor::CursorId id)

    Set the cursor to use for synchronization.
*/

/*!
    \fn UiCursor::CursorId UiUartAnalyzer::syncCursor() const

    Returns the cursor used for synchronization.
*/


/*!
    Start to analyze the signal data.
*/
void UiUartAnalyzer::analyze()
{
    mUartItems.clear();

    if (mSignalId == -1) return;

    CaptureDevice* device = DeviceManager::instance().activeDevice()->captureDevice();
    int sampleRate = device->usedSampleRate();
    QVector<int>* uartData = device->digitalData(mSignalId);

    if (uartData == NULL || uartData->size() == 0) return;

    int numSamplesPerBit = sampleRate / mBaudRate;
    // if there aren't enough samples per bit the decoding isn't reliable
    if (numSamplesPerBit < 3) return;

    int startIdx = 0;
    int value = 0;
    int numDataBits = 0;
    int numStopBits = 0;
    int pos = 0;
    int onesInBit = 0;
    int onesInValue = 0;
    int bitValue = 0;
    int bitStart = 0;


    bool startFound = false;
    bool findTransition = true;
    bool parityError = false;
    bool done = false;

    if (mSyncCursor != UiCursor::NoCursor) {
        double t = CursorManager::instance().cursorPosition(mSyncCursor);
        if (t > 0 && CursorManager::instance().isCursorOn(mSyncCursor)) {
            pos = sampleRate*t;
        }
        if (pos >= uartData->size()) {
            pos = 0;
        }
    }

    UartState state = STATE_START;

    int prev = uartData->at(pos);

    while(!done) {
        if (pos + numSamplesPerBit >= uartData->size()) break;

        if (findTransition) {
            if (uartData->at(pos) != prev) {
               findTransition = false;
            }
            else {
                prev = uartData->at(pos);
                pos++;

                continue;
            }
        }

        // check value of the bit
        onesInBit = 0;
        bitStart = pos;

        for(int i = 0; i < numSamplesPerBit; i++) {

            if (pos > 0 && uartData->at(pos-1) != uartData->at(pos)) {

                // resyncing if a transition occurs when at least half
                // the bit time has elapsed
                if (i >= numSamplesPerBit/2) {
                    break;
                }
            }

            if (uartData->at(pos++) == 1) {
                onesInBit++;
            }
        }
        // value determined by state during at least half the bit time
        bitValue = (((double)onesInBit/numSamplesPerBit) >= 0.5) ? 1 : 0;

        switch(state) {

        case STATE_START:
            if (bitValue == 0) {
                startFound = true;
                startIdx = bitStart;
                numDataBits = 0;
                numStopBits = 0;
                onesInValue = 0;
                value = 0;
                parityError = false;

                state = STATE_DATA;
            }

            // it was not a start bit
            else {

                // restart if the start bit has never been seen
                if (!startFound) {
                    findTransition = true;
                }

                // frame error if start bit has been seen at least once
                else {
                    UartItem item(UartItem::TYPE_FRAME_ERROR, 0, bitStart, -1);
                    mUartItems.append(item);
                    done = true;
                }

            }
            break;


        case STATE_DATA:
            // TODO: also support MSB first
            value |= (bitValue << numDataBits);
            numDataBits++;

            if (bitValue == 1) {
                onesInValue++;
            }

            if (numDataBits == mDataBits) {
                if (mParity != Types::ParityNone) {
                    state = STATE_PARITY;
                }
                else {
                    state = STATE_STOP;
                }
            }
            break;
        case STATE_PARITY:

            parityError = false;
            switch(mParity) {
            case Types::ParityNone:
                break;
            case Types::ParityOdd:
                if ( (((onesInValue%2) == 0) && bitValue == 0) ||
                     (((onesInValue%2) != 0 && bitValue == 1)))
                {
                    parityError = true;
                }

                break;
            case Types::ParityEven:

                if ( (((onesInValue%2) != 0) && bitValue == 0) ||
                     (((onesInValue%2) == 0 && bitValue == 1)))
                {
                    parityError = true;
                }

                break;
            case Types::ParityMark:
                parityError = (bitValue == 0);
                break;
            case Types::ParitySpace:
                parityError = (bitValue == 1);
                break;
            default:
                break;
            }

            state = STATE_STOP;

            break;
        case STATE_STOP:
            if (bitValue == 1) {
                numStopBits++;

                if (numStopBits == mStopBits) {

                    if (!parityError) {
                        UartItem item(UartItem::TYPE_DATA, value, startIdx, pos);
                        mUartItems.append(item);
                    }
                    else {
                        UartItem item(UartItem::TYPE_PARITY_ERROR, 0, startIdx, pos);
                        mUartItems.append(item);
                    }

                    state = STATE_START;
                    prev = uartData->at(pos-1);

                    if (prev == 1) {
                        // resync by finding transition
                        findTransition = true;
                    }


                }
            }

            // no stop bit -> frame error
            else {
                UartItem item(UartItem::TYPE_FRAME_ERROR, 0, startIdx, -1);
                mUartItems.append(item);
                done = true;
            }
            break;
        }

    }

}

/*!
    Configure the analyzer.
*/
void UiUartAnalyzer::configure(QWidget *parent)
{
    UiUartAnalyzerConfig dialog(parent);
    dialog.setSignalId(mSignalId);
    dialog.setDataFormat(mFormat);
    dialog.setBaudRate(mBaudRate);
    dialog.setParity(mParity);
    dialog.setStopBits(mStopBits);
    dialog.setDataBits(mDataBits);
    dialog.setSyncCursor(mSyncCursor);
    dialog.exec();

    setSignalId(dialog.signalId());
    setDataFormat(dialog.dataFormat());
    setBaudRate(dialog.baudRate());
    setParity(dialog.parity());
    setStopBits(dialog.stopBits());
    setDataBits(dialog.dataBits());
    setSyncCursor(dialog.syncCursor());


    analyze();
    update();
}

/*!
    Returns a string representation of this analyzer.
*/
QString UiUartAnalyzer::toSettingsString() const
{
    // type;name;Signal;Format;Baud;DataBits;StopBits;Parity;Sync

    QString str;
    str.append(UiUartAnalyzer::name);str.append(";");
    str.append(getName());str.append(";");
    str.append(QString("%1;").arg(signalId()));
    str.append(QString("%1;").arg(dataFormat()));
    str.append(QString("%1;").arg(baudRate()));
    str.append(QString("%1;").arg(dataBits()));
    str.append(QString("%1;").arg(stopBits()));
    str.append(QString("%1;").arg(parity()));
    str.append(QString("%1").arg(syncCursor()));

    return str;
}

/*!
    Create a UART analyzer from the string representation \a s.

    \sa toSettingsString
*/
UiUartAnalyzer* UiUartAnalyzer::fromSettingsString(const QString &s)
{
    UiUartAnalyzer* analyzer = NULL;
    QString name;

    bool ok = false;

    do {
        // type;name;Signal;Format;Baud;DataBits;StopBits;Parity;Sync
        QStringList list = s.split(';');
        if (list.size() != 9) break;

        // --- type
        if (list.at(0) != UiUartAnalyzer::name) break;

        // --- name
        name = list.at(1);
        if (name.isNull()) break;

        // --- signal ID
        int signalId = list.at(2).toInt(&ok);
        if (!ok) break;

        // --- data format
        Types::DataFormat format;
        int f = list.at(3).toInt(&ok);
        if (!ok) break;
        if (f < 0 || f >= Types::DataFormatNum) break;
        format = (Types::DataFormat)f;

        // --- baud rate
        int baudRate = list.at(4).toInt(&ok);
        if (!ok) break;

        // --- data bits
        int dataBits = list.at(5).toInt(&ok);
        if (!ok) break;

        // --- stop bits
        int stopBits = list.at(6).toInt(&ok);
        if (!ok) break;

        // --- parity
        int p = list.at(7).toInt(&ok);
        if (!ok) break;
        if (p < 0 || p >= Types::ParityNum) break;
        Types::UartParity parity = (Types::UartParity)p;

        // --- sync cursor
        int sc = list.at(8).toInt(&ok);
        if (sc < 0 || sc > UiCursor::NumCursors) break;
        UiCursor::CursorId syncCursor = (UiCursor::CursorId)sc;

        // Deallocation: The caller of this function is responsible for
        //               deallocation
        analyzer = new UiUartAnalyzer();
        if (analyzer == NULL) break;

        analyzer->setSignalName(name);
        analyzer->setSignalId(signalId);
        analyzer->setDataFormat(format);
        analyzer->setBaudRate(baudRate);
        analyzer->setDataBits(dataBits);
        analyzer->setStopBits(stopBits);
        analyzer->setParity(parity);
        analyzer->setSyncCursor(syncCursor);

    } while (false);

    return analyzer;
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiUartAnalyzer::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);

    int textMargin = 3;

    // -----------------
    // draw background
    // -----------------
    paintBackground(&painter);


    painter.setClipRect(plotX(), 0, width()-infoWidth(), height());
    painter.translate(0, height()/2);

    CaptureDevice* device = DeviceManager::instance().activeDevice()->captureDevice();
    int sampleRate = device->usedSampleRate();


    double from = 0;
    double to = 0;
    int fromIdx = 0;
    int toIdx = 0;

    int h = height()/4;

    QString shortTxt;
    QString longTxt;

    QPen pen = painter.pen();
    pen.setColor(Configuration::instance().analyzerColor());
    painter.setPen(pen);

    for (int i = 0; i < mUartItems.size(); i++) {
        UartItem item = mUartItems.at(i);

        fromIdx = item.startIdx;
        toIdx = item.stopIdx;


        typeAndValueAsString(item.type, item.value, shortTxt, longTxt);

        int shortTextWidth = painter.fontMetrics().width(shortTxt);
        int longTextWidth = painter.fontMetrics().width(longTxt);


        from = mTimeAxis->timeToPixelRelativeRef((double)fromIdx/sampleRate);

        // no need to draw when signal is out of plot area
        if (from > width()) break;

        if (toIdx != -1) {
            to = mTimeAxis->timeToPixelRelativeRef((double)toIdx/sampleRate);
        }
        else  {

            // see if the long text version fits
            to = from + longTextWidth+textMargin*2;

            if (i+1 < mUartItems.size()) {

                // get position for the start of the next item
                double tmp = mTimeAxis->timeToPixelRelativeRef(
                            (double)mUartItems.at(i+1).startIdx/sampleRate);


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


        if (to-from > 4) {
            painter.drawLine(from, 0, from+2, -h);
            painter.drawLine(from, 0, from+2, h);

            painter.drawLine(from+2, -h, to-2, -h);
            painter.drawLine(from+2, h, to-2, h);

            painter.drawLine(to, 0, to-2, -h);
            painter.drawLine(to, 0, to-2, h);
        }

        // drawing a vertical line when the allowed width is too small
        else {
            painter.drawLine(from, -h, from, h);
        }

        // only draw the text if it fits between 'from' and 'to'
        QRectF textRect(from+1, -h, (to-from), 2*h);
        if (longTextWidth < (to-from)) {
            painter.drawText(textRect, Qt::AlignCenter, longTxt);
        }
        else if (shortTextWidth < (to-from)) {
            painter.drawText(textRect, Qt::AlignCenter, shortTxt);
        }

    }

}

/*!
    Event handler called when this widget is being shown
*/
void UiUartAnalyzer::showEvent(QShowEvent* event)
{
    (void) event;
    doLayout();
    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Called when the info width has changed for this widget.
*/
void UiUartAnalyzer::infoWidthChanged()
{
    doLayout();
}

/*!
    Position the child widgets.
*/
void UiUartAnalyzer::doLayout()
{
    UiSimpleAbstractSignal::doLayout();

    QRect r = infoContentRect();
    int y = r.top();

    mIdLbl->move(r.left(), y);

    int x = mIdLbl->pos().x()+mIdLbl->width() + SignalIdMarginRight;
    mNameLbl->move(x, y);
    mEditName->move(x, y);

    mSignalLbl->move(r.left(), r.bottom()-mSignalLbl->height());

}

/*!
    Calculate and return the minimum width for this widget.
*/
int UiUartAnalyzer::calcMinimumWidth()
{
    int w = mNameLbl->pos().x() + mNameLbl->minimumSizeHint().width();
    if (mEditName->isVisible()) {
        w = mEditName->pos().x() + mEditName->width();
    }

    int w2 = mSignalLbl->pos().x()+mSignalLbl->width();
    if (w2 > w) w = w2;

    return w+infoContentMargin().right();
}

/*!
    Convert UART \a type and data \a value to string representation. A short
    and long representation is returned in \a shortTxt and \a longTxt.
*/
void UiUartAnalyzer::typeAndValueAsString(UartItem::ItemType type,
                                          int value,
                                          QString &shortTxt,
                                          QString &longTxt)
{


    switch(type) {
    case UartItem::TYPE_DATA:
        shortTxt = formatValue(mFormat, value);
        longTxt = formatValue(mFormat, value);
        break;
    case UartItem::TYPE_PARITY_ERROR:
        shortTxt = "PE";
        longTxt = "Parity Error";
        break;
    case UartItem::TYPE_FRAME_ERROR:
        shortTxt = "FE";
        longTxt = "Frame Error";
        break;
    }

}
