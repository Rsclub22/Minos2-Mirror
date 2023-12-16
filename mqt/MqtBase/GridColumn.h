/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef TreeUtilsH
#define TreeUtilsH

#include <QString>

enum TAlignment { taLeftJustify, taRightJustify, taCenter };

class GridColumn
{
   public:
      int fieldId;
      QString width;
      const char * title;
      TAlignment alignment;
      GridColumn( ) :
            fieldId( -1 ), title( nullptr ), alignment( taLeftJustify )
      {}
      GridColumn( int fieldId, QString width, const char * title, TAlignment alignment ) :
            fieldId( fieldId ), width( width ), title( title ), alignment( alignment )
      {}
};

#endif
