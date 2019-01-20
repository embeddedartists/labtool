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
#include "configuration.h"

#define COLOR_SCHEME_LIGHT "Light"
#define COLOR_SCHEME_DARK  "Dark"

const QString Configuration::ProjectFilename   = "Default.prj";
const QString Configuration::ProjectFileExt    = ".prj";
const QString Configuration::ProjectBinFileExt = ".eab";

/*!
    \class Configuration
    \brief This class keeps configuration settings common for all parts of
    the application.

    \ingroup Common

*/

/*!
    \fn Configuration& Configuration::instance()

    Returns an instance of the Configuration class.
*/

/*!
    \var const QString Configuration::ProjectFilename

    The default project file name.
*/

/*!
    \var const QString Configuration::ProjectFileExt

    The file extension used for project files.
*/

/*!
    \var const QString Configuration::ProjectBinFileExt

    The file extension used for binary signal data.
*/



/*!
    Constructs the Configuration.
*/
Configuration::Configuration()
{
    loadColorScheme(COLOR_SCHEME_LIGHT);

    // Cable colors does not change with the color scheme
    // so it is specified here instead
    mDigitalCableColors[0] = Qt::black;
    mDigitalCableColors[1] = QColor("#A0522D"); // sienna (brown)
    mDigitalCableColors[2] = Qt::red;
    mDigitalCableColors[3] = QColor("#FFA500"); // orange
    mDigitalCableColors[4] = Qt::yellow;
    mDigitalCableColors[5] = QColor("#32CD32"); // lime green
    mDigitalCableColors[6] = Qt::blue;
    mDigitalCableColors[7] = QColor("#DA70D6"); // orchid (purple)
    mDigitalCableColors[8] = Qt::gray;
    mDigitalCableColors[9] = Qt::white;
    mDigitalCableColors[10] = Qt::black;
    mAnalogInCableColors[0] = Qt::black;
    mAnalogInCableColors[1] = Qt::red;
    mAnalogOutCableColors[0] = Qt::blue;
    mAnalogOutCableColors[1] = Qt::gray;
}

/*!
    Returns a list of the color schemes defined for this applicaiton.
*/
QList<QString> Configuration::colorSchemes()
{
    return QList<QString>()
            << COLOR_SCHEME_LIGHT
            << COLOR_SCHEME_DARK;
}

/*!
    Returns the active color scheme.
*/
QString Configuration::activeColorScheme()
{
    return mActiveColorScheme;
}

/*!
    Load the color scheme with name \a scheme.
*/
void Configuration::loadColorScheme(QString scheme)
{
    if (scheme == COLOR_SCHEME_LIGHT) {
        loadLightScheme();
    }
    else if (scheme == COLOR_SCHEME_DARK) {
        loadDarkScheme();
    }
}

/*!
    Returns the background color for the signal plot.
*/
QColor Configuration::plotBackgroundColor()
{
    return mPlotBackgroundColor;
}

/*!
    Set the background color of the signal plot to \a c.
*/
void Configuration::setPlotBackgroundColor(QColor &c)
{
    mPlotBackgroundColor = c;
}

/*!
    Returns the color used when drawing text.
*/
QColor Configuration::textColor()
{
    return mTextColor;
}

/*!
    Set the color used when drawing text to \a c.
*/
void Configuration::setTextColor(QColor &c)
{
    mTextColor = c;
}

/*!
    Returns the color for the digital signal with ID \a id.
*/
QColor Configuration::digitalSignalColor(int id)
{
    if (id < 0) id = 0;

    return mDigitalSignalColors[ (id%MaxDigitalColors) ];
}

/*!
    Set the color for the digital signal with ID \a id to \a c.
*/
void Configuration::setDigitalSignalColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mDigitalSignalColors[ (id%MaxDigitalColors) ] = c;
}

/*!
    Returns the color for the analog signal with ID \a id.
*/
QColor Configuration::analogSignalColor(int id)
{
    if (id < 0) id = 0;

    return mAnalogSignalColors[ (id%MaxAnalogColors) ];
}

/*!
    Set the color for the analog signal with ID \a id to \a c.
*/
void Configuration::setAnalogSignalColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mAnalogSignalColors[ (id%MaxAnalogColors) ] = c;
}

/*!
    Returns the "ground" color for the analog signal with ID \a id.
*/
QColor Configuration::analogGroundColor(int id)
{
    if (id < 0) id = 0;

    return mAnalogGroundColors[ (id%MaxAnalogColors) ];
}

/*!
    Set the ground color for the analog signal with ID \a id to \a c.
*/
void Configuration::setAnalogGroundColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mAnalogGroundColors[ (id%MaxAnalogColors) ] = c;
}

/*!
    Returns the color for the cursor with ID \a id.
*/
QColor Configuration::cursorColor(int id)
{
    if (id < 0) id = 0;

    return mCursorColors[ (id%MaxCursorColors) ];
}

/*!
    Set the color for the cursor with ID \a id to \a c.
*/
void Configuration::setCursorColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mCursorColors[ (id%MaxCursorColors) ] = c;
}

