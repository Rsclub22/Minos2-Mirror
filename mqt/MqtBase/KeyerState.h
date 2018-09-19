#ifndef KEYERSTATE_H
#define KEYERSTATE_H
#include "base_pch.h"

const QString KeyerStateType("KeyerState");

class KeyerState: public PubSubValue
{
    QString _report;
    QString _selected;
public:
    KeyerState(QString s);
    KeyerState(QString st, QString sel):PubSubValue(KeyerStateType), _report(st), _selected(sel){}
    virtual ~KeyerState() override;

    virtual QString pack() const override;
    virtual void unpack(QString) override;
    QString report() const;
    QString selected() const;
};

#endif // KEYERSTATE_H
