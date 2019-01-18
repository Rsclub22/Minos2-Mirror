/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//
// COPYRIGHT         (c) M. J. Goodey G0GJV 2005 - 2008
//
/////////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------

#ifndef MinosTestExportH
#define MinosTestExportH 
#include "base_pch.h"

//---------------------------------------------------------------------------

class RPCParam;
class RPCParamStruct;
class RPCRequest;
class MinosTestExport
{
   private:
      LoggerContestLog *ct;

      int exp_stanzaCount;
      void makeHeader( RPCParamStruct * st, unsigned long ts );
      void sendRequest( QSharedPointer<QFile> expfd, const QString &cmd, RPCParamStruct *st );
      void exportMode( QSharedPointer<QFile> expfd );
      void exportContest(QSharedPointer<QFile> expfd );
      void exportQTH( QSharedPointer<QFile> expfd );
      void exportEntry(QSharedPointer<QFile> expfd );
      void exportStation( QSharedPointer<QFile> expfd );
      void exportCurrent(QSharedPointer<QFile> expfd );
      void exportOperators( QSharedPointer<QFile> expfd );
      void exportApps(QSharedPointer<QFile> expfd);
      void exportBundles(QSharedPointer<QFile> expfd );
      void exportComment( QSharedPointer<QFile> expfd, const QSharedPointer<BaseContact> lct );
      void exportRigMemory(QSharedPointer<QFile> expfd, int memno );
      void exportRunMemory(QSharedPointer<QFile> expfd, int memno );
      int exportStackDisplay(QSharedPointer<QFile> expfd);
      void exportAllMemories(QSharedPointer<QFile> expfd);
      void exportClusterFilter(QSharedPointer<QFile> expfd);
      void exportAllClusterFilters(QSharedPointer<QFile> expfd );
   public:
      MinosTestExport( LoggerContestLog * const ct );
      MinosTestExport( );
      ~MinosTestExport();

      int exportAllDetails( QSharedPointer<QFile> expfd, bool newfile );
      int exportTest(QSharedPointer<QFile> expfd, int mindump, int maxdump );

      int exportQSO( QSharedPointer<QFile> expfd, const QSharedPointer<BaseContact>lct );


};
#endif
