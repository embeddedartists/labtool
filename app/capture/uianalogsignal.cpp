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
#include "uianalogsignal.h"

#include <QDebug>
#include <QPainter>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QButtonGroup>

#include "common/configuration.h"
#include "uianalogtrigger.h"
#include "device/devicemanager.h"

#include "uilistspinbox.h"


const double UiAnalogSignal::MaxVPerDiv = 4.99;
const double UiAnalogSignal::MinVPerDiv = 0.11;


// ###########################################################################
//
// ###########################################################################


/*!
    \class UiAnalogSignalPrivate
    \brief Internal class used to keep track of each analog signal in the
    analog signal widget.

    \ingroup Capture

    \privatesection

*/

class UiAnalogSignalPrivate
{
public:
    UiAnalogSignalPrivate();
    ~UiAnalogSignalPrivate();

    /*! Analog signal */
    AnalogSignal* mSignal;

    /*! Color widget */
    QLabel* mColorLbl;
    /*! ID widget */
    QLabel* mIdLbl;
    /*! Name widget */
    QLabel* mName;
    /*! Name editor widget */
    QLineEdit* mEditName;

    /*! Disable/close button */
    QPushButton* mDisableBtn;
    /*! Spinbox used for volts per division */
    UiListSpinBox* mVPerDivBox;
    /*! Widget used for the analog trigger */
    UiAnalogTrigger* mAnalogTrigger;

    /*! DC coupling radio button */
    QRadioButton* mDcBtn;
    /*! AC coupling radio button */
    QRadioButton* mAcBtn;
    /*! Groups coupling buttons */
    QButtonGroup* mCouplingGroup;

    /*! Holds the vertical position for 'ground' for this signal */
    double mGndPos;
    /*! The valid geometry of this signal */
    QRect geometry;

    void setup(AnalogSignal *signal, UiAnalogSignal *parent);
    void setGeometry(int x, int y, int w, int h);
    double calcPeakToPeak();

    bool hasNameBeenClicked(int x, int y);
    void enableNameEditing(bool enable);
    int minimumWidth();
    int minimumHeight();

    void paintInfo(QPainter* painter, QColor color);
};

/*!
    Constructs an UiAnalogSignalPrivate instance.
*/
UiAnalogSignalPrivate::UiAnalogSignalPrivate()
{
    mSignal = NULL;
    mColorLbl  = NULL;
    mIdLbl  = NULL;
    mName   = NULL;
    mEditName   = NULL;
    mDisableBtn = NULL;
    mVPerDivBox = NULL;
    mAnalogTrigger = NULL;
    mDcBtn         = NULL;
    mAcBtn         = NULL;
    mCouplingGroup = NULL;
    mGndPos        = 0;
}

/*!
    Closes and deletes all UI elements related to the analog signal
*/
UiAnalogSignalPrivate::~UiAnalogSignalPrivate()
{
    mColorLbl->close();
    delete mColorLbl;

    mIdLbl->close();
    delete mIdLbl;

    mName->close();
    delete mName;

    mEditName->close();
    delete mEditName;

    mDisableBtn->close();
    delete mDisableBtn;

    mVPerDivBox->close();
    delete mVPerDivBox;

    mAnalogTrigger->close();
    delete mAnalogTrigger;

    mDcBtn->close();
    delete mDcBtn;

    mAcBtn->close();
    delete mAcBtn;

    delete mCouplingGroup;
}

