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
#ifndef UILABTOOLTRIGGERCONFIG_H
#define UILABTOOLTRIGGERCONFIG_H

#include <QWidget>
#include <QDialog>
#include <QSlider>
#include <QLineEdit>
#include <QCheckBox>

class UiLabToolTriggerConfig : public QDialog
{
    Q_OBJECT
public:
    explicit UiLabToolTriggerConfig(QWidget *parent = 0);

    void setPostFillTimeLimit(int ms);
    int postFillTimeLimit();
    void setPostFillPercent(int percent);
    int postFillPercent();
    void setNoiseFilter(bool enabled, int level);
    bool isNoiseFilterEnabled();
    int noiseFilterLevel();

    qint16 noiseFilter12BitLevel() { return (1<<noiseFilterLevel()); }

signals:

public slots:

private slots:

    void noiseFilterStateChanged(int state);

private:
    QSlider* mPostFillPercent;
    QLineEdit * mPostFillTimeLimit;
    QSlider* mNoiseLevel;
    QCheckBox* mNoiseFilterEnabled;

};

#endif // UILABTOOLTRIGGERCONFIG_H