/*!
    Returns the color for grid.
*/
QColor Configuration::gridColor()
{
    return mGridColor;
}

/*!
    Set the color for the grid to \a c.
*/
void Configuration::setGridColor(QColor &c)
{
    mGridColor = c;
}

/*!
    Returns the color used when painting analyzer signals.
*/
QColor Configuration::analyzerColor()
{
    return mAnalyzerColor;
}

/*!
    Set the color used when painting analyzer signals to \a c.
*/
void Configuration::setAnalyzerColor(QColor &c)
{
    mAnalyzerColor = c;
}

/*!
    Returns the color of the cable for the digital signal with ID \a id.
*/
QColor Configuration::digitalCableColor(int id)
{
    if (id < 0) id = 0;

    return mDigitalCableColors[ (id%MaxDigitalColors) ];
}

/*!
    Set the color of the cable for the digital signal with ID \a id to \a c.
*/
void Configuration::setDigitalCableColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mDigitalCableColors[ (id%MaxDigitalColors) ] = c;
}

/*!
    Returns the color of the cable for the analog input signal with ID \a id.
*/
QColor Configuration::analogInCableColor(int id)
{
    if (id < 0) id = 0;

    return mAnalogInCableColors[ (id%MaxAnalogColors) ];
}

/*!
    Set the color of the cable for the analog input signal with ID \a id to \a c.
*/
void Configuration::setAnalogInCableColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mAnalogInCableColors[ (id%MaxAnalogColors) ] = c;
}

/*!
    Returns the color of the cable for the analog output signal with ID \a id.
*/
QColor Configuration::analogOutCableColor(int id)
{
    if (id < 0) id = 0;

    return mAnalogOutCableColors[ (id%MaxAnalogColors) ];
}

/*!
    Set the color of the cable for the analog output signal with ID \a id to \a c.
*/
void Configuration::setAnalogOutCableColor(int id, QColor &c)
{
    if (id < 0) id = 0;

    mAnalogOutCableColors[ (id%MaxAnalogColors) ] = c;
}

/*!
    Returns the color used as background color for widgets surrounding the
    signal plot.
*/
QColor Configuration::outsidePlotColor()
{
    //return QColor(235, 235, 235);
    //return QColor(249, 249, 249);

    // Make outsidePlotColor() obey the color scheme while still providing some contrast.
    // Note that the default black color is #000000, so .lighter(...) will effectively
    // multiply with 0, which does nothing.
    return mActiveColorScheme == COLOR_SCHEME_DARK ?
                // workaround for .lighter(...)
                QColor::fromHsv(mPlotBackgroundColor.hsvHue(), mPlotBackgroundColor.hsvSaturation(),
                                mPlotBackgroundColor.value()+30) :
                QColor(mPlotBackgroundColor).darker(105);
}

/*!
    Load the color scheme called "Light".
*/
void Configuration::loadLightScheme()
{
    mActiveColorScheme = COLOR_SCHEME_LIGHT;

    mPlotBackgroundColor = Qt::white;
    mTextColor           = Qt::black;

    for (int i = 0; i < MaxDigitalColors; i++) {
        mDigitalSignalColors[i] = Qt::black;
    }
    mAnalyzerColor = Qt::black;

    mAnalogSignalColors[0] = QColor(0,255,255);   // Aqua
    mAnalogSignalColors[1] = QColor(30,144,255);  // DodgerBlue
    mAnalogSignalColors[2] = QColor(255,160,120); // LightSalmon
    mAnalogSignalColors[3] = QColor(255,0,0);     // Red

    for (int i = 0; i < MaxAnalogColors; i++) {
        mAnalogGroundColors[i] = Qt::green;
    }

    mCursorColors[0] = Qt::red;
    mCursorColors[1] = Qt::blue;
    mCursorColors[2] = Qt::darkBlue;
    mCursorColors[3] = Qt::green;
    mCursorColors[4] = Qt::darkGreen;

    mGridColor = Qt::gray;
}

/*!
    Load the color scheme called "Dark".
*/
void Configuration::loadDarkScheme()
{
    mActiveColorScheme = COLOR_SCHEME_DARK;

    mPlotBackgroundColor = Qt::black;
    mTextColor           = Qt::white;

    for (int i = 0; i < MaxDigitalColors; i++) {
        mDigitalSignalColors[i] = QColor(240, 240, 240);
    }
    mAnalyzerColor = QColor(240, 240, 240);;

    mAnalogSignalColors[0] = QColor(0,255,255);   // Aqua
    mAnalogSignalColors[1] = QColor(30,144,255);  // DodgerBlue
    mAnalogSignalColors[2] = QColor(255,160,120); // LightSalmon
    mAnalogSignalColors[3] = QColor(255,0,0);     // Red

    for (int i = 0; i < MaxAnalogColors; i++) {
        mAnalogGroundColors[i] = Qt::green;
    }

    mCursorColors[0] = Qt::red;
    mCursorColors[1] = Qt::blue;
    mCursorColors[2] = QColor(0, 255, 255);
    mCursorColors[3] = Qt::green;
    mCursorColors[4] = Qt::darkGreen;

    mGridColor = Qt::gray;
}
