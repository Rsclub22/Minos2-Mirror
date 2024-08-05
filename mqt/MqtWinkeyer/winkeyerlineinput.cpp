/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Winkeyer Control
//
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2024
//
// Interprocess Control Logic
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2017
//
//
//
/////////////////////////////////////////////////////////////////////////////
#include <QIntValidator>
#include <QKeyEvent>
#include "winkeyerlineinput.h"

WinkeyerLineInput::WinkeyerLineInput(QWidget *parent) : QLineEdit(parent)
{

}


void WinkeyerLineInput::keyPressEvent(QKeyEvent *event)
{
    QLineEdit::keyPressEvent(event);
    if (!validateInput() && !this->text().isEmpty())
    {

        this->undo();
    }
}

bool WinkeyerLineInput::validateInput()
{

    QString text = this->text();
    // Check if it can be converted to an integer
    bool ok;
    int value = text.toInt(&ok);
    const QIntValidator *validator = static_cast<const QIntValidator *>(this->validator());
    if (validator)
    {
        if (ok && (text.isEmpty() || (validator->bottom() <= value && value <= validator->top())))
        {
            return true;
        }
    }else
    {
        if (ok)
        {
            return true;
        }
    }

    return false;
}



void WinkeyerLineInput::setValidatorRange(int min, int max)
{
    minValue = min;
    maxValue = max;

    QIntValidator *validator = new QIntValidator(min, max, this);
    this->setValidator(validator);
}

int WinkeyerLineInput::getMinValue()
{
    return minValue;
}

int WinkeyerLineInput::getMaxValue()
{
    return maxValue;
}
