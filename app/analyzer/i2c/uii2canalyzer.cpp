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
#include "uii2canalyzer.h"

#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>

#include "uii2canalyzerconfig.h"
#include "common/configuration.h"
#include "capture/cursormanager.h"
#include "device/devicemanager.h"

/*!
    Counter used when creating the editable name.
*/
int UiI2CAnalyzer::i2cAnalyzerCounter = 0;

/*!
    Name of this analyzer.
*/
const QString UiI2CAnalyzer::signalName = "I2C Analyzer";

/*!
    \class UiI2CAnalyzer
    \brief This class is an I2C protocol analyzer.

    \ingroup Analyzer

    The class will analyze specified digital signals and visualize the
    interpretation as I2C protocol data.

*/


/*!
    Constructs the UiI2CAnalyzer with the given \a parent.
*/
UiI2CAnalyzer::UiI2CAnalyzer(QWidget *parent) :
    UiAnalyzer(parent)
{
    mSclSignalId = -1;
    mSdaSignalId = -1;
    mFormat = Types::DataFormatHex;

    mIdLbl->setText("I2C");
    mNameLbl->setText(QString("I2C %1").arg(i2cAnalyzerCounter++));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mSclLbl = new QLabel(this);
    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mSdaLbl = new QLabel(this);
    mSyncCursor = UiCursor::NoCursor;

    QPalette palette= mSclLbl->palette();
    palette.setColor(QPalette::Text, Qt::gray);
    mSclLbl->setPalette(palette);
    mSdaLbl->setPalette(palette);

    setFixedHeight(50);
}

/*!
    Set the SCL signal ID to \a sclSignalId.
*/
void UiI2CAnalyzer::setSclSignalId(int sclSignalId)
{
    mSclSignalId = sclSignalId;
    mSclLbl->setText(QString("SCL: D%1").arg(sclSignalId));
}

/*!
    Set the SDA signal ID to \a sdaSignalId.
*/
void UiI2CAnalyzer::setSdaSignalId(int sdaSignalId)
{
    mSdaSignalId = sdaSignalId;
    mSdaLbl->setText(QString("SDA: D%1").arg(sdaSignalId));
}

/*!
    Set the \a format to use when showing data.
*/
void UiI2CAnalyzer::setDataFormat(Types::DataFormat format)
{
    mFormat = format;
}

/*!
    \fn int UiI2CAnalyzer::sclSignalId()

    Returns the SCL signal ID.
*/

/*!
    \fn int UiI2CAnalyzer::sdaSignalId()

    Returns the SDA signal ID.
*/

/*!
    \fn Types::DataFormat UiI2CAnalyzer::dataFormat()

    Returns the format used to format I2C data.
*/

/*!
    \fn Types::DataFormat UiI2CAnalyzer::dataFormat()

    Returns the format used to format I2C data.
*/

/*!
    \fn void UiI2CAnalyzer::setSyncCursor(UiCursor::CursorId id)

    Set the cursor to use for synchronization.
*/

/*!
    \fn UiCursor::CursorId UiI2CAnalyzer::syncCursor()

    Returns the cursor used for synchronization.
*/


