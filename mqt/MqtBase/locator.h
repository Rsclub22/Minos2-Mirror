#ifndef LOCATOR_H
#define LOCATOR_H

#include "XMPP_pch.h"
#include "minositem.h"

#define LOC_NOT_VALIDATED -1
#define LOC_OK 0
#define LOC_SHORT 1
#define LOC_PARTIAL 2
#define ERR_NOLOC 30
#define ERR_LOC_RANGE 31

class Locator
{
      MinosStringItem<QString> loc;
      int valRes = LOC_NOT_VALIDATED;
      int validate( );
   public:
      Locator();
      //Locator(const QString &);
      Locator(const Locator&);
      Locator& operator =(const Locator&);
      ~Locator();
      int validate( double &lon, double &lat );
      int reValidate()
      {
          valRes = LOC_NOT_VALIDATED;
          validate();
          return valRes;
      }

    char getValRes() const
    {
        return valRes;
    }
    void setValRes(int vr)
    {
        valRes = vr;
    }
    void clearValRes()
    {
        valRes = LOC_NOT_VALIDATED;
    }

    int setLoc(const QString &l)
    {
        loc.setValue(l.trimmed().toUpper());
        reValidate();
        return valRes;
    }
    QString getLoc() const
    {
        return loc.getValue();
    }
    void setDirty()
    {
        loc.setDirty();
    }
    void clearDirty()
    {
        loc.clearDirty();
    }
    bool isDirty() const
    {
        return loc.isDirty();
    }
    void addIfDirty(RPCParamStruct *st, const QString &stname, bool &d)
    {
        loc.addIfDirty(st, stname, d);
    }

};

#endif // LOCATOR_H
