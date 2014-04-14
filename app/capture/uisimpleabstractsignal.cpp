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
#include "uisimpleabstractsignal.h"

#include <QDebug>

/*!
    \class UiSimpleAbstractSignal
    \brief Abstract base class for "simple" signal widgets.

    \ingroup Capture

    This class handles common tasks for signal widgets which represent
    ONE signal, that is, one ID, one name, and so on.

*/


/*!
    Constructs an UiSimpleAbstractSignal with the given \a parent.
*/
UiSimpleAbstractSignal::UiSimpleAbstractSignal(QWidget *parent) :
    UiAbstractSignal(parent)
{

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mDisableBtn = new QPushButton(this);
    mDisableBtn->setIcon(QIcon(":/resources/8_close.png"));
    mDisableBtn->setFlat(true);
    mDisableBtn->resize(12, 12); //slightly bigger than the 8x8 icon
    connect(mDisableBtn, SIGNAL(clicked()), this, SLOT(closeSignal()));

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mConfigureBtn = new QPushButton(this);
    mConfigureBtn->setIcon(QIcon(":resources/8_configure.png"));
    mConfigureBtn->setFlat(true);
    mConfigureBtn->resize(12, 12);
    connect(mConfigureBtn, SIGNAL(clicked()), this, SLOT(configure()));
    // by default a simple signal is not configurable
    mConfigureBtn->hide();

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mColorLbl = new QLabel(this);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mIdLbl = new QLabel(this);

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mNameLbl = new QLabel(this);

    // mEditName is used when changing the name of a signal
    // it will be displayed when a user clicks on the signal name

    // Deallocation: "Qt Object trees" (See UiMainWindow)
    mEditName = new QLineEdit(this);
    mEditName->hide();
    connect(mEditName, SIGNAL(editingFinished()), this,
            SLOT(nameEdited()));

    mSelected = false;
}

/*!
    Set the signal name to \a name.
*/
void UiSimpleAbstractSignal::setSignalName(QString name)
{

    mNameLbl->setText(name);

    mEditName->hide();
    mNameLbl->show();

    setMinimumInfoWidth(calcMinimumWidth());
}

/*!
    Returns the name of this signal.
*/
QString UiSimpleAbstractSignal::getName() const
{
    return mNameLbl->text();
}

/*!
    \var QLabel* UiSimpleAbstractSignal::mIdLbl

    ID label.
*/

/*!
    \var QLabel* UiSimpleAbstractSignal::mNameLbl

    Signal name label.
*/

/*!
    \var QLineEdit* UiSimpleAbstractSignal::mEditName

    Signal name editor widget.
*/


/*!
    Sets the signal widget to be configurable (shows config symbol).
*/
void UiSimpleAbstractSignal::setConfigurable()
{
    mConfigureBtn->show();
}

/*!
    The mouse press event handler is called when a mouse button is pressed.
*/
void UiSimpleAbstractSignal::mousePressEvent(QMouseEvent* event)
{

    if (event->button() == Qt::LeftButton) {

        if (mNameLbl->geometry().contains(event->pos())) {

            // enable editing the signal name
            if (mNameLbl->isVisible()) {
                mNameLbl->hide();
                mEditName->setText(mNameLbl->text());
                mEditName->show();
                mEditName->setFocus();

                setMinimumInfoWidth(calcMinimumWidth());
            }

        }

    }

    QWidget::mousePressEvent(event);
}

/*!
    Layouts the child widgets
*/
void UiSimpleAbstractSignal::doLayout()
{
    QMargins m = infoContentMargin();
    mDisableBtn->move(infoWidth()-mDisableBtn->width()-m.right(), m.top());

    int x = mDisableBtn->pos().x()-mConfigureBtn->width();
    mConfigureBtn->move(x, m.top());
}


/*!
    \fn virtual int UiSimpleAbstractSignal::calcMinimumWidth() = 0

    Calculates and returns the minimum width for this widget.
*/

/*!
    \fn virtual void UiSimpleAbstractSignal::configure(QWidget* parent)

    Handles a configure request for this widget.
*/


/*!
    Returns rectangle offsets for this widget where content can be placed
*/
QRect UiSimpleAbstractSignal::infoContentRect()
{
    QRect r = UiAbstractSignal::infoContentRect();
    r.adjust(0, mDisableBtn->height(), 0, 0);
    return r;
}

/*!
    Called when the name has changed
*/
void UiSimpleAbstractSignal::nameEdited()
{

    QString n = mEditName->text();
    if (n.isNull() || n.size() == 0) {
        n = mNameLbl->text();
    }

    setSignalName(n);
}

/*!
    Called when the widget is asked to be configured.
*/
void UiSimpleAbstractSignal::configure()
{
    configure(this);
}



