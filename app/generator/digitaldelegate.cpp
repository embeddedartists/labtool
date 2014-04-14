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
#include "digitaldelegate.h"

#include <QDebug>
#include <QPainter>
#include <QEvent>
#include <QMouseEvent>

#include <QMessageBox>
#include <QSpinBox>

#include "uieditdigital.h"

#include "common/configuration.h"
#include "device/digitalsignal.h"


/*!
    \class DigitalDelegate
    \brief This class provides the display and editing facilities for a
    digital signal.

    \ingroup Generator

    Digital signals are visualized in a table as rows and columns. This
    delegate is responsible for displaying the signal data and making
    it possible to modify signal states.
*/

/*!
    Constructs an DigitalDelegate with the given \a parent.
*/
DigitalDelegate::DigitalDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

/*!
    Reimplemented from QStyledItemDelegate::paint.

    Renders the digital signal in the associated view.
*/
void DigitalDelegate::paint(QPainter *painter,
                            const QStyleOptionViewItem &option,
                            const QModelIndex &index) const
{
    do {

        if (!index.data().canConvert<DigitalSignal*>()) break;
        DigitalSignal* signal = index.data().value<DigitalSignal*>();

        if (signal == NULL) break;

        if (option.state & QStyle::State_Selected) {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        QString str = "";

        painter->save();

        if (index.column() == 0) {
            int colorSquareSize = option.rect.height()/2;
            int colorSquareGap = 5;
            str = signal->name();
            QColor c = Configuration::instance().digitalCableColor(signal->id());
            painter->fillRect(option.rect.x()+colorSquareGap,
                              option.rect.y() + (option.rect.height()-colorSquareSize)/2,
                              colorSquareSize,
                              colorSquareSize, c);
            if (c == Qt::white) {
                // Add border so that the square is not invisible
                painter->drawRect(option.rect.x()+colorSquareGap,
                                  option.rect.y() + (option.rect.height()-colorSquareSize)/2,
                                  colorSquareSize,
                                  colorSquareSize);
            }
            painter->drawText(option.rect.x() + colorSquareSize + 2*colorSquareGap,
                              option.rect.y(),
                              option.rect.width() - (colorSquareSize + 2*colorSquareGap),
                              option.rect.height(),
                              Qt::AlignLeft | Qt::AlignVCenter, str);
        }

        else {
            bool high = signal->data().at(index.column()-1);
            str = "1";
            if (!high) {
                str = "0";
            }

            QRect txtRect = option.rect;

            QPen pen = painter->pen();
            pen.setWidth(2);
            painter->setPen(pen);

            if (high) {
                painter->drawLine(option.rect.left(), option.rect.top()+2,
                                  option.rect.right(), option.rect.top()+2);
                txtRect.adjust(0, 2, 0, 0);

            }
            else {
                painter->drawLine(option.rect.left(), option.rect.bottom()-2,
                                  option.rect.right(), option.rect.bottom()-2);

                txtRect.adjust(0, 0, 0, -2);
            }


            pen.setColor(Qt::gray);
            painter->setPen(pen);
            painter->drawText(txtRect, Qt::AlignCenter, str);
        }

        painter->restore();



        return;

    } while (false);


    // let the parent try to paint if we failed (should never happen)
    QStyledItemDelegate::paint(painter, option, index);
}

/*!
    Reimplemented from QStyledItemDelegate::sizeHint.

    Returns the size needed by the delegate to display the item specified
    by \a index.
*/
QSize DigitalDelegate::sizeHint(const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{

    do {

        if (!index.data().canConvert<DigitalSignal*>()) break;
        DigitalSignal* signal = index.data().value<DigitalSignal*>();

        if (signal == NULL) break;


        QSize sz;

        if (index.column() == 0) {
            QString name = signal->name();
            sz.setWidth(option.fontMetrics.width(name)+22);
            sz.setHeight(option.fontMetrics.height()+2);
        }

        else {
            sz.setWidth(option.fontMetrics.width('0')+2);
            sz.setHeight(option.fontMetrics.height()+8);
        }



        return sz;

    } while (false);


    return QStyledItemDelegate::sizeHint(option, index);
}

/*!
    Reimplemented from QStyledItemDelegate::editorEvent.

    When editing of an item starts, this function is called with the \a event
    that triggered the editing.

*/
bool DigitalDelegate::editorEvent(QEvent *event,
                                  QAbstractItemModel *model,
                                  const QStyleOptionViewItem &option,
                                  const QModelIndex &index)
{

    do {

        if (!index.data().canConvert<DigitalSignal*>()) break;
        DigitalSignal* signal = index.data().value<DigitalSignal*>();

        if (signal == NULL) break;

        if (index.column() > 0) {

            if (event->type() == QEvent::MouseButtonRelease) {
                signal->toogleState(index.column()-1);
            }

            return true;
        }


    } while (false);

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

/*!
    Reimplemented from QStyledItemDelegate::createEditor.

    Returns the widget used to edit the item specifyied by \a index.

*/
QWidget* DigitalDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    // we only allow editing of items in column 0

    if (index.column() == 0) {

        do {
            if (!index.data().canConvert<DigitalSignal*>()) break;
            DigitalSignal* signal = index.data().value<DigitalSignal*>();

            if (signal == NULL) break;

            // Deallocation: "Qt Object trees" (See UiMainWindow)
            UiEditDigital* editor = new UiEditDigital(signal, parent);
            editor->move(parent->mapToGlobal(parent->pos()));

            return editor;

        } while(false);
    }

    return QStyledItemDelegate::createEditor(parent, option, index);
}


/*!
    Reimplemented from QStyledItemDelegate::updateEditorGeometry.
*/
void DigitalDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &index) const
{
    (void)editor;
    (void)option;
    (void)index;

    // Do nothing. The geometry has been set in createEditor (relative
    // to the parent).
}