/*!
    Start to analyze the signal data.
*/
void UiI2CAnalyzer::analyze()
{
    /*
        Specification details

        1. SDA line can only change when SCL line is LOW for data
        2. START = HIGH to LOW on SDA line while SCL line is HIGH
        3. STOP  = LOW to HIGH on SDA line while SCL line is HIGH
        4. Each byte put on the SDA line must be 8 bits long
        5. Each byte is followed by an Acknowledge bit (ACK or NACK)
        6. ACK  = SDA line LOW during ninth clock pulse
        7. NACK = SDA line HIGH during ninth clock pulse
        8. 7-bit Address:
              7 bits + 1 bit which indicate R/W ( Read (1) or Write (0) )
        9. 10-bit Address:
              - The 7 first bits of the first byte are the combination 1111 0XX
                of which the last two bits are the two most-significant bits of
                the 10-bit address; the eight bit of the first byte is the R/W
                bit.
              - As always a byte is followed by an Acknowledge bit
              - The second byte is the 8 least-significant bits of the 10-bit
                address.

     */

    mI2cItems.clear();

    if (mSclSignalId == -1 || mSdaSignalId == -1) return;

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();

    QVector<int>* sclData = device->digitalData(mSclSignalId);
    QVector<int>* sdaData = device->digitalData(mSdaSignalId);

    if (sclData == NULL || sdaData == NULL) return;
    if (sclData->size() == 0 || sdaData->size() == 0
            || sclData->size() != sdaData->size()) return;

    int sda = 0;
    int scl = 0;
    int prevSda = sdaData->at(0);
    int prevScl = sclData->at(0);
    int sclHLIdx = -1;

    int data = 0;
    int dataBitCnt = 8;
    int startIdx = -1;

    bool findAddress = false;
    bool tenBit = false;
    int address = 0;
    int dir = 0;

    int numErrors = 0;
    bool errorFound = false;

    // start to analyze when start condition has been detected
    bool detectStart = true;
    bool startFound = false;

    int pos = 0;
    if (mSyncCursor != UiCursor::NoCursor) {
        double t = CursorManager::instance().cursorPosition(mSyncCursor);
        if (t > 0 && CursorManager::instance().isCursorOn(mSyncCursor)) {
            pos = device->usedSampleRate()*t;
        }
        if (pos >= sclData->size()) {
            pos = 0;
        }
    }

    for (int i = pos; i < sclData->size(); i++) {

        sda = sdaData->at(i);
        scl = sclData->at(i);

        //
        // HIGH -> LOW transition for SCL starts a bit transaction. A transition
        // on SDA is only allowed to occur when SCL is low (except for START/STOP)
        //
        if (prevScl > scl) {

            do {

                if (detectStart && !startFound) break;

                // record the HIGH-LOW transition index for SCL.
                sclHLIdx = i;

                // record start index for a data byte
                if (dataBitCnt == 8) {
                    startIdx = i;
                    break;
                }

                // nothing to do until dataBitCnt = 0
                if (dataBitCnt != 0) {
                    break;
                }

                // ---
                // at this point a complete byte has been received
                // ---

                if (findAddress) {
                    I2CItem::I2CType i2cType = I2CItem::I2C_7_ADDRESS_WRITE;

                    // 10-bit address: See Spec 9.
                    if ((data & 0xF8) == 0xF0) {
                        tenBit = true;
                        address = ((data & 0x06) << 7);

                        // direction (R/W) is defined by bit 0 in the first byte
                        dir = (data & 0x01);

                        if (dir) {
                            i2cType = I2CItem::I2C_10_ADDRESS_READ;
                        }
                        else {
                            i2cType = I2CItem::I2C_10_ADDRESS_WRITE;
                        }
                    }

                    // 7-bit address or second byte for 10-bit address
                    else {

                        if (tenBit) {
                            address |= (data & 0xFF);
                        }

                        // 7-bit address
                        else {

                            address = ((data >> 1) & 0xFF);

                            // direction (R/W) is defined by bit 0 in the address byte
                            dir = (data & 0x01);

                            if (dir) {
                                i2cType = I2CItem::I2C_7_ADDRESS_READ;
                            }
                            else {
                                i2cType = I2CItem::I2C_7_ADDRESS_WRITE;
                            }

                        }


                        I2CItem item(i2cType, address, startIdx, i);
                        mI2cItems.append(item);


                        tenBit = false;
                        findAddress = false;
                    }

                }

                // DATA
                else {

                    I2CItem item(I2CItem::I2C_DATA, data, startIdx, i);
                    mI2cItems.append(item);
                }



           } while (0);

        }


        //
        // LOW -> HIGH transition for SCL. SDA should remain stable when SCL
        // is high to detect a correct bit value.
        //
        else if (prevScl < scl){

            do {

                if (detectStart && !startFound) break;

                // SDA must not change when SCL is high (See Spec 1.)
                if (prevSda != sda) {

                    errorFound = true;
                    I2CItem item(I2CItem::I2C_ERROR, -1, i, -1);
                    mI2cItems.append(item);

                    numErrors++;
                    break;
                }

                // read data
                if (dataBitCnt > 0) {
                    // the left-shift is a bit index (0-7)
                    // -> decrease dataBitCnt before shifting
                    data |= (sda << (--dataBitCnt));
                }

                // check acknowledge bit
                else {

                    // ACK
                    if (sda == 0) {

                        // using the last HIGH-LOW transition for SCL as start index
                        I2CItem item(I2CItem::I2C_ACK, -1, sclHLIdx, -1);
                        mI2cItems.append(item);
                    }

                    // NACK
                    else {

                        // using the last HIGH-LOW transition for SCL as start index
                        I2CItem item(I2CItem::I2C_NACK, -1, sclHLIdx, -1);
                        mI2cItems.append(item);
                    }


                    // ready to read a new byte
                    dataBitCnt = 8;
                    data = 0;
                }



           } while (0);

        }


        //
        // Detect Start and Stop conditions. Transition while SCL is HIGH
        //
        if (!errorFound && scl == 1 && sda != prevSda) {

            do {

                // This should not occur while reading a data byte
                // If it does it is a bus error (See Spec 1.)
                if (dataBitCnt > 0 && dataBitCnt < 7) {

                    // reset reading data
                    dataBitCnt = 8;

                    I2CItem item(I2CItem::I2C_ERROR, -1, i, -1);
                    mI2cItems.append(item);

                    numErrors++;
                    break;
                }

                // HIGH -> LOW = Start
                if (prevSda > sda) {

                    I2CItem item(I2CItem::I2C_START, -1, i, -1);
                    mI2cItems.append(item);

                    findAddress = true;
                    startFound = true;
                }

                // LOW -> HIGH = Stop
                else {

                    if (!detectStart || (detectStart&&startFound)) {
                        I2CItem item(I2CItem::I2C_STOP, -1, i, -1);
                        mI2cItems.append(item);
                    }

                }

                data = 0;
                dataBitCnt = 8;

            } while (0);
        }


        prevSda = sda;
        prevScl = scl;
        errorFound = false;

        if (numErrors > MaxNumBusErrors) {
            qDebug() << "Too many bus errors "<<numErrors<<" > " << MaxNumBusErrors;
            break;
        }

    }

}

