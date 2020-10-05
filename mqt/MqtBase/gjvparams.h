#ifndef GJVPARAMS_H
#define GJVPARAMS_H
#include "XMPP_pch.h"

#define GJVVERSION "0.41"
#define VERLENGTH (4)

#define DATELENGTH 8
#define TIMELENGTH 5
#define CALLSIGNLENGTH 15
#define LOCLENGTH 8
#define COMMENTLENGTH 27
#define EXTRALENGTH 50
//#define RSTLENGTH 3
#define SERIALLENGTH 4

//#define INISECTLENGTH 40
// control block for repeated actions on contest contacts
class GJVParams
{
   public:
      int diskBlock;
      QSharedPointer<QFile> fd;
      int count;

      GJVParams( QSharedPointer<QFile> f );
      ~GJVParams();
};

#endif // GJVPARAMS_H
