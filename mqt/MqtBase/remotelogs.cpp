#include "remotelogs.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "MonitoredLog.h"
#include "RPCCommandConstants.h"
#include "monitoredstation.h"

RemoteLogs::RemoteLogs()
{
    MinosRPC *rpc = MinosRPC::validMinosRPC();
    connect(rpc, &MinosRPC::notify, this, &RemoteLogs::on_notify);
    connect(rpc, &MinosRPC::provider, this, &RemoteLogs::on_provider);

    QStringList sv = {rpcConstants::monitorLogCategory};
    rpc->findProviders(rpcConstants::LoggerCategory, sv);

    MinosConfig *config = MinosConfig::getMinosConfig();
    localRouterName = config->getThisRouterName();
}

void RemoteLogs::closeLog(MonitoredLog *l)
{
    l->setEnabled(false);
    l->setManualClose(true);
    l->setFrame(nullptr);
}

void RemoteLogs::closeAll()
{
    stationList.clear();
}

bool RemoteLogs::hasWorked(const Callsign &cs)
{
    for ( auto const &s: qAsConst(stationList) )
    {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (QVector< QSharedPointer<MonitoredLog> >::const_iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#else
        for (QVector< QSharedPointer<MonitoredLog> >::iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#endif
       {
            if ((*l)->getState() != psRevoked)
            {
                bool worked = (*l)->getCallsigns().contains(cs);
                if (worked)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
void RemoteLogs::on_provider(Provider provider, QString /*cat*/)
{
    stationList[provider] = new MonitoredStation;
    emit syncNeeded();
}

void RemoteLogs::on_notify(AnalysePubSubNotify an, const QString from )
{
    // pubsub notify

    trace( "Notify callback from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        PublishState state = an.getState();
        QString key = an.getKey();          // key is minos file name
        QString value = an.getValue();      // value is stanzacount;[band] name;start time;end time

        if ( an.getCategory() == rpcConstants::monitorLogCategory )
        {
            QString router = an.getRouter();
            if ( router.size() == 0 )
            {
                // it is for us...
                router = localRouterName;
            }

            QString logval = router + " : " + key ;
            trace( "ContestLog " + logval + " " + value );

            MonitoredStation *stat = stationList[Provider(an)];

            QVector< QSharedPointer<MonitoredLog> >::iterator log = std::find_if( stat->slotList.begin(), stat->slotList.end(), MonitoredLogCmp( key ) );
            if (state == psPublished)
            {
                QStringList args = value.split(";");
                if ( log == stat->slotList.end() )
                {
                    QSharedPointer<MonitoredLog> ml(new MonitoredLog(stat));
                    emit newMonitoredLog(ml.data());

                    ml->initialise( router, key );

                    if (args.count() >= 4)
                    {
                        ml->setDisplayName(args[1]);
                        ml->setStartEnd(args[2], args[3]);
                    }
                    else if (args.count() >= 2)
                    {
                        ml->setDisplayName(args[1]);
                    }
                    if (args.count() >= 1)
                    {
                        ml->setExpectedStanzaCount( args[0].toInt() );
                    }
                    else
                    {
                        ml->setDisplayName(key);
                    }

                    ml->setState(state);
                    stat->slotList.push_back( ml );
                    emit syncNeeded();

                }
                else
                {
                    if (args.count() >= 1)
                    {
                        trace(QString("args 0 %1 ").arg(args[0]));
                        (*log)->setExpectedStanzaCount( args[0].toInt() );
                    }
                    (*log)->setState(state);
                }
            }
            else
            {
                if ( log != stat->slotList.end() )
                {
                    (*log)->setState(state);
                }
            }
        }
    }
}
