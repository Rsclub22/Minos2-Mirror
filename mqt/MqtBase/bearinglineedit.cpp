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

// we could remove the remove char code as it is now removed before display...

BearingLineEdit::BearingLineEdit(QWidget * parent): QLineEdit (parent),
    bearingValid(false)
{

    setValidator(&ucValidator);
    connect(this, &QLineEdit::textChanged, this, &BearingLineEdit::onTextChanged);


}


void BearingLineEdit::onTextChanged(const QString& brg)
{

    bearingValid = false;
    QString bearingStr = brg;
    bearingStr = bearingStr.remove(DEGREE_SYMBOL, Qt::CaseInsensitive).remove(BEARING_TRUE_CHAR).remove(SHORTLOC_DELIMITER_START).remove(SHORTLOC_DELIMITER_END);

    if (!bearingStr.isEmpty())
    {
        QRegularExpression re = QRegularExpression(anchoredPattern("\\d*"));  // match ghz_mhz.khz_hz
        QRegularExpressionMatch rem = re.match(bearingStr.trimmed());
        if (rem.hasMatch())
        {
            // all digits
            int bearing = bearingStr.trimmed().toInt();
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

