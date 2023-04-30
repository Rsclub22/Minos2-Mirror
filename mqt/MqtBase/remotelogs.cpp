#include <QSettings>

#include "remotelogs.h"
#include "MTrace.h"
#include "MinosRPC.h"
#include "monitoredstation.h"
#include "MonitoredLog.h"
#include "RPCCommandConstants.h"

QString RemoteLogs::settingsFile;
RemoteLogs::RemoteLogs()
{
    MinosRPC *rpc = MinosRPC::validMinosRPC();
    connect(rpc, &MinosRPC::notify, this, &RemoteLogs::on_notify);
    connect(rpc, &MinosRPC::provider, this, &RemoteLogs::on_provider);

    QStringList sv = {rpcConstants::currentLogCategory};
    rpc->findProviders(rpcConstants::monitorLogCategory, sv);

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
void RemoteLogs::testAutoStart()
{
    QString fname = RemoteLogs::getSettingsFile();
    QSettings settings(fname, QSettings::IniFormat);
    QString autoSyncStations = settings.value("autoStations").toString();

    QStringList autoStations;

    if (autoSyncStations != "None")
    {
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
        autoStations = autoSyncStations.split(";", Qt::SkipEmptyParts);
#else
        autoStations = autoSyncStations.split(";", QString::SkipEmptyParts);
#endif
    }

    for ( auto const &s: qAsConst(RemoteLogs::getRemoteLogs()->stationList) )
    {
        // if never set, autostations will be empty, so take all
        if (autoStations.size() == 0 || autoStations.contains(s->name))
        {
            for ( auto &ml: s->slotList )
            {
                if (ml->testAutoStart())
                {
                    emit logAutoStarted(ml);
                }
            }
        }
    }

}
bool RemoteLogs::hasWorked(const Callsign &cs, QString band, QString mode)
{
    for ( auto const &s: qAsConst(stationList) )
    {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (QVector< QSharedPointer<MonitoredLog> >::const_iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#else
        for (QVector< QSharedPointer<MonitoredLog> >::iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#endif
       {
            if ((*l)->enabled() && (*l)->getState() != psRevoked && s->currentLog == *l)
            {
                BaseContact cc((*l)->getContest(), dtg(true));
                cc.cs = cs;
                cc.band = band;
                cc.mode.setValue(mode);
                DupContact dc(&cc);

                bool worked = (*l)->getCallsigns().contains(dc);
                if (worked)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
void RemoteLogs::traceCS()
{
    for ( auto const &s: qAsConst(stationList) )
    {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (QVector< QSharedPointer<MonitoredLog> >::const_iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#else
        for (QVector< QSharedPointer<MonitoredLog> >::iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#endif
       {
            if ((*l)->enabled() && (*l)->getState() != psRevoked && s->currentLog == *l)
            {
                for (const auto &cs:qAsConst((*l)->getCallsigns()))
                {
                    trace(cs.dct->cs.getFullCall());
                }
            }
        }
    }

}
Callsign RemoteLogs::myCall()
{
    for ( auto const &s: qAsConst(stationList) )
    {

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        for (QVector< QSharedPointer<MonitoredLog> >::const_iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#else
        for (QVector< QSharedPointer<MonitoredLog> >::iterator l = s->slotList.begin(); l != s->slotList.end(); l++)
#endif
       {
            if ((*l)->enabled() && (*l)->getState() != psRevoked && s->currentLog == *l)
            {
                BaseContestLog *ct = (*l)->getContest();
                Callsign mcall = ct->mycall;
                //QString fcall = mcall.getFullCall();
                return mcall;
            }
        }
    }
    return Callsign();
}
void RemoteLogs::on_provider(Provider provider, QString cat)
{
    if (cat == rpcConstants::monitorLogCategory)
    {
        if (!stationList.contains(provider))
        {
            MonitoredStation *s = new MonitoredStation;
            s->name = provider.app + "@" + provider.routerName;
            stationList[provider] = s;
            emit syncNeeded();
        }
    }
}

void RemoteLogs::on_notify(AnalysePubSubNotify an, const QString /*from*/ )
{
    // pubsub notify

    //trace( "RemoteLogs on_notify from " + from + ( !an.getOK() ? ":Error" : ":Normal" ) );

    if ( an.getOK() )
    {
        PublishState state = an.getState();
        QString key = an.getKey();          // key is minos file name
        QString value = an.getValue();      // value is stanzacount;[band] name;start time;end time
        QString router = an.getPublisherRouter();
        if ( router.size() == 0 )
        {
            // it is for us...
            router = localRouterName;
        }

        QString logval = router + " : " + key ;

        if ( an.getCategory() == rpcConstants::monitorLogCategory )
        {
            trace( "ContestLog " + logval + " " + value );

            MonitoredStation *stat = stationList[Provider(an)];

            if (stat)
            {
                QVector< QSharedPointer<MonitoredLog> >::iterator log = std::find_if( stat->slotList.begin(), stat->slotList.end(), MonitoredLogCmp( key ) );
                if (state == psPublished)
                {
                    QStringList args = value.split(";");
                    if ( log == stat->slotList.end() )
                    {
                        QSharedPointer<MonitoredLog> ml(new MonitoredLog(stat));

                        ml->initialise( router, key );
                        stat->slotList.push_back( ml );
                        log = std::find_if( stat->slotList.begin(), stat->slotList.end(), MonitoredLogCmp( key ) );

                        emit newMonitoredLog(ml);

                    }
                    (*log)->setState(state);
                    if (args.count() >= 4)
                    {
                        (*log)->setDisplayName(args[1]);
                        (*log)->setStartEnd(args[2], args[3]);
                    }
                    else if (args.count() >= 2)
                    {
                        (*log)->setDisplayName(args[1]);
                    }
                    if (args.count() >= 1)
                    {
                        (*log)->setExpectedStanzaCount( args[0].toInt() );
                    }
                    else
                    {
                        (*log)->setDisplayName(key);
                    }

                    if (args.count() >= 1)
                    {
                        (*log)->setExpectedStanzaCount( args[0].toInt() );
                    }

                    emit syncNeeded();
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
        else if (an.getCategory() == rpcConstants::currentLogCategory)
        {
            MonitoredStation *stat = stationList[Provider(an)];

            if (stat)
            {
                QVector< QSharedPointer<MonitoredLog> >::iterator log = std::find_if( stat->slotList.begin(), stat->slotList.end(), MonitoredLogCmp( value ) );
                if (log != stat->slotList.end())
                {
                    // this is now the current log
                    stat->currentLog = *log;
                    emit currentLogChanged(stat->currentLog);
                }
            }
        }
    }
}
