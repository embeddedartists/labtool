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
#include "analyzermanager.h"
#include "i2c/uii2canalyzer.h"
#include "uart/uiuartanalyzer.h"
#include "spi/uispianalyzer.h"

/*!
    \class AnalyzerManager
    \brief This class is responsible for creating analyzers.

    \ingroup Analyzer

*/


/*!
    Returns list with names of supported analyzers.
*/
QList<QString> AnalyzerManager::analyzers()
{
    // return supported analyzers

    return QList<QString>()
            << UiI2CAnalyzer::signalName
            << UiUartAnalyzer::name
            << UiSpiAnalyzer::signalName;
}

/*!
    Create and return an analyzer based on its \a name.
*/
UiAnalyzer* AnalyzerManager::createAnalyzer(const QString name)
{
    UiAnalyzer* analyzer = NULL;

    if (name == UiI2CAnalyzer::signalName) {
        // Deallocation: caller is responsible for deallocation
        analyzer = new UiI2CAnalyzer();
    }
    else if (name == UiUartAnalyzer::name) {
        // Deallocation: caller is responsible for deallocation
        analyzer = new UiUartAnalyzer();
    }

    else if (name == UiSpiAnalyzer::signalName) {
        // Deallocation: caller is responsible for deallocation
        analyzer = new UiSpiAnalyzer();
    }

    return analyzer;
}

/*!
    Returns a string representation of the specified \a analyzer. This is
    typically used to save the analyzer to persistent storage.
*/
QString AnalyzerManager::analyzerToString(const UiAnalyzer *analyzer)
{
    if (analyzer == NULL) return NULL;

    return analyzer->toSettingsString();
}

/*!
    Create an analyzer from the string representation \a s. This is typically
    used when loading an analyzer from persistent storage.

    \sa analyzerToString
*/
UiAnalyzer* AnalyzerManager::analyzerFromString(const QString &s)
{
    UiAnalyzer* analyzer = NULL;

    QStringList list = s.split(';');
    if (list.size() < 1) return NULL;

    QString type = list.at(0);
    if (type ==  UiI2CAnalyzer::signalName) {
        analyzer = UiI2CAnalyzer::fromSettingsString(s);
    }
    else if (type == UiUartAnalyzer::name) {
        analyzer = UiUartAnalyzer::fromSettingsString(s);
    }
    else if (type == UiSpiAnalyzer::signalName) {
        analyzer = UiSpiAnalyzer::fromSettingsString(s);
    }

    return analyzer;

}

