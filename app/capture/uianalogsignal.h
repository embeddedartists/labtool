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
#ifndef UIANALOGSIGNAL_H
#define UIANALOGSIGNAL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QShowEvent>
#include <QPushButton>
#include <QImage>

#include "uiabstractsignal.h"

#include "device/analogsignal.h"

class UiAnalogSignalPrivate;

class UiAnalogSignal : public UiAbstractSignal
{
    Q_OBJECT
public:

    enum Constants {
        MaxNumSignals = 4
    };

    explicit UiAnalogSignal(QWidget *parent = 0);
    ~UiAnalogSignal();

    void addSignal(AnalogSignal *signal);
    QList<AnalogSignal*> addedSignals();

    void clearTriggers();
    
signals:
    void measurmentChanged(QList<double>level, QList<double>pk, bool active);
    void triggerSet();
    
public slots:

protected:
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void showEvent(QShowEvent* event);
    void leaveEvent(QEvent* event);

private slots:
    void nameEdited();

    void disableSignal();
    void changeVPerDiv(double v);
    void changeTriggers();
    void handleTriggerLevelChanged();
    void handleCouplingChanged(QAbstractButton* btn);

private:


    enum PrivateConstants {
        NumDivs = 10,
        DistanceBetweenArea = 4,
        SignalIdMarginRight = 10
    };


    QList<UiAnalogSignalPrivate*> mSignals;

    int mNumSupportedSignals;

    bool mDragging;
    double mDragStart;
    UiAnalogSignalPrivate* mDragSignal;

    int mMouseOverXPos;
    bool mMouseOverValid;

    static const double MaxVPerDiv;
    static const double MinVPerDiv;
    int mNumPxPerDiv;

    void setName(QString &name, UiAnalogSignalPrivate *signal);
    int calcMinimumWidth();
    void updateMinimumWidth();

    UiAnalogSignalPrivate *findSignal(QPoint pxPoint);
    void findIntersect(UiAnalogSignalPrivate* signal, double time, QPointF* intersect);


    void paintDivLines(QPainter* painter);
    void paintSignalValue(QPainter* painter, double time);
    void paintSignals(QPainter* painter);
    void paintTriggerLevel(QPainter* painter);

    void infoWidthChanged();
    void doLayout();
    void disableSignal(int idx);


    friend class UiAnalogSignalPrivate;
};

#endif // UIANALOGSIGNAL_H
