#ifndef CWRIGKEYERVALIDATOR_H
#define CWRIGKEYERVALIDATOR_H

#include <QValidator>

class CWRigKeyerValidator:public QValidator
{
    Q_OBJECT

public:
    CWRigKeyerValidator(QObject* parent = nullptr);
    virtual ~CWRigKeyerValidator() override
    {}

    QValidator::State validate(QString & input, int & /*pos*/) const override;

    void setValidCwCharStr(const QString cwValidCharStr_);
    void setMaxNumCwChars(int maxNumChars_);


private:

    QString validCwCharStr;
    int maxNumChars = 0;


};


#endif // CWRIGKEYERVALIDATOR_H