/*!
    Initialize and setup UI elements related to the analog signal \a signal.
    The parameter \a parent is used as parent for the UI elements.
*/
void UiAnalogSignalPrivate::setup(AnalogSignal* signal, UiAnalogSignal* parent)
{
    mSignal = signal;

    // Deallocation: Destructor
    mColorLbl = new QLabel(parent);
    mColorLbl->setText("    ");
    QString color = Configuration::instance().analogInCableColor(signal->id()).name();
    mColorLbl->setStyleSheet(QString("QLabel { background-color : %1; }").arg(color));
    mColorLbl->show();

    // Deallocation: Destructor
    mIdLbl = new QLabel(parent);
    mIdLbl->setText(QString("A%1").arg(signal->id()));
    mIdLbl->show();

    // Deallocation: Destructor
    mName = new QLabel(parent);
    mName->setText(signal->name());
    mName->show();

    // edit field for signal name

    // Deallocation: Destructor
    mEditName = new QLineEdit(parent);
    mEditName->hide();
    parent->connect(mEditName, SIGNAL(editingFinished()),
                    parent, SLOT(nameEdited()));

    // Deallocation: Destructor
    mDisableBtn = new QPushButton(parent);
    mDisableBtn->setIcon(QIcon(":/resources/8_close.png"));
    mDisableBtn->setFlat(true);
    mDisableBtn->resize(12, 12); //slightly bigger than the 8x8 icon
    mDisableBtn->show();
    parent->connect(mDisableBtn, SIGNAL(clicked()),
                    parent, SLOT(disableSignal()));

    // Deallocation: Destructor
    mVPerDivBox = new UiListSpinBox(parent);
    mVPerDivBox->setSuffix(" V/div");
    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    QList<double> vPerDivList = device->supportedVPerDiv();

    mVPerDivBox->setSupportedValues(vPerDivList);
    mVPerDivBox->setValue(signal->vPerDiv());

    parent->connect(mVPerDivBox, SIGNAL(valueChanged(double)),
                    parent, SLOT(changeVPerDiv(double)));
    mVPerDivBox->show();

    // Deallocation: Destructor
    mAnalogTrigger = new UiAnalogTrigger(parent);
    mAnalogTrigger->setLevel(mSignal->triggerLevel());
    mAnalogTrigger->setState(mSignal->triggerState());
    mAnalogTrigger->setVPerDiv(signal->vPerDiv());
    mAnalogTrigger->show();
    parent->connect(mAnalogTrigger, SIGNAL(triggerChanged()),
                    parent, SLOT(changeTriggers()));
    parent->connect(mAnalogTrigger, SIGNAL(levelChanged()),
                    parent, SLOT(handleTriggerLevelChanged()));

    // Deallocation: Destructor
    mDcBtn = new QRadioButton("DC", parent);
    mDcBtn->setToolTip(parent->tr("DC coupling"));
    if (mSignal->coupling() == AnalogSignal::CouplingDc) {
        mDcBtn->setChecked(true);
    }
    mDcBtn->show();

    // Deallocation: Destructor
    mAcBtn = new QRadioButton("AC", parent);
    mAcBtn->setToolTip(parent->tr("AC coupling"));
    if (mSignal->coupling() == AnalogSignal::CouplingAc) {
        mAcBtn->setChecked(true);
    }
    mAcBtn->show();

    // Deallocation: Destructor
    mCouplingGroup = new QButtonGroup(parent);
    mCouplingGroup->setExclusive(true);
    mCouplingGroup->addButton(mDcBtn);
    mCouplingGroup->addButton(mAcBtn);
    parent->connect(mCouplingGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            parent, SLOT(handleCouplingChanged(QAbstractButton*)));

    mGndPos = -1;
}

/*!
    Calculate peak-to-peak (voltage) for this analog signal
*/
double UiAnalogSignalPrivate::calcPeakToPeak()
{
    double result = 0;

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    QVector<double>* data = device->analogData(mSignal->id());

    if (data != NULL) {
        double min = 123456789;
        double max = 0;
        for (int i = 0; i < data->size(); i++) {
            if (data->at(i) > max) {
                max = data->at(i);
            }

            if (data->at(i) < min) {
                min = data->at(i);
            }
        }

        result = max - min;
    }

    return result;
}

