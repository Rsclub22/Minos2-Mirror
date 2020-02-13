#ifndef MATCHCOLLECTION_H
#define MATCHCOLLECTION_H
#include "base_pch.h"
#include "MatchContact.h"

class matchElement
{
   public:
      bool match;
      bool empty;
      //      bool trimmed;

      QString mstr;
      QString rawstr;

      matchElement( );
      unsigned char set
         ( const QString & );
      unsigned char checkGreater( const QString &, QString &match_temp  );
      bool checkMatch( const QString & );
};

class TMatchCollection
{
   public:
      ContestMatchList contestMatchList;
      int contactCount();
      TMatchCollection();
      ~TMatchCollection();
      int getContestCount( );
      QSharedPointer<BaseMatchContest> pcontestAt( int );
};

typedef QSharedPointer< TMatchCollection > SharedMatchCollection;
Q_DECLARE_METATYPE(SharedMatchCollection)

#endif // MATCHCOLLECTION_H
