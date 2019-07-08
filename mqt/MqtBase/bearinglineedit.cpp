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


}


void BearingLineEdit::onTextChanged(const QString& brg)
{

    bearingValid = false;

    if (!brg.isEmpty())
    {

        QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
        if (re.exactMatch(brg.trimmed()))
        {
            // all digits
            int bearing = brg.trimmed().toInt();
            if (bearing >= COMPASS_MIN0 && bearing <= COMPASS_MAX360)
            {
                bearingValid = true;
                showBearingGoodBad(bearingValid);

            }

        }

        if (!bearingValid)
        {
            bearingValid = false;
            showBearingGoodBad(bearingValid);
        }


    }
    else
    {

        showBearingGoodBad(true);
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

int BearingLineEdit::getBearing()
{

    int bearing = text().trimmed().remove(DEGREE_SYMBOL, Qt::CaseInsensitive).remove(BEARING_TRUE_CHAR).remove(SHORTLOC_DELIMITER_START).remove(SHORTLOC_DELIMITER_END).toInt();

    return bearing;
}

void BearingLineEdit::setBearing(const QString brg)
{
    bool ok = false;
    int bearing = brg.toInt(&ok);
    if (!brg.isEmpty() && ok)
    {
        if (bearing >= COMPASS_MIN0 && bearing < COMPASS_MAX360)
            setText(brg);
    }
}

