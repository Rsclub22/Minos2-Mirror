#ifndef MONITORPARAMETERS_H
#define MONITORPARAMETERS_H
#include "base_pch.h"

class KSTMinosParameters : public MinosParametersAdapter
{
   public:
      KSTMinosParameters()
      {}
      ~KSTMinosParameters() override
      {}
      virtual bool getAllowLoc4() override;
      virtual bool getAllowLoc8() override;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) override;
      virtual void mshowMessage(const QString &mess, QWidget* Owner = nullptr ) override;
};

#endif // MONITORPARAMETERS_H
