/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      CallsignLineEdit
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef CALLSIGNLINEEDIT_H
#define CALLSIGNLINEEDIT_H

#include <QObject>
#include <QLineEdit>

#include "cutils.h"

class CallsignLineEdit : public QLineEdit
{

    Q_OBJECT

public:
    CallsignLineEdit(QWidget*);

    bool isValid();

signals:

    void textChanged(QLineEdit*, const QString&);
    void editingFinished(QLineEdit*);

    void callsignFinished(const QString&);





private slots:
    void onTextChanged(const QString &);
    void onEditingFinished();

private:


    bool callsignValid;
    QString CallsignLineEditFrRedBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : black}";
    QString CallsignLineEditFrBlackBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : black}";
    void showCallsignGoodBad(bool state);
};

#endif // CALLSIGNLINEEDIT_H
