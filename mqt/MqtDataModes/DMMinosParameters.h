#ifndef DMMINOSPARAMETERS_H
#define DMMINOSPARAMETERS_H

#include "MinosParameters.h"

class DMMinosParameters : public MinosParametersAdapter
{
   public:
      DMMinosParameters()
      {}
      ~DMMinosParameters() override
      {}
      virtual bool getAllowLoc4() override;
      virtual bool getAllowLoc8() override;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) override;
      virtual void mshowMessage(const QString &mess, QWidget* Owner = nullptr ) override;
};

#endif // DMMINOSPARAMETERS_H
