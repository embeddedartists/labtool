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
#include "uicursorgroup.h"

#include <QDebug>
#include "common/stringutil.h"
#include "device/devicemanager.h"

/*!
    \class UiCursorGroup
    \brief UI widget that show cursor related measurements.

    \ingroup Capture

*/


/*!
    Constructs an UiCursorGroup with the given \a parent.
*/
UiCursorGroup::UiCursorGroup(QWidget *parent) :
    QGroupBox(parent),
    mMinSize(0, 0)
{
    setTitle("Cursor Measurements");
    setupLabels();
}

/*!
    Enable/disable the cursor \a cursor according to parameter \a enabled and
    set the cursor position to \a time.
*/
void UiCursorGroup::setCursorData(UiCursor::CursorId cursor, bool enabled,
                                  double time)
{

    QLabel* lbl = NULL;
    switch (cursor) {
    case UiCursor::Cursor1:
        lbl = &mCursorTime[0];
        mCursorTimes[0] = time;

        break;
    case UiCursor::Cursor2:
        lbl = &mCursorTime[1];
        mCursorTimes[1] = time;

        break;
    case UiCursor::Cursor3:
        lbl = &mCursorTime[2];
        mCursorTimes[2] = time;

        break;
    case UiCursor::Cursor4:
        lbl = &mCursorTime[3];
        mCursorTimes[3] = time;

        break;
    default:
        return;
    }

    lbl->setEnabled(enabled);
    if (!enabled) {
        lbl->setText("");

    }
    else {

        // display time relative to trigger
        CaptureDevice* device = DeviceManager::instance().activeDevice()
                ->captureDevice();
        double triggerTime = (double)device->digitalTriggerIndex()
                /device->usedSampleRate();


        lbl->setText(StringUtil::timeInSecToString(time-triggerTime));
    }


    for (int i = 0; i < UiCursor::NumCursors-1; i+=2) {


        if (mCursorTime[i].isEnabled() && mCursorTime[i+1].isEnabled()) {
            double diff = mCursorTimes[i] - mCursorTimes[i+1];
            if (diff < 0) diff = -diff;

            mCursorPairTime[i/2].setText(StringUtil::timeInSecToString(diff));

            if (diff != 0) {
                mCursorPairFreq[i/2].setText(
                            StringUtil::frequencyToString(1/diff));
            } else {
                // infinity character
                mCursorPairFreq[i/2].setText(QString(0x221e));
            }

        }
        else {
            mCursorPairTime[i/2].setText("");
            mCursorPairFreq[i/2].setText("");
        }

    }

    doLayout();

}

/*!
    This event handler is called when this widget is made visible.
*/
void UiCursorGroup::showEvent(QShowEvent* event)
{
    (void)event;
    doLayout();
}

/*!
    Returns the minimum size of this widget.
*/
QSize UiCursorGroup::minimumSizeHint() const
{
    return mMinSize;
}

/*!
    Returns the recommended size of this widget.
*/
QSize UiCursorGroup::sizeHint() const
{
    return minimumSizeHint();
}

/*!
    Create and setup labels.
*/
void UiCursorGroup::setupLabels()
{
    for (int i = 0; i < UiCursor::NumCursors-1; i++) {
        mCursorTimeLbl[i].setParent(this);
        mCursorTime[i].setParent(this);

        mCursorTimeLbl[i].setText(QString("C%1:").arg(i+1));
        mCursorTime[i].setEnabled(false);
    }

    for (int i = 0; i < (UiCursor::NumCursors-1)/2; i++) {
        mCursorPairFreqLbl[i].setParent(this);
        mCursorPairFreq[i].setParent(this);
        mCursorPairFreqLbl[i].setText(QString("C%1-C%2 [f]:")
                                      .arg((i*2+1)).arg((i*2+2)));

        mCursorPairTimeLbl[i].setParent(this);
        mCursorPairTime[i].setParent(this);
        mCursorPairTimeLbl[i].setText(QString("C%1-C%2 [t]:")
                                      .arg((i*2+1)).arg((i*2+2)));
    }
}

