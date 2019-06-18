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
#include "locatorlineedit.h"

LocatorLineEdit::LocatorLineEdit(QWidget * parent) : QLineEdit (parent),
    locatorValid(false)
{
    setValidator(new UpperCaseValidator());
    connect(this, SIGNAL(textChanged(const QString& )), this, SLOT(onTextChanged(const QString&)));
    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

}




void LocatorLineEdit::onTextChanged(const QString& locator)
{

    if (!locator.isEmpty())
    {
        double latitude;
        double longitude;

        if (lonlat(locator, longitude, latitude, false) == LOC_OK)
        {
            locatorValid = true;
        }
        else
        {
            locatorValid = false;
        }
        showLocatorGoodBad(locatorValid);
    }
    else
    {
        locatorValid = true;
        showLocatorGoodBad(locatorValid);
    }


}


void LocatorLineEdit::onEditingFinished()
{
    locator = text().trimmed();
    if (!locator.isEmpty())
    {

       emit locatorFinished(locator);

    }


}



void LocatorLineEdit::showLocatorGoodBad(bool state)
{
    if (state)
    {
        setStyleSheet(LocatorLineEditFrBlackBkWhite);
    }
    else
    {
        setStyleSheet(LocatorLineEditFrRedBkWhite);
    }
}



bool LocatorLineEdit::isValid()
{
    return locatorValid;
}


QString LocatorLineEdit::getLocator()
{
    return locator;
}

void LocatorLineEdit::setLocator(const QString loc)
{
    setText(loc);
}
