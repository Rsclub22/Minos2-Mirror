/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef TinyUtilsH
#define TinyUtilsH

#include <QString>

class TiXmlElement;
extern bool checkElementName( TiXmlElement *e, const QString &expected );
extern QString getAttribute( TiXmlElement *tix, const QString &attname );
extern int GetStringAttribute( TiXmlElement *e, const QString &name, QString &s );
extern int GetStringAttribute( TiXmlElement *e, const QString &name, QString &s, const QString &def );
extern int GetIntAttribute( TiXmlElement *e, const QString &name, int &i );
extern int GetIntAttribute( TiXmlElement *e, const QString &name, int &i, int def );
extern int GetBoolAttribute( TiXmlElement *e, const QString &name, bool &b );
extern int GetBoolAttribute( TiXmlElement *e, const QString &name, bool &b, bool def );

#endif
