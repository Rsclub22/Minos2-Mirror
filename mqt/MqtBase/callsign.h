#ifndef CALLSIGN_H
#define CALLSIGN_H

#include "XMPP_pch.h"

#define CS_OK 0
#define CS_NOT_VALIDATED -1
#define ERR_NOCS 21
#define ERR_CSDIGIT 22
#define ERR_INVCS 23
#define ERR_DUPCS 24

class Callsign
{
   #define BITLENGTH 4
    #define NUMBITLENGTH 4
    #define TRAILBITLENGTH 3
   public:
      MinosStringItem<QString> fullCall; // full call
      // eg for <pe/f0ctt/mm> (g0gjv/p) [F6CTT/RVI/P]
      QString locCtryPrefix; // <pe> (g) [/RVI]country of location
      QString dupPrefix;  // <f> (g) [F]country of issue
      QString number; // <0> (0) [6]numeric part
      QString body;  // <ctt> (gjv) [CTT]main body
      QString suffix; // <mm> (p) [P]trailer

      QString realCall; // the actual calsign, stripped of decorations
      QString wpxPrefix;
      int valRes;   // current validation result

      Callsign( );
      Callsign( const QString &orig );
      ~Callsign();
      bool operator==( const Callsign& rhs ) const;
      bool operator!=( const Callsign& rhs ) const
      {
          return !(rhs == *this);
      }
      bool operator<( const Callsign& rhs ) const;
      Callsign& operator = ( const Callsign& );


      int validate( );
      bool isUK() const;
private:
      bool isValidStructure();
      static bool isValidStructure( const QString &prefix,  const QString &number,  const QString &body );
};

#endif // CALLSIGN_H
