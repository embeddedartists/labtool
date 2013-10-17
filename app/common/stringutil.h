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
#ifndef STRINGUTIL_H
#define STRINGUTIL_H

#include <QString>

class StringUtil
{
public:
    StringUtil();

    static QString timeInSecToString(double time);
    static QString frequencyToString(double freq);

    static bool isFrequencyStringValid(QString &freqStr);
    static QString frequencyToString(int freqInHz);
    static int frequencyToInt(QString &freqStr);

private:
    static const QString FrequencyRegExpPattern;
};

#endif // STRINGUTIL_H