/*!
    Position child widgets.
*/
void UiCursorGroup::doLayout()
{
    int maxLblWidth = 0;
    int minWidth = 0;
    QMargins boxMargins = contentsMargins();

    /*
        make sure all labels are resized to their minimum size
    */

    for (int i = 0; i < UiCursor::NumCursors-1; i++) {
        mCursorTimeLbl[i].resize(mCursorTimeLbl[i].minimumSizeHint());
        mCursorTime[i].resize(mCursorTime[i].minimumSizeHint());

        if (mCursorTimeLbl[i].minimumSizeHint().width() > maxLblWidth) {
            maxLblWidth = mCursorTimeLbl[i].minimumSizeHint().width();
        }
    }
    for (int i = 0; i < (UiCursor::NumCursors-1)/2; i++) {
        mCursorPairFreqLbl[i].resize(mCursorPairFreqLbl[i].minimumSizeHint());
        mCursorPairFreq[i].resize(mCursorPairFreq[i].minimumSizeHint());

        if (mCursorPairFreqLbl[i].minimumSizeHint().width() > maxLblWidth) {
            maxLblWidth = mCursorPairFreqLbl[i].minimumSizeHint().width();
        }

        mCursorPairTimeLbl[i].resize(mCursorPairTimeLbl[i].minimumSizeHint());
        mCursorPairTime[i].resize(mCursorPairTime[i].minimumSizeHint());

        if (mCursorPairTimeLbl[i].minimumSizeHint().width() > maxLblWidth) {
            maxLblWidth = mCursorPairTimeLbl[i].minimumSizeHint().width();
        }
    }

    /*
        position the labels
    */

    int yPos = MarginTop + boxMargins.top();
    int xPos = MarginLeft + boxMargins.left();
    int xPosRight = xPos + maxLblWidth + HoriDistBetweenRelated;

    for (int i = 0; i < UiCursor::NumCursors-1; i++) {
        mCursorTimeLbl[i].move(xPos, yPos);
        mCursorTime[i].move(xPosRight, yPos);

        yPos += mCursorTimeLbl[i].height()+VertDistBetweenRelated;

        if (mCursorTime[i].x()+mCursorTime[i].width() > minWidth) {
            minWidth = mCursorTime[i].x()+mCursorTime[i].width();
        }
    }

    yPos += VertDistBetweenUnrelated;
    for (int i = 0; i < (UiCursor::NumCursors-1)/2; i++) {
        mCursorPairFreqLbl[i].move(xPos, yPos);
        mCursorPairFreq[i].move(xPosRight, yPos);
        yPos += mCursorPairFreqLbl[i].height() + VertDistBetweenRelated;

        if (mCursorPairFreq[i].x()+mCursorPairFreq[i].width() > minWidth) {
            minWidth = mCursorPairFreq[i].x()+mCursorPairFreq[i].width();
        }

        mCursorPairTimeLbl[i].move(xPos, yPos);
        mCursorPairTime[i].move(xPosRight, yPos);
        yPos += mCursorPairFreqLbl[i].height() + VertDistBetweenUnrelated;

        if (mCursorPairTime[i].x()+mCursorPairTime[i].width() > minWidth) {
            minWidth = mCursorPairTime[i].x()+mCursorPairTime[i].width();
        }
    }

    /*
        size constraints
    */

    // this label is positioned at the bottom of the group
    int lastLabel = (UiCursor::NumCursors-1)/2-1;

    mMinSize.setHeight(mCursorPairTime[lastLabel].y()
                       +mCursorPairTime[lastLabel].height()
                       +MarginBottom+boxMargins.bottom());

    // check if QGroupBox has a larger width (because of the box title).
    if (QGroupBox::minimumSizeHint().width()+5 > minWidth) {
        minWidth = QGroupBox::minimumSizeHint().width()+5;
    }
    mMinSize.setWidth(minWidth);

}


