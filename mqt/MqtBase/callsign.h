#ifndef CALLSIGN_H
#define CALLSIGN_H

#include "XMPP_pch.h"
#include "minositem.h"

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
      // eg for <pe/f0ctt/mm> (g0gjv/p) [F6CTT/RVI/P]
      QString locCtryPrefix; // <pe> (g) [/RVI]country of location
      QString dupPrefix;  // <f> (g) [F]country of issue
      QString number; // <0> (0) [6]numeric part
      QString body;  // <ctt> (gjv) [CTT]main body
      QString suffix; // <mm> (p) [P]trailer

      QString realCall; // the actual calsign, stripped of decorations
      QString wpxPrefix;

      Callsign( );
      ~Callsign();
      bool operator==( const Callsign& rhs ) const;
      bool operator!=( const Callsign& rhs ) const
      {
          return !(rhs == *this);
      }
      bool operator<( const Callsign& rhs ) const;
//      Callsign& operator = ( const Callsign& );


      bool isUK() const;

      int getValRes() const
      {
          return valRes;
      }
      void setValRes(int c)
      {
          valRes = c;
      }
      void clearValRes()
      {
          valRes = CS_NOT_VALIDATED;
      }

      int setFullCall(const QString &pcs);
      QString getFullCall() const
      {
          return fullCall.getValue();
      }
      void setDirty()
      {
          fullCall.setDirty();
      }
      void clearDirty()
      {
          fullCall.clearDirty();
      }
      bool isDirty() const
      {
          return fullCall.isDirty();
      }
      void addIfDirty(RPCParamStruct *st, const QString &stname, bool &d)
      {
          fullCall.addIfDirty(st, stname, d);
      }

private:
      int valRes = CS_NOT_VALIDATED;   // current validation result
      MinosStringItem<QString> fullCall; // full call

      int validate( );
      bool isValidStructure();
      static bool isValidStructure( const QString &prefix,  const QString &number,  const QString &body );
};

#endif // CALLSIGN_H
