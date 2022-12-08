#include "MTrace.h"
#include "MinosRPC.h"
#include "RPCCommandConstants.h"
#include "MinosTestImport.h"
#include "monitoredstation.h"
#include "MonitoredContestLog.h"
#include "MonitoredLog.h"


bool MonitoredLogCmp::operator()(QSharedPointer<MonitoredLog> s1) const
{
    return s1->getPublishedName().compare(cmpstr, Qt::CaseInsensitive ) == 0;
}

MonitoredLog::MonitoredLog(MonitoredStation *s) : QObject()
  , station(s)
{
}
MonitoredLog::~MonitoredLog()
{
   mt->endImportTest();
   delete mt;
   mt = nullptr;
   delete contest;
   contest = nullptr;
}
void MonitoredLog::initialise(const QString &prouter, const QString &name )
{
    MinosRPC *rpc = MinosRPC::validMinosRPC();

    connect(rpc, &MinosRPC::routerCall, this, &MonitoredLog::on_routerCall);

   publishedName = name;
   router = prouter;

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
    setEnabled(true);
    //lastScannedStanza = -1;
}
void MonitoredLog::getLogStanza( int stanza )
{
    inStanzaRequest = QDateTime::currentMSecsSinceEpoch();

    int stanzaCount = expectedStanzaCount - stanza + 1;

    stanzaCount = std::min(stanzaCount, 10);

    // and here we want to start getting the log from the remote logger
    RPCGeneralClient rpc(rpcConstants::loggerStanzaRequest);
    QSharedPointer<RPCParam>st(new RPCParamStruct);
    st->addMember( publishedName, "LogName" );
    st->addMember( stanza, "Stanza" );
    st->addMember( stanzaCount, "Count" );
    rpc.getCallArgs() ->addParam( st );
    rpc.queueCall( rpcConstants::loggerApp + "@" + router );
}

bool MonitoredLog::getManualClose() const
{
    return manualClose;
}

void MonitoredLog::setManualClose(bool newManualClose)
{
    manualClose = newManualClose;
}

QString MonitoredLog::getDisplayName() const
{
    return displayName;
}

void MonitoredLog::setDisplayName(const QString &value)
{
    displayName = value;
}

void MonitoredLog::setStartEnd(QString s, QString e)
{
    startTime = CanonicalToTDT(s);
    endTime = CanonicalToTDT(e);
}

bool MonitoredLog::testAutoStart()
{
/*

    if contest is not protected (if it is we won't be told of it)
    and "now" date same or +1 from contest, autostart

    BUT if we have manually stopped monitor, don't restart it

    If you don't want monitoring of an "old" contest, protect it!

    We never STOP monitor automatically

    This should cater for all multi-band VHF contests, including
    VHFNFD where 50 and 70 are split across two days

*/
    if (startTime.isValid() && endTime.isValid())
    {
        QDateTime tnow = QDateTime::currentDateTimeUtc();
        QDateTime epoch = QDateTime::fromMSecsSinceEpoch(0);
        int sday = epoch.daysTo(startTime);
        int eday = epoch.daysTo(endTime);
        int tday = epoch.daysTo(tnow);

        if (!enabled() && !getManualClose() && tday >= sday && (eday >= tday || eday + 1 >= tday))
        {
            trace("Start trace by time");
            return true;
        }
    }
    return false;
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
   if ( enabled() && ( inStanzaRequest == 0 || ( tick - inStanzaRequest > 10000 ) ) )
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

      // This is what slows it down most...
      //although QSO scanning gets slower as we go on


   }
}
void MonitoredLog::on_routerCall(bool err, QSharedPointer<MinosRPCObj> mro, const QString from )
{
    trace( "MonitoredLog router callback from " + from + ( err ? ":Error" : ":Normal" ) );
    if ( !err )
    {
        // This will return stanza id, pubname, and stanza content
        QString call = mro->getMethodName();
        if (call == rpcConstants::loggerStanzaResponse)
        {

            QSharedPointer<RPCParam> psLogName;
            QSharedPointer<RPCParam> psStanzaData;
            QSharedPointer<RPCParam> psStanza;
            QSharedPointer<RPCParam> psResult;
            RPCArgs *args = mro->getCallArgs();
            if ( args->getStructArgMember( 0, "LogName", psLogName )
                 && args->getStructArgMember( 0, "LoggerResult", psResult )
                 && args->getStructArgMember( 0, "Stanza", psStanza )
                 && args->getStructArgMember( 0, "StanzaData", psStanzaData )
                 )
            {
                QString logName;
                QString stanzaData;
                bool result;
                int stanza;

                if ( psLogName->getString( logName ) && psStanzaData->getString( stanzaData )
                     && psStanza->getInt( stanza ) && psResult->getBoolean( result )
                     )
                {
                    trace( "Name " + logName + " stanza " + QString::number( stanza ) );
                    // Find the matching MonitoredLog and send the stanza their for processing

                    QStringList sl = from.split('@');
                    if (sl.count() != 2)
                    {
                        return;
                    }
                    QString ss = sl[1] + "/" + sl[0] + "/xxx";
                    Provider p(ss);

                    for ( auto const &l: qAsConst(station->slotList) )
                    {
                        if (l && l->getPublishedName() == logName )
                        {
                            emit newStanzas(this);

                            trace( "||" + stanzaData + "||" );
                            l ->processLogStanza( stanza, stanzaData );

                            BaseContestLog *contest = l->getContest();
                            if (contest->lastInserted >= 0)
                            {
                                if ( contest->lastInserted == contest->ctList.count() - 1)
                                {
                                    // new last contact; import will have checked it
                                    emit newLastContact(this);
                                }
                                else
                                {
                                    // change to a contact; we need a full rescan to understand it
                                    emit contactChanged(this);
                                }
                            }
                            return ;
                        }
                    }
                }
            }
        }
    }
}