/*!
    Set the geometry for this analog signal to \a x, \a y, \a w,
    and \a h
*/
void UiAnalogSignalPrivate::setGeometry(int x, int y, int w, int h)
{
    if (h != geometry.height()) {
        mGndPos = -1;
    }

    geometry = QRect(x, y, w, h);

    int wx = x;
    int wy = y;

    mDisableBtn->move(x+w-mDisableBtn->width(), wy);
    wy = wy + mDisableBtn->height();

    mColorLbl->move(wx, wy);
    wx = mColorLbl->pos().x()+mColorLbl->width()
            + UiAnalogSignal::SignalIdMarginRight;

    mIdLbl->move(wx, wy);

    wx = mIdLbl->pos().x()+mIdLbl->width()
            + UiAnalogSignal::SignalIdMarginRight;
    mName->move(wx, wy);
    mEditName->move(wx, wy);

    mAnalogTrigger->resize(mAnalogTrigger->width(), h-mDisableBtn->height()-4);
    wy = wy+(h-mDisableBtn->height())/2 - mAnalogTrigger->height()/2;
    mAnalogTrigger->move(x+w-mAnalogTrigger->width(), wy);

    wy = mName->pos().y() + mName->height() + 7;
    if (mEditName->isVisible()) {
        wy = mEditName->pos().y() + mEditName->height() + 7;
    }

    wx = w/2-mVPerDivBox->width()/2;
    mVPerDivBox->move(wx, wy);

    // signal color is painted below mVPerDivBox (see paintInfo)
    wy = mVPerDivBox->pos().y()+mVPerDivBox->height()+3+5+5;

    mDcBtn->move(w/2-mDcBtn->width(), wy);
    mAcBtn->move(w/2, wy);

    if (mGndPos == -1) {
        mGndPos = y + h/2;
    }
}

/*!
    Paint the info part of the analog signal using \a painter and \a color.
*/
void UiAnalogSignalPrivate::paintInfo(QPainter* painter, QColor color)
{
    QPen pen = painter->pen();
    pen.setColor(color);
    painter->setPen(pen);
    painter->setBrush(color);

    int w = mVPerDivBox->width();
    int y = mVPerDivBox->pos().y()+mVPerDivBox->height()+3;
    QRect rect(geometry.width()/2-w/2, y, w, 5);
    painter->drawRoundRect(rect, 10, 10);
}

/*!
    Returns true if the name widget is within the specified coordinates
    \a x and \a y.
*/
bool UiAnalogSignalPrivate::hasNameBeenClicked(int x, int y)
{
    return (x > mName->pos().x()
            && x < mName->pos().x()+mName->width()
            && y > mName->pos().y()
            && y < mName->pos().y()+mName->height());
}

/*!
    Enable/disable name editing according to \a enable.
*/
void UiAnalogSignalPrivate::enableNameEditing(bool enable)
{
    if (enable) {
        mName->hide();
        mEditName->setText(mName->text());
        mEditName->show();
        mEditName->setFocus();
    } else {
        mEditName->hide();
        mName->show();
    }
}

/*!
    Get the minimum width of this analog signal.
*/
int UiAnalogSignalPrivate::minimumWidth()
{
    int w = 0;

    // check name/edit fiels
    w = mName->pos().x() + mName->minimumSizeHint().width();
    if (mEditName->isVisible()) {
        w = mEditName->pos().x() + mEditName->width();
    }

    int w2 = mIdLbl->pos().x()+mIdLbl->width()+mVPerDivBox->width();
    if (w2 > w) w = w2;

    w += 15;
    w += mAnalogTrigger->width();

    return w;
}

/*!
    Get the minimum height of this analog signal.
*/
int UiAnalogSignalPrivate::minimumHeight()
{
    int h = mDisableBtn->height() + mAnalogTrigger->minimumHeight() + 10;
    h += (5 + mDcBtn->height());

    return h;
}


// ###########################################################################
//
// ###########################################################################


