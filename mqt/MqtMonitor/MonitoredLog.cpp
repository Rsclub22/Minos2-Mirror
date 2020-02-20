#include "MinosTestImport.h"
#include "MonitoredContestLog.h"
#include "MonitoringFrame.h"
#include "MonitoredLog.h"

MonitoredLog::MonitoredLog()
{}
MonitoredLog::~MonitoredLog()
{
   mt->endImportTest();
   delete mt;
   mt = nullptr;
   delete contest;
   contest = nullptr;
}
void MonitoredLog::initialise( const QString &srv, const QString &name )
{
   publishedName = name;
   server = srv;

   delete contest;
   contest = new MonitoredContestLog();

   static int slotcnt = 0;
   contest->cslotno = slotcnt++;

   delete mt;
   mt = new MinosTestImport( contest );
   mt->startImportTest();
}

void MonitoredLog::startMonitor()
{
   monitorEnabled = true;
   //lastScannedStanza = -1;
}
void MonitoredLog::stopMonitor()
{
   monitorEnabled = false;

   mt->endImportTest();

   inStanzaRequest = 0;
   lastScannedStanza = -1;
   //   expectedStanzaCount = 0; // no - it is still correct as published...
   stanzasPulled.clear();

   frame = nullptr;

   initialise( server, publishedName );  // make sure reset for next time
}
void MonitoredLog::getLogStanza( int stanza )
{
    inStanzaRequest = QDateTime::currentMSecsSinceEpoch();

    // and here we want to start getting the log from the remote logger
    RPCGeneralClient rpc(rpcConstants::loggerStanzaRequest);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    QSharedPointer<RPCParam>sName(new RPCStringParam( publishedName ));
    QSharedPointer<RPCParam>iStanza(new RPCIntParam( stanza ));
    st->addMember( sName, "LogName" );
    st->addMember( iStanza, "Stanza" );
    rpc.getCallArgs() ->addParam( st );
    rpc.queueCall( rpcConstants::loggerApp + "@" + server );
}

QString MonitoredLog::getDisplayName() const
{
    return displayName;
}

void MonitoredLog::setDisplayName(const QString &value)
{
    displayName = value;
}
void MonitoredLog::checkMonitor()
{
    // here we need to check that we haven't got any gaps in the received log
    // and re-request the lot as necessary
   if ( !contest || !frame || state != psPublished )
   {
      return ;
   }
   int curCount = contest->getCtStanzaCount();
   qint64 tick = QDateTime::currentMSecsSinceEpoch();
   if ( monitorEnabled && ( inStanzaRequest == 0 || ( tick - inStanzaRequest > 10000 ) ) )
   {
      if ( expectedStanzaCount > curCount )
      {
         getLogStanza( curCount + 1 );
      }
   }
}
void MonitoredLog::processLogStanza( int stanza, const QString &stanzaData )
{
   inStanzaRequest = 0;
   if ( stanzasPulled.find( stanza ) == stanzasPulled.end() )
   {
      // we have a stanza - so pass it into the contest object
      contest->ct_stanzaCount = mt->importTestBuffer( stanzaData.toUtf8() );
      stanzasPulled.insert(stanza);

      if (frame)
          frame->update();
   }
}
