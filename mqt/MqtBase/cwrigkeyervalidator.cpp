#include "cwrigkeyervalidator.h"


CWRigKeyerValidator::CWRigKeyerValidator(QObject* parent) : QValidator(parent)
{

}

QValidator::State CWRigKeyerValidator::validate(QString & input, int &/*pos*/) const
{

    input = input.toUpper();

    if (input.length() <= maxNumChars && validCwCharStr.contains(input.right(1)))
    {
        return Acceptable;
    }


    return Invalid;


}



void CWRigKeyerValidator::setValidCwCharStr(const QString cwValidCharStr_)
{
    validCwCharStr = cwValidCharStr_;
}



void CWRigKeyerValidator::setMaxNumCwChars(int maxNumChars_)
{
    maxNumChars = maxNumChars_;
}