/*!
    \class UiAnalogSignal
    \brief UI widget that represents the analog signals.

    \ingroup Capture

    This widget is responsible for all analog signals, that is, each analog
    signal is painted within the same widget. The reason is to get a similar
    behaviour as with oscilloscopes where the signals can moved relative to
    each other.
*/

/*!
    \enum UiAnalogSignal::Constants

    This enum describes constant integer values used by this widget.

    \var UiAnalogSignal::Constants UiAnalogSignal::MaxNumSignals
    The maximum number of signals that can be handled by this widget.

*/


/*!
    Constructs an UiAnalogSignal with the given \a parent.
*/
UiAnalogSignal::UiAnalogSignal(QWidget *parent) :
    UiAbstractSignal(parent)
{
    mDragging = false;
    mDragStart = 0;
    mDragSignal = 0;
    mMouseOverXPos = 0;
    mMouseOverValid = false;

    setMouseTracking(true);
}

/*!
    Deletes signals added to this widget.
*/
UiAnalogSignal::~UiAnalogSignal()
{
    qDeleteAll(mSignals);
}

/*!
    Add the analog signal \a signal to this widget.
*/
void UiAnalogSignal::addSignal(AnalogSignal* signal)
{

    // Deallocation: By disableSignal or Destructor
    UiAnalogSignalPrivate* p = new UiAnalogSignalPrivate();
    p->setup(signal, this);
    mSignals.append(p);

    setMinimumInfoWidth(calcMinimumWidth());
    doLayout();

    update();
}

/*!
    Get a list with the analog signals added to this widget.
*/
QList<AnalogSignal*> UiAnalogSignal::addedSignals()
{
    QList<AnalogSignal*> l;

    foreach(UiAnalogSignalPrivate* p, mSignals) {
        l.append(p->mSignal);
    }

    return l;
}

/*!
    Clear, that is, set triggers to none for all analog signals.
*/
void UiAnalogSignal::clearTriggers()
{
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);
        p->mAnalogTrigger->setState(AnalogSignal::AnalogTriggerNone);
        p->mSignal->setTriggerState(AnalogSignal::AnalogTriggerNone);
    }
}

/*!
    \fn void UiAnalogSignal::measurmentChanged(QList<double>level, QList<double>pk, bool active)

    This signal is emitted when a measurement related to an analog signal
    has changed.
*/

/*!
    \fn void UiAnalogSignal::triggerSet()

    This signal is emitted when a trigger has been set for an analog signal.
*/



/*!
    Paint event handler responsible for painting this widget.
*/
void UiAnalogSignal::paintEvent(QPaintEvent *event)
{
    (void)event;
    QPainter painter(this);
#if QT_VERSION >= 0x050000
    painter.setRenderHint(QPainter::Qt4CompatiblePainting);
#endif


    // -----------------
    // draw background
    // -----------------
    paintBackground(&painter);

    // -----------------
    // draw Div lines
    // -----------------
    paintDivLines(&painter);


    if (mTimeAxis != NULL) {

        // -----------------
        // paint signals
        // -----------------
        paintSignals(&painter);


        // -----------------
        // paint signal value at mouse over
        // -----------------
        if (mMouseOverValid) {
            double mouseOverTime = mTimeAxis->pixelToTimeRelativeRef(mMouseOverXPos);

            paintSignalValue(&painter, mouseOverTime);
        }

        // -----------------
        // paint trigger level
        // -----------------
        paintTriggerLevel(&painter);


    }
}

/*!
    The mouse press event handler is called when a mouse button is pressed.
    This implementation will move an individual signal within the plot or
    enable editing of the signal name.
*/
void UiAnalogSignal::mousePressEvent(QMouseEvent* event)
{

    if (event->button() == Qt::LeftButton) {

        for(int i = 0; i < mSignals.size(); i++) {
            UiAnalogSignalPrivate* p = mSignals.at(i);

            if (p->hasNameBeenClicked(event->pos().x(), event->pos().y()))
            {

                // enable editing the signal name
                if (p->mName->isVisible()) {
                    p->enableNameEditing(true);

                    setMinimumInfoWidth(calcMinimumWidth());
                }
            }
        }


        if (event->pos().x() > infoWidth())
        {
            mDragSignal = findSignal(QPoint(event->pos().x(),
                                            event->pos().y()));

            if (mDragSignal != NULL) {
                mDragging = true;
                mDragStart = event->pos().y();
            }
        }

    }

    QWidget::mousePressEvent(event);
}

