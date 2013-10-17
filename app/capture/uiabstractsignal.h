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
#ifndef UIABSTRACTSIGNAL_H
#define UIABSTRACTSIGNAL_H

#include <QWidget>
#include <QPainter>
#include <QRect>
#include <QMargins>

#include "uiabstractplotitem.h"
#include "uitimeaxis.h"

class UiAbstractSignal : public UiAbstractPlotItem
{
    Q_OBJECT
public:

    explicit UiAbstractSignal(QWidget *parent = 0);
    void setTimeAxis(UiTimeAxis* axis);
    virtual void handleSignalDataChanged() {}

    
signals:
    void closed(UiAbstractSignal* s);
    
public slots:
    void closeSignal();

protected:
    UiTimeAxis* mTimeAxis;
    bool mSelected;

    enum Margins {
        InfoMarginTop = 1,
        InfoMarginRight = 3,
        InfoMarginBottom = 1,
        InfoMarginLeft = 3
    };

    virtual QRect infoContentRect();
    virtual QMargins infoContentMargin();
    virtual void paintBackground(QPainter* painter);
    virtual void enterEvent(QEvent* event);
    virtual void leaveEvent(QEvent* event);




};

#endif // UIABSTRACTSIGNAL_H
