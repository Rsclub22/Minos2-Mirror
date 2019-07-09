/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      BearingLineEdit
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
//
/////////////////////////////////////////////////////////////////////////////


#ifndef BEARINGLINEEDIT_H
#define BEARINGLINEEDIT_H

#include <QObject>
#include <QLineEdit>
#include "cutils.h"
#include "rotatorcommon.h"

class BearingLineEdit : public QLineEdit
{

     Q_OBJECT

public:
    BearingLineEdit(QWidget*);
    int getBearing();
    void setBearing(QString);
    bool isValid();


signals:

    void textChanged(QLineEdit*, const QString&);




private slots:
    void onTextChanged(const QString &);


private:

    int bearing;
    bool bearingValid;
    QString BearingLineEditFrRedBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : black}";
    QString BearingLineEditFrBlackBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : black}";
    void showBearingGoodBad(bool state);



};

#endif // BEARINGLINEEDIT_H
