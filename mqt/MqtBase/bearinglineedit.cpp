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


#include "bearinglineedit.h"

BearingLineEdit::BearingLineEdit(QWidget * parent): QLineEdit (parent),
    bearingValid(false)
{

    setValidator(new UpperCaseValidator());
    connect(this, SIGNAL(textChanged(const QString& )), this, SLOT(onTextChanged(const QString&)));
    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));


}


void BearingLineEdit::onTextChanged(const QString& brg)
{
    if (!brg.isEmpty())
    {
        if (brg <= COMPASS_MIN0 && brg >= COMPASS_MAX360)
        {
            bearingValid = false;
            showBearingGoodBad(bearingValid);

        }
        else
        {
            bearingValid = true;
            showBearingGoodBad(bearingValid);
        }
    }
    else
    {
        bearingValid = true;
        showBearingGoodBad(bearingValid);
    }
}


void BearingLineEdit::onEditingFinished()
{
    bearing = text().trimmed();
    if (!bearing.isEmpty())
    {

       emit bearingFinished(bearing);

    }


}


void BearingLineEdit::showBearingGoodBad(bool state)
{
    if (state)
    {
        setStyleSheet(BearingLineEditFrBlackBkWhite);
    }
    else
    {
        setStyleSheet(BearingLineEditFrRedBkWhite);
    }
}



bool BearingLineEdit::isValid()
{
    return bearingValid;
}

QString BearingLineEdit::getBearing()
{
    bearing = text().trimmed();
    return bearing;
}

void BearingLineEdit::setBearing(const QString brg)
{
    if (brg >= COMPASS_MIN0 && brg <= COMPASS_MAX360)
        setText(brg);
}

