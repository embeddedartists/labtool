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
#include "uianalyzer.h"

/*!
    \class UiAnalyzer
    \brief This is a base class for all analyzers.

    \ingroup Analyzer

*/


/*!
    Constructs the SignalManager with the given \a parent.
*/
UiAnalyzer::UiAnalyzer(QWidget *parent) :
    UiSimpleAbstractSignal(parent)
{
    setConfigurable();
}


/*!
    \fn virtual void UiAnalyzer::analyze() = 0

    Call to start to analyze the signal(s)
*/

/*!
    \fn virtual QString UiAnalyzer::toSettingsString() const = 0

    Create a string representation of this analyzer.
*/


/*!
    Called when signal data has changed.
*/
void UiAnalyzer::handleSignalDataChanged()
{
    analyze();
}


/*!
    \fn virtual void UiAnalyzer::configure(QWidget* parent) = 0

    Configure this analyzer. If an analyzer can be configured a subclass can
    show a dialog window using \a parent as UI context.
*/


/*!
    Helper function to convert the value \a value to a string according
    to \a format.
*/
QString UiAnalyzer::formatValue(Types::DataFormat format, int value)
{
    QLatin1Char fillChar('0');
    QString s;

    switch(format) {
    case Types::DataFormatHex:
        s = QString("0x%1").arg(value, 2, 16, fillChar);
        break;
    case Types::DataFormatDecimal:
        s = QString("%1").number(value, 10);
        break;
    case Types::DataFormatAscii:
        s = QChar(value).toLatin1();
        break;
    default:
        break;
    }

    return s;
}
