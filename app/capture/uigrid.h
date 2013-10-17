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
#ifndef UIGRID_H
#define UIGRID_H

#include <QWidget>
#include "uiabstractplotitem.h"
#include "uitimeaxis.h"

class UiGrid : public UiAbstractPlotItem
{
    Q_OBJECT
public:
    explicit UiGrid(UiTimeAxis* axis, QWidget *parent = 0);
    
signals:
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);

private:
    UiTimeAxis* mTimeAxis;

    void infoWidthChanged() {}
    
};

#endif // UIGRID_H