/*!
    The mouse release event handler is called when a mouse button is released.
    This implementation will stop moving a signal.
*/
void UiAnalogSignal::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        mDragging = false;
    }

    QWidget::mouseReleaseEvent(event);
}

/*!
    The mouse move event handler is called when a mouse is moved.
*/
void UiAnalogSignal::mouseMoveEvent(QMouseEvent *event)
{

    if (mDragging && mDragSignal != NULL) {
        mMouseOverValid = false;

        int yPos = event->pos().y();
        if (yPos < 5) yPos = 5;
        if (yPos > height()-5) yPos = height()-5;

        double diff = mDragStart - yPos;
        mDragStart = yPos;

        mDragSignal->mGndPos -= diff;

        update();
    }
    else {

        if (event->pos().x() >= infoWidth()) {

            mMouseOverXPos = event->pos().x();
            mMouseOverValid = true;            

            update();
        }
        else {
            mMouseOverValid = false;
        }

    }

    QWidget::mouseMoveEvent(event);
}

/*!
    The show event handler is called when this widget is being made visible.
*/
void UiAnalogSignal::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
}

/*!
    Event handler that is called when the mouse cursor leaves this widget
*/
void UiAnalogSignal::leaveEvent(QEvent* event)
{
    UiAbstractSignal::leaveEvent(event);
    mMouseOverValid = false;
    emit measurmentChanged(QList<double>(), QList<double>(), false);
}

/*!
    Called when the signal name has been edited.
*/
void UiAnalogSignal::nameEdited(void)
{

    QObject* o = QObject::sender();
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* s = mSignals.at(i);

        if (s->mEditName == o) {
            QString n = s->mEditName->text();
            if (n.isNull() || n.size() == 0) {
                n = s->mName->text();
            }

            setName(n, s);

            break;
        }
    }
}

/*!
    Called when the volts per division value has been changed.
*/
void UiAnalogSignal::changeVPerDiv(double v)
{

    QObject* o = QObject::sender();
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (p->mVPerDivBox == o) {

            p->mSignal->setVPerDiv(v);
            p->mAnalogTrigger->setVPerDiv(v);

            doLayout();
            update();

            break;
        }
    }

}

/*!
    Called when the trigger has been changed.
*/
void UiAnalogSignal::changeTriggers()
{

    QObject* o = QObject::sender();
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (p->mAnalogTrigger == o) {
            p->mSignal->setTriggerState(p->mAnalogTrigger->state());
        }

        // only one analog signal may have a trigger enabled
        if (p->mAnalogTrigger != o) {
            p->mAnalogTrigger->setState(AnalogSignal::AnalogTriggerNone);
            p->mSignal->setTriggerState(AnalogSignal::AnalogTriggerNone);
        }
    }


    emit triggerSet();

    update();
}

/*!
    Called when the trigger level has been changed.
*/
void UiAnalogSignal::handleTriggerLevelChanged()
{

    QObject* o = QObject::sender();
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (p->mAnalogTrigger == o) {
            p->mSignal->setTriggerLevel(p->mAnalogTrigger->level());
            break;
        }

    }

    update();
}

/*!
    Called when the coupling setting has been changed.
*/
void UiAnalogSignal::handleCouplingChanged(QAbstractButton* btn)
{
    QObject* o = QObject::sender();
    foreach(UiAnalogSignalPrivate* p, mSignals) {

        if (p->mCouplingGroup == o) {

            if (p->mDcBtn == btn) {
                p->mSignal->setCoupling(AnalogSignal::CouplingDc);
            }
            else {
                p->mSignal->setCoupling(AnalogSignal::CouplingAc);
            }

            break;
        }

    }
}

