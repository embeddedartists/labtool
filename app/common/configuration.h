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
#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QColor>
#include <QList>

class Configuration
{

public:
    
    static Configuration& instance()
    {
        static Configuration singleton;
        return singleton;
    }

    static const QString ProjectFilename;
    static const QString ProjectFileExt;
    static const QString ProjectBinFileExt;

    QList<QString> colorSchemes();
    QString activeColorScheme();
    void loadColorScheme(QString scheme);

    QColor plotBackgroundColor();
    void   setPlotBackgroundColor(QColor &c);
    QColor textColor();
    void   setTextColor(QColor &c);
    QColor digitalSignalColor(int id);
    void   setDigitalSignalColor(int id, QColor &c);
    QColor analogSignalColor(int id);
    void   setAnalogSignalColor(int id, QColor &c);
    QColor analogGroundColor(int id);
    void   setAnalogGroundColor(int id, QColor &c);
    QColor cursorColor(int id);
    void   setCursorColor(int id, QColor &c);
    QColor gridColor();
    void   setGridColor(QColor &c);
    QColor analyzerColor();
    void   setAnalyzerColor(QColor &c);
    void   setDigitalCableColor(int id, QColor &c);
    QColor digitalCableColor(int id);
    void   setAnalogInCableColor(int id, QColor &c);
    QColor analogInCableColor(int id);
    void   setAnalogOutCableColor(int id, QColor &c);
    QColor analogOutCableColor(int id);

    QColor outsidePlotColor();

    void loadLightScheme();
    void loadDarkScheme();

signals:
    
public slots:

private:

    explicit Configuration();
    // hide copy constructor
    Configuration(const Configuration&);
    // hide assign operator
    Configuration& operator=(const Configuration &);

    enum {
        MaxDigitalColors = 32,
        MaxAnalogColors = 4,
        MaxCursorColors = 5
    };    

    QString mActiveColorScheme;

    QColor mPlotBackgroundColor;
    QColor mTextColor;
    QColor mDigitalSignalColors[MaxDigitalColors];
    QColor mAnalogSignalColors[MaxAnalogColors];
    QColor mAnalogGroundColors[MaxAnalogColors];
    QColor mCursorColors[MaxCursorColors];
    QColor mGridColor;
    QColor mAnalyzerColor;
    QColor mDigitalCableColors[MaxDigitalColors];
    QColor mAnalogInCableColors[MaxAnalogColors];
    QColor mAnalogOutCableColors[MaxAnalogColors];
};

#endif // CONFIGURATION_H

