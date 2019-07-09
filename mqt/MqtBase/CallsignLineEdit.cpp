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






#include "CallsignLineEdit.h"




CallsignLineEdit::CallsignLineEdit(QWidget * parent): QLineEdit (parent),
    callsignValid(false)
{

    setValidator(new UpperCaseValidator());
    connect(this, SIGNAL(textChanged(const QString& )), this, SLOT(onTextChanged(const QString&)));
    connect(this, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));

}




void CallsignLineEdit::onTextChanged(const QString& callsign)
{
    if (!callsign.isEmpty())
    {
        Callsign cs(callsign);
        if (cs.validate() != CS_OK)
        {
            callsignValid = false;
            showCallsignGoodBad(callsignValid);

        }
        else
        {
            callsignValid = true;
            showCallsignGoodBad(callsignValid);
        }
    }
    else
    {
        callsignValid = true;
        showCallsignGoodBad(callsignValid);
    }
}


void CallsignLineEdit::onEditingFinished()
{
    callsign = text().trimmed();
    if (!callsign.isEmpty())
    {

       emit callsignFinished(callsign);

    }


}



void CallsignLineEdit::showCallsignGoodBad(bool state)
{
    if (state)
    {
        setStyleSheet(CallsignLineEditFrBlackBkWhite);
    }
    else
    {
        setStyleSheet(CallsignLineEditFrRedBkWhite);
    }
}



bool CallsignLineEdit::isValid()
{
    return callsignValid;
}

QString CallsignLineEdit::getCallsign()
{
    callsign = text().trimmed();
    return callsign;
}

void CallsignLineEdit::setCallsign(const QString call)
{
    setText(call);
}