/*!
    Called when the user clicks the close/disable button.
*/
void UiAnalogSignal::disableSignal()
{

    QObject* o = QObject::sender();
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (p->mDisableBtn == o) {
            disableSignal(i);
            break;
        }
    }

    if (mSignals.size() == 0) {
        closeSignal();
    }
}

/*!
    Close/disable an analog signal at index \a idx
*/
void UiAnalogSignal::disableSignal(int idx)
{
    UiAnalogSignalPrivate* p = mSignals.at(idx);
    mSignals.removeAt(idx);

    CaptureDevice* dev = DeviceManager::instance().activeDevice()
            ->captureDevice();
    if (dev != NULL) {
        dev->removeAnalogSignal(p->mSignal);
    }

    delete p;

    doLayout();
    update();
}

/*!
    Set the name of the analog signal to \a name.
*/
void UiAnalogSignal::setName(QString &name, UiAnalogSignalPrivate* signal)
{
    signal->mName->setText(name);
    signal->mSignal->setName(name);

    // if I don't call hide followed by show the text isn'a always showed
    // it is not enough to call update or repaint???
    signal->mName->hide();
    signal->mName->show();
    signal->enableNameEditing(false);

    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Return the minimum width for this widget
*/
int UiAnalogSignal::calcMinimumWidth()
{
    int w = 0;

    for (int i = 0; i < mSignals.size(); i++) {

        UiAnalogSignalPrivate* p = mSignals.at(i);
        if (p->minimumWidth() > w) {
            w = p->minimumWidth();
        }
    }

    return InfoMarginLeft+w+InfoMarginRight;
}


/*!
    Find the pixel point where a vertical line at \a time intersects
    with \a signal. The out parameter \a intersect will contain the point. If
    no signal data was found the x part of the point will be set to -1.
*/
void UiAnalogSignal::findIntersect(UiAnalogSignalPrivate* signal, double time,
                                   QPointF* intersect)
{

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    int rate = device->usedSampleRate();

    double t = time*rate;
    int idx = (int)t;
    QLineF sigPart;


    intersect->setX(-1);

    // t=0 is the starting point for all samples
    if (t<0) return;

    // 1. Find the two closest samples from a signal based on the time axis
    // 2. Find the intersect between a vertical line and the signal

    QVector<double>* data = device->analogData(signal->mSignal->id());

    if (data != NULL && idx>= 0 && idx+1 < data->size()) {
        sigPart.setLine(idx, data->at(idx),
                        idx+1, data->at(idx+1));
        sigPart.intersect(QLineF(t, 0, t, 5), intersect);

        // convert x back to absolute time
        intersect->setX(intersect->x()/rate);
    }

}

/*!
    Find the signal closest to the pixel point \a pxPoint. NULL is returned
    if a signal wasn't found.
*/
UiAnalogSignalPrivate* UiAnalogSignal::findSignal(QPoint pxPoint)
{
    qreal ix = -1;

    if (mSignals.size() == 0) return NULL;

    QVector<QPointF> intersect(mSignals.size());

    double time = mTimeAxis->pixelToTimeRelativeRef(pxPoint.x());

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);
        findIntersect(p, time, &intersect[i]);

        if (intersect[i].x() != -1) ix = intersect[i].x();
    }

    // no intersect found
    if (ix == -1) return NULL;


    // calculate distance to pxPoint
    double dist[mSignals.size()];
    for (int i = 0; i < mSignals.size(); i++) {

        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (intersect[i].x() == -1) {
            dist[i] = 12345678; // large value
            continue;
        }

        double yPx = (mNumPxPerDiv/p->mSignal->vPerDiv())*(-intersect[i].y())
                + p->mGndPos;
        dist[i] = pxPoint.y()-yPx;
        if (dist[i] < 0) dist[i] = -dist[i];
    }

    // find signal with smallest distance
    UiAnalogSignalPrivate* result = NULL;
    double resultDist = 12345678;
    for (int i = 0; i < mSignals.size(); i++) {

        if (dist[i] <= 15 && dist[i] < resultDist) {
            resultDist = dist[i];
            result = mSignals.at(i);
        }
    }

    return result;
}

