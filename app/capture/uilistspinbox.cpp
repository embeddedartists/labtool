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
#include "uilistspinbox.h"

#include <QDebug>

/*!
    \class UiListSpinBox
    \brief A Spin Box that gets its supported values from a list of values
    instead of a range of values (which is the case for QSpinBox).

    \ingroup Capture

*/


/*!
    Constructs an UiListSpinBox with the given \a parent.
*/
UiListSpinBox::UiListSpinBox(QWidget *parent) :
    QSpinBox(parent)
{
    setRange(0, 0);
    connect(this, SIGNAL(valueChanged(int)),
            this, SLOT(handleValueChanged(int)));
}

/*!
    Set the current value to \a val.
*/
void UiListSpinBox::setValue(double val)
{

    for (int i = 0; i < mValues.size(); i++) {
        double diff = qAbs<double>(mValues.at(i) - val);
        if (mValues.at(i) == val || diff < 0.001) {
            QSpinBox::setValue(i);
            break;
        }
    }
}

/*!
    Set the list of supported values to \a list.
*/
void UiListSpinBox::setSupportedValues(QList<double> &list)
{
    mValues = list;
    setRange(0, mValues.size()-1);
    setValue(mValues.at(list.size()/2));

}


/*!
    \fn void UiListSpinBox::valueChanged(double d)

    This signal is emitted when the value of the spin box has been changed.
*/


/*!
    Get the text representation of the value \a value.
*/
QString UiListSpinBox::textFromValue(int value) const
{

    QString v = "";

    if (value >= 0 && value < mValues.size()) {
        v = QString("%1").arg(mValues.at(value));
    }

    return v;
}

/*!
    Get the value given the text representation \a text.
*/
int UiListSpinBox::valueFromText(const QString &text) const
{
    bool ok = false;
    int result = value();

    QString suf = suffix();
    QString s(text);
    s = s.remove(suf).trimmed();

    double v = s.toDouble(&ok);

    if (ok) {
        for (int i = 0; i < mValues.size(); i++) {
            double diff = qAbs<double>(mValues.at(i) - v);
            if (mValues.at(i) == v || diff < 0.001) {
                result = i;
                break;
            }
        }
    }

    return result;
}

/*!
    Determines if the input \a text at position \a pos is a valid.
*/
QValidator::State UiListSpinBox::validate(QString &text, int &pos) const
{

    if (pos == text.size() || pos == 0) return QValidator::Acceptable;

    if (text.at(pos-1).isDigit()) return QValidator::Acceptable;

    if (text.at(pos-1) == '.' || text.at(pos-1) == ',') {
        int n1 = text.count('.');
        int n2 = text.count(',');

        if (n1 > 1 || n2 > 2) {
            return QValidator::Invalid;
        }
        else {
            return QValidator::Intermediate;
        }

    }

    return QValidator::Invalid;
}

/*!
    Called when the value has changed.
*/
void UiListSpinBox::handleValueChanged(int i)
{
    if (i >= 0 && i < mValues.size()) {
        emit valueChanged(mValues.at(i));
    }
}