/*!
    Configure the analyzer.
*/
void UiI2CAnalyzer::configure(QWidget *parent)
{
    UiI2CAnalyzerConfig dialog(parent);
    dialog.setSclSignalId(mSclSignalId);
    dialog.setSdaSignalId(mSdaSignalId);
    dialog.setDataFormat(mFormat);
    dialog.setSyncCursor(mSyncCursor);
    dialog.exec();

    setSclSignalId(dialog.sclSignalId());
    setSdaSignalId(dialog.sdaSignalId());
    setDataFormat(dialog.dataFormat());
    setSyncCursor(dialog.syncCursor());

    analyze();
    update();
}

/*!
    Returns a string representation of this analyzer.
*/
QString UiI2CAnalyzer::toSettingsString() const
{
    // type;name;SCL;SDA;Format;Sync

    QString str;
    str.append(UiI2CAnalyzer::signalName);str.append(";");
    str.append(getName());str.append(";");
    str.append(QString("%1;").arg(mSclSignalId));
    str.append(QString("%1;").arg(mSdaSignalId));
    str.append(QString("%1;").arg(mFormat));
    str.append(QString("%1").arg(mSyncCursor));

    return str;
}

/*!
    Create an I2C analyzer from the string representation \a s.

    \sa toSettingsString
*/
UiI2CAnalyzer* UiI2CAnalyzer::fromSettingsString(const QString &s)
{
    UiI2CAnalyzer* analyzer = NULL;
    QString name;

    bool ok = false;

    do {
        // type;name;SCL,SDA,Format
        QStringList list = s.split(';');
        if (list.size() != 6) break;

        // --- type
        if (list.at(0) != UiI2CAnalyzer::signalName) break;

        // --- name
        name = list.at(1);
        if (name.isNull()) break;

        // --- signal ID for SCL
        int sclId = list.at(2).toInt(&ok);
        if (!ok) break;

        // --- signal ID for SDA
        int sdaId = list.at(3).toInt(&ok);
        if (!ok) break;

        // --- signal ID for SDA
        Types::DataFormat format;
        int f = list.at(4).toInt(&ok);
        if (!ok) break;

        if (f < 0 || f >= Types::DataFormatNum) break;
        format = (Types::DataFormat)f;

        // --- sync cursor
        int sc = list.at(5).toInt(&ok);
        if (sc < 0 || sc > UiCursor::NumCursors) break;
        UiCursor::CursorId syncCursor = (UiCursor::CursorId)sc;

        // Deallocation: Caller of this function is responsible for deallocation
        analyzer = new UiI2CAnalyzer();
        if (analyzer == NULL) break;

        analyzer->setSignalName(name);
        analyzer->setSclSignalId(sclId);
        analyzer->setSdaSignalId(sdaId);
        analyzer->setDataFormat(format);
        analyzer->setSyncCursor(syncCursor);

    } while (false);

    return analyzer;
}