/*!
    Paint horizontal division lines using \a painter.
*/
void UiAnalogSignal::paintDivLines(QPainter* painter)
{
    painter->save();

    QPen pen = painter->pen();
    pen.setColor(Qt::lightGray);
    pen.setStyle(Qt::DotLine);
    painter->setPen(pen);

    int pX = plotX();
    for (int i = mNumPxPerDiv; i < height(); i+= mNumPxPerDiv) {
        painter->drawLine(pX, i, width(), i);
    }
    painter->restore();
}

/*!
    Paint a specific signal value at \a time.
*/
void UiAnalogSignal::paintSignalValue(QPainter* painter, double time)
{
    qreal ix = -1;

    QList<double> level;
    QList<double> pk;


    QVector<QPointF> intersect(mSignals.size());

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        findIntersect(p, time, &intersect[i]);

        if (intersect[i].x() != -1) ix = intersect[i].x();
    }

    if (ix == -1) return;

    double xPix = mTimeAxis->timeToPixelRelativeRef(ix);

    if (xPix < plotX()) return;

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (intersect[i].x() != -1) {

            double yPx = (mNumPxPerDiv/p->mSignal->vPerDiv())
                    *(-intersect[i].y()) + p->mGndPos;

            QString voltageLevel =
                    QString::number(intersect[i].y(), 'f', 2) + " V";

            QPen pen = painter->pen();
            pen.setColor(Configuration::instance().textColor());
            painter->setPen(pen);

            painter->drawText(xPix+3, yPx-3, voltageLevel);
            painter->fillRect(xPix-2, yPx-1, 5, 5, Configuration::instance()
                              .analogSignalColor(p->mSignal->id()));

            level.append(intersect[i].y());
            pk.append(p->calcPeakToPeak());
        }
    }

    emit measurmentChanged(level, pk, true);
}

