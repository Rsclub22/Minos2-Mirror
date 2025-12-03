/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      BearingLineEdit
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
//
//
//
/////////////////////////////////////////////////////////////////////////////




#include "bearinglineedit.h"

/*

// change to QPalette to prevent interfering with disable widget setting

BearingLineEdit::BearingLineEdit(QWidget *parent)
    : QLineEdit(parent), bearingValid(false)
{
    connect(this, &QLineEdit::textChanged, this, &BearingLineEdit::onTextChanged);
}

void BearingLineEdit::onTextChanged(const QString &brg)
{
    if (!isEnabled())
    {
        // Skip validation and visual changes if the widget is disabled
        return;
    }

    bearingValid = false;
    QString bearingStr = brg;


    if (!bearingStr.isEmpty())
    {

        QRegularExpression re(R"(^\d*$)"); // Match digits only
        QRegularExpressionMatch rem = re.match(bearingStr.trimmed());
        if (rem.hasMatch())
        {
            int bearing = bearingStr.trimmed().toInt();
            if (bearing >= minBearing && bearing <= maxBearing)
            {
                bearingValid = true;
            }
        }
    }

    // Update the visual state based on validity
    showBearingGoodBad(bearingValid);
}

void BearingLineEdit::showBearingGoodBad(bool state)
{

    if (!isEnabled())
    {
        // Reset to the default disabled palette
        setPalette(QPalette());
        return;
    }

    QPalette palette = this->palette(); // Get the current palette

    if (state)
    {
        // Good bearing: Black text on a white background
        palette.setColor(QPalette::Base, Qt::white);    // Background color
        palette.setColor(QPalette::Text, Qt::black);   // Text color
    } else
    {
        // Bad bearing: Red text on a white background
        palette.setColor(QPalette::Base, Qt::white);    // Background color
        palette.setColor(QPalette::Text, Qt::red);      // Text color
    }

    // Apply the updated palette
    setPalette(palette);
}

bool BearingLineEdit::isValid() const
{
    return bearingValid;
}

void BearingLineEdit::setMaxMinBearing(int maxBrg, int minBrg)
{
    maxBearing = maxBrg;
    minBearing = minBrg;
}

int BearingLineEdit::getBearing() const
{
    QString bearingStr = text().trimmed();
    return bearingStr.toInt();
}

*/



BearingLineEdit::BearingLineEdit(QWidget *parent)
    : QLineEdit(parent), bearingValid(false)
{
    connect(this, &QLineEdit::textChanged, this, &BearingLineEdit::onTextChanged);
}

void BearingLineEdit::onTextChanged(const QString &brg)
{
    if (!isEnabled())
    {
        // Skip validation and visual changes if the widget is disabled
        return;
    }

    bearingValid = false;
    QString bearingStr = brg;

    if (!bearingStr.isEmpty())
    {
        static QRegularExpression re(R"(^\d*$)"); // Match digits only
        QRegularExpressionMatch rem = re.match(bearingStr.trimmed());
        if (rem.hasMatch())
        {
            int bearing = bearingStr.trimmed().toInt();
            if (bearing >= minBearing && bearing <= maxBearing)
            {
                bearingValid = true;
            }
        }
    }

    // Update the visual state based on validity
    showBearingGoodBad(bearingValid);
}

void BearingLineEdit::showBearingGoodBad(bool state)
{
    if (!isEnabled())
    {
        // Do not modify the palette if the widget is disabled
        return;
    }

    QPalette palette = this->palette(); // Get the current palette

    if (state)
    {
        // Good bearing: Black text on a white background
        palette.setColor(QPalette::Base, Qt::white);    // Background color
        palette.setColor(QPalette::Text, Qt::black);   // Text color
    }
    else
    {
        // Bad bearing: Red text on a white background
        palette.setColor(QPalette::Base, Qt::white);    // Background color
        palette.setColor(QPalette::Text, Qt::red);      // Text color
    }

    // Apply the updated palette
    setPalette(palette);
}

bool BearingLineEdit::isValid() const
{
    return bearingValid;
}

void BearingLineEdit::setMaxMinBearing(int maxBrg, int minBrg)
{
    maxBearing = maxBrg;
    minBearing = minBrg;
}

int BearingLineEdit::getBearing() const
{
    QString bearingStr = text().trimmed();
    return bearingStr.toInt();
}

void BearingLineEdit::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::EnabledChange)
    {
        if (!isEnabled())
        {
            // Reset the palette to the default when disabled
            setPalette(QPalette());
        }
        else
        {
            // Reapply the custom palette based on current validity
            showBearingGoodBad(bearingValid);
        }
    }

    QLineEdit::changeEvent(event); // Call base class implementation
}



