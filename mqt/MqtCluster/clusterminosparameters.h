#ifndef CLUSTERMINOSPARAMETERS_H
#define CLUSTERMINOSPARAMETERS_H

#include "base_pch.h"

class ClusterMinosParameters : public MinosParametersAdapter
{
   public:
      ClusterMinosParameters()
      {}
      ~ClusterMinosParameters() override
      {}
      virtual bool getAllowLoc4() override;
      virtual bool getAllowLoc8() override;
      virtual bool yesNoMessage( QWidget* Owner, const QString &mess ) override;
      virtual void mshowMessage(const QString &mess, QWidget* Owner = nullptr ) override;
};

#endif // CLUSTERMINOSPARAMETERS_H
