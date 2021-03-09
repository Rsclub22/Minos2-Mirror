#ifndef QRZSERVERMINOSPARAMETERS_H
#define QRZSERVERMINOSPARAMETERS_H

#include "base_pch.h"

class QrzServerMinosParameters : public MinosParametersAdapter
{
public:
    QrzServerMinosParameters()
    {}
    ~QrzServerMinosParameters() override
    {}

    virtual bool getAllowLoc4() override;
    virtual bool getAllowLoc8() override;
};

#endif // QRZSERVERMINOSPARAMETERS_H