/*!
    Paint all signals.
*/
void UiAnalogSignal::paintSignals(QPainter* painter)
{

    CaptureDevice* device = DeviceManager::instance().activeDevice()
            ->captureDevice();
    int pX = plotX();

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);
        int id = p->mSignal->id();

        QPen pen = painter->pen();

        // info part of the signal
        painter->save();

        painter->setRenderHint(QPainter::Antialiasing);
        p->paintInfo(painter, Configuration::instance().analogSignalColor(id));
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        if (i > 0) {
            painter->drawLine(0, i*height()/mSignals.size(), infoWidth(),
                              i*height()/mSignals.size());
        }

        painter->restore();

        QVector<double>* data = device->analogData(id);

        // no signal data
        if (data == NULL) continue;

        int rate = device->usedSampleRate();
        int fromIdx = (int)(mTimeAxis->rangeLower()*rate);

        if (fromIdx >= data->size()) continue;
        if (fromIdx < 0) fromIdx = 0;

        painter->save();

        painter->setClipRect(pX, 0, width()-pX, height());
        painter->translate(0, p->mGndPos);

        // draw gnd line
        pen.setColor(Configuration::instance().analogGroundColor(id));
        pen.setStyle(Qt::DashLine);
        painter->setPen(pen);
        painter->drawLine(pX, 0, width(), 0);

        // draw signal
        pen.setColor(Configuration::instance().analogSignalColor(id));
        pen.setStyle(Qt::SolidLine);
        painter->setPen(pen);

        double from;
        double to;

        double fromVal;
        double toVal;

        double minVal;
        double maxVal;

        double tOnePixel = mTimeAxis->pixelToTime(1)-mTimeAxis->pixelToTime(0);
        for (int j = fromIdx+1; j < data->size(); j++) {

            if ((double)(j-fromIdx)/rate < tOnePixel) continue;

            from = mTimeAxis->timeToPixelRelativeRef((double)fromIdx/rate);
            to = mTimeAxis->timeToPixelRelativeRef((double)j/rate);

            // no need to draw when signal is out of plot area
            if (to < 0) continue;
            if (from > width()) break;

            fromVal = data->at(fromIdx);
            toVal = data->at(j);

            //
            // When skipping data due to optimization we just don't draw a line
            // between the 'from' value and 'to' value. Instead we find the minimum
            // and maximum values in the dataset between 'from' and 'to' and draw
            // a line between these values. This gives a more correct view of
            // the signal.
            //
            if (j > fromIdx + 1) {
                minVal = 999;
                maxVal = -999;
                for (int k = fromIdx; k <= j; k++) {
                    if (data->at(k) < minVal) {
                        minVal = data->at(k);
                    }

                    if (data->at(k) > maxVal) {
                        maxVal = data->at(k);
                    }
                }

                if (data->at(fromIdx) < data->at(j)) {
                    fromVal = minVal;
                    toVal = maxVal;
                }
                else {
                    fromVal = maxVal;
                    toVal = minVal;
                }


            }

            painter->drawLine(from,
                              (mNumPxPerDiv/p->mSignal->vPerDiv())*(-fromVal),
                              to,
                              (mNumPxPerDiv/p->mSignal->vPerDiv())*(-toVal));

            fromIdx = j;
        }

        painter->restore();

    }


}

/*!
    Paint the trigger level.
*/
void UiAnalogSignal::paintTriggerLevel(QPainter* painter)
{

    painter->save();

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if(p->mAnalogTrigger->state() != AnalogSignal::AnalogTriggerNone) {

            painter->setClipRect(plotX(), 0, width()-plotX(), height());
            painter->translate(0, p->mGndPos);

            QPen pen = painter->pen();
            pen.setColor(Configuration::instance()
                         .analogSignalColor(p->mSignal->id()));
            pen.setWidth(2);
            pen.setStyle(Qt::DotLine);
            painter->setPen(pen);
            int y = (mNumPxPerDiv/p->mSignal->vPerDiv())
                    *(-p->mAnalogTrigger->level());
            painter->drawLine(plotX(), y, width(), y);

            break;
        }

    }

    painter->restore();
}

/*!
    Called when the info width has changed.
*/
void UiAnalogSignal::infoWidthChanged()
{
    doLayout();
}

/*!
    Update the layout, that is, position and redraw signals.
*/
void UiAnalogSignal::doLayout()
{

    int x = InfoMarginLeft;
    int y = InfoMarginTop;

    // calculate required height for this widget
    int wHeight = InfoMarginTop+InfoMarginBottom;
    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        if (i > 0) {
            wHeight += DistanceBetweenArea;
        }
        wHeight += p->minimumHeight();
    }

    if (wHeight < 200) {
        wHeight = 200;
    }

    int areaHeight = 0;

    if (mSignals.size() > 0) {
        areaHeight = (wHeight-InfoMarginTop-InfoMarginBottom
                      -(mSignals.size()-1)*DistanceBetweenArea)
                / mSignals.size();
    }

    int oldHeight = height();
    resize(width(), wHeight);

    if (oldHeight != wHeight) {
        emit sizeChanged();
    }

    for (int i = 0; i < mSignals.size(); i++) {
        UiAnalogSignalPrivate* p = mSignals.at(i);

        int h = areaHeight;

        p->setGeometry(x, y,
                       infoWidth()-InfoMarginLeft-InfoMarginRight,
                       areaHeight);

        y += h + DistanceBetweenArea;
    }

    mNumPxPerDiv = height()/NumDivs;
}