/*!
    Paint event handler responsible for painting this widget.
*/
void UiI2CAnalyzer::paintEvent(QPaintEvent *event)
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

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
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

    for (int i = 0; i < mI2cItems.size(); i++) {
        I2CItem item = mI2cItems.at(i);

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

            if (i+1 < mI2cItems.size()) {

                // get position for the start of the next item
                double tmp = mTimeAxis->timeToPixelRelativeRef(
                            (double)mI2cItems.at(i+1).startIdx/sampleRate);


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
void UiI2CAnalyzer::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Convert I2C \a type and data \a value to string representation. A short
    and long representation is returned in \a shortTxt and \a longTxt.
*/
void UiI2CAnalyzer::typeAndValueAsString(I2CItem::I2CType type,
                                         int value,
                                         QString &shortTxt,
                                         QString &longTxt)
{
    QLatin1Char fillChar('0');

    switch(type) {
    case I2CItem::I2C_START:
        shortTxt = "S";
        longTxt = "Start";
        break;
    case I2CItem::I2C_STOP:
        shortTxt = "P";
        longTxt = "Stop";
        break;
    case I2CItem::I2C_ACK:
        shortTxt = "A";
        longTxt = "Ack";
        break;
    case I2CItem::I2C_NACK:
        shortTxt = "N";
        longTxt = "Nack";
        break;
    case I2CItem::I2C_DATA:
        shortTxt = formatValue(mFormat, value);
        longTxt = "Data = " + formatValue(mFormat, value);
        break;
    case I2CItem::I2C_7_ADDRESS_WRITE:
        shortTxt = QString("W:0x%1").arg(value, 2, 16, fillChar);
        longTxt = QString("Write to 0x%1").arg(value, 2, 16, fillChar);

        break;
    case I2CItem::I2C_7_ADDRESS_READ:
        shortTxt = QString("R:0x%1").arg(value, 2, 16, fillChar);
        longTxt = QString("Read from 0x%1").arg(value, 2, 16, fillChar);
        break;
    case I2CItem::I2C_10_ADDRESS_WRITE:
        shortTxt = QString("W:0x%1").arg(value, 2, 16, fillChar);
        longTxt = QString("Write to 0x%1").arg(value, 2, 16, fillChar);

        break;
    case I2CItem::I2C_10_ADDRESS_READ:
        shortTxt = QString("R:0x%1").arg(value, 2, 16, fillChar);
        longTxt = QString("Read from 0x%1").arg(value, 2, 16, fillChar);

        break;
    case I2CItem::I2C_ERROR:
        shortTxt = "Err";
        longTxt = "Bus Error";
        break;
    }

}

/*!
    Called when the info width has changed for this widget.
*/
void UiI2CAnalyzer::infoWidthChanged()
{
    doLayout();
}

/*!
    Position the child widgets.
*/
void UiI2CAnalyzer::doLayout()
{
    UiSimpleAbstractSignal::doLayout();

    QRect r = infoContentRect();
    int y = r.top();

    mIdLbl->move(r.left(), y);

    int x = mIdLbl->pos().x()+mIdLbl->width() + SignalIdMarginRight;
    mNameLbl->move(x, y);
    mEditName->move(x, y);

    mSclLbl->move(r.left(), r.bottom()-mSclLbl->height());
    mSdaLbl->move(mSclLbl->pos().x() + mSclLbl->width() + 5,
                  r.bottom()-mSdaLbl->height());
}

/*!
    Calculate and return the minimum width for this widget.
*/
int UiI2CAnalyzer::calcMinimumWidth()
{
    int w = mNameLbl->pos().x() + mNameLbl->minimumSizeHint().width();
    if (mEditName->isVisible()) {
        w = mEditName->pos().x() + mEditName->width();
    }

    int w2 = mSdaLbl->pos().x()+mSdaLbl->width();
    if (w2 > w) w = w2;

    return w+infoContentMargin().right();
}





