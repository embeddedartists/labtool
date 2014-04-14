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
#ifndef UISIMPLEABSTRACTSIGNAL_H
#define UISIMPLEABSTRACTSIGNAL_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMouseEvent>

#include "uiabstractsignal.h"

class UiSimpleAbstractSignal : public UiAbstractSignal
{
    Q_OBJECT
public:
    explicit UiSimpleAbstractSignal(QWidget *parent = 0);
    virtual void setSignalName(QString signalName);
    QString getName() const;


signals:
    
public slots:

protected:

    QLabel* mColorLbl;
    QLabel* mIdLbl;
    QLabel* mNameLbl;
    QLineEdit* mEditName;

    void setConfigurable();
    virtual void mousePressEvent(QMouseEvent* event);

    virtual void doLayout();
    virtual int calcMinimumWidth() = 0;
    QRect infoContentRect();


protected slots:
    virtual void configure(QWidget* parent) {(void)parent;/* nothing to configure by default */}

private slots:
    void nameEdited();
    void configure();

private:

    QPushButton *mDisableBtn;
    QPushButton *mConfigureBtn;



    
};

#endif // UISIMPLEABSTRACTSIGNAL_H
