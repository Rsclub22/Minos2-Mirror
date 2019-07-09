/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      LocatorLineEdit
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2019
//
//
//
//
/////////////////////////////////////////////////////////////////////////////
#ifndef LOCATORLINEEDIT_H
#define LOCATORLINEEDIT_H

#include <QLineEdit>
#include "cutils.h"

class LocatorLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LocatorLineEdit(QWidget*);
    QString getLocator();
    bool isValid();
    void setLocator(const QString loc);
    void setAllowLoc4(bool loc4);

signals:

    void textChanged(QLineEdit*, const QString&);
    void editingFinished(QLineEdit*);

    void locatorFinished(const QString&);





private slots:
    void onTextChanged(const QString &);
    void onEditingFinished();

private:

    QString locator;
    bool locatorValid;
    bool allowLoc4;
    QString LocatorLineEditFrRedBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: red ; color : black}";
    QString LocatorLineEditFrBlackBkWhite = "QLineEdit { background-color: white ; border-style: outset ; border-width: 1px ; border-color: black ; color : black}";
    void showLocatorGoodBad(bool state);


};

#endif // LOCATORLINEEDIT_H
