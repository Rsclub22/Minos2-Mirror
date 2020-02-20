/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef MinosTestImportH
#define MinosTestImportH 
#include "base_pch.h"
//---------------------------------------------------------------------------
class MinosTestImport: public RPCDispatcher
{
   protected:
      typedef void ( MinosTestImport::* DispatchCallback ) ( XStanza *stanza ) ;
      DispatchCallback dispatchCallback;

      virtual void dispatchResponse( XStanza *a ) override;
      void analyseImportLog( XStanza * );
      void analyseImportTest( XStanza * );
   private:
      BaseContestLog *ct;
      int curfpos;

      QSharedPointer<RPCParam> body;

      int imp_stanzaCount;

      bool getStructArgMemberValueDTG( QSharedPointer<RPCParam> rpm, const QString &name, QString &val );
      bool getStructArgMemberValueDTG( QSharedPointer<RPCParam>rpm, const QString &name, MinosStringItem<QString> &val );
      bool getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, QString &val );
      bool getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, MinosStringItem<QString> &val );
      bool getStructArgMemberValue(QSharedPointer<RPCParam>rpm, const QString &name, int &val );
      bool getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, MinosItem<int> &val );
      bool getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, bool &val );
      bool getStructArgMemberValue( QSharedPointer<RPCParam>rpm, const QString &name, MinosItem<bool> &val );
   public:
      void processMinosStanza( RPCRequest *xs );
      void processLogMinosStanza( RPCRequest *xs );
      MinosTestImport( BaseContestLog * const ct );
      MinosTestImport( );
      virtual ~MinosTestImport() override;

      void startImportTest();
      int importTestBuffer(const QByteArray &buffer );
      void endImportTest();

      int importTest( QSharedPointer<QFile> ctfile );
      bool analyseTest( QSharedPointer<QFile> ctfile );
      int readTestFile( QSharedPointer<QFile> ctfile );

      bool getStructArgMemberValueDTG( const QString &name, QString &val );
      bool getStructArgMemberValueDTG( const QString &name, MinosStringItem<QString> &val );
      bool getStructArgMemberValue( const QString &name, QString &val );
      bool getStructArgMemberValue( const QString &name, MinosStringItem<QString> &val );
      bool getStructArgMemberValue( const QString &name, int &val );
      bool getStructArgMemberValue( const QString &name, MinosItem<int> &val );
      bool getStructArgMemberValue( const QString &name, bool &val );
      bool getStructArgMemberValue( const QString &name, MinosItem<bool> &val );

      void setBit( unsigned short &cf, unsigned short bit, bool set
                    );
};
#endif
