#include <QHostInfo>
#include <cmath>
#include "MinosLoggerEvents.h"
#include "ContestApp.h"
#include "cutils.h"
#include "contest.h"
#include "contacts.h"
#include "LoggerContest.h"

#include "n1mmbroadcast.h"

N1MMBroadcast::N1MMBroadcast()
{
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::afterQSOSaved, this, &N1MMBroadcast::afterQSOSaved);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::wsjtxDatagram, this, &N1MMBroadcast::wsjtxDatagram);
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::callsignLookup, this, &N1MMBroadcast::callsignLookup);

    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::broadcastSpots, this, &N1MMBroadcast::dxSpots);
}
bool N1MMBroadcast::setAddress(QString addr, QHostAddress &host)
{
    QHostInfo haddr = QHostInfo::fromName( addr );
    for (auto &a: haddr.addresses())
    {
        bool ok = true;
        quint32 iaddr;

        // This is a guess as to when the change came in
        iaddr = a.toIPv4Address(
            #if QT_VERSION > QT_VERSION_CHECK(5, 4, 0)
                    &ok
            #endif
                    );
        if (ok)
        {
            host.setAddress(iaddr);
            return true;
        }
    }
    return false;
}

void N1MMBroadcast::configure()
{
    int temp;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpcontactsSelect, contactsSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpcontactsPort, temp );
    contactsPort = static_cast<quint16>(temp);
    if (contactsAddr.isEmpty())
    {
        contactsAddr = "127.0.0.1";
    }
    if (contactsPort == 0)
    {
        contactsPort = 12060;
    }

    if (contactsSelect)
    {
        setAddress(contactsAddr, contactsHost);
    }

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpextCSSelect, extCSSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpextCSPort, temp );
    extCSPort = static_cast<quint16>(temp);
    if (extCSAddr.isEmpty())
    {
        extCSAddr = "127.0.0.1";
    }
    if (extCSPort == 0)
    {
        extCSPort = 12060;
    }
    if (extCSSelect)
    {
        setAddress(extCSAddr, extCSHost);
    }
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpextSpotsSelect, spotsSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpextSpotsAddr, spotsAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpextSpotsPort, temp );
    spotsPort = static_cast<quint16>(temp);
    if (spotsAddr.isEmpty())
    {
        spotsAddr = "127.0.0.1";
    }
    if (spotsPort == 0)
    {
        spotsPort = 12060;
    }
    if (spotsSelect)
    {
        setAddress(spotsAddr, spotsHost);
    }

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpwsjtxRbSelect, wsjtxRbSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpwsjtxRbAddr, wsjtxRbAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpwsjtxRbPort, temp );
    wsjtxRbPort = static_cast<quint16>(temp);
    if (wsjtxRbAddr.isEmpty())
    {
        wsjtxRbAddr = "127.0.0.1";
    }
    if (wsjtxRbPort == 0)
    {
        wsjtxRbPort = 12060;
    }

    if (wsjtxRbSelect)
    {
        setAddress(wsjtxRbAddr, wsjtxRbHost);
    }

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpADIFSelect, ADIFSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpADIFAddr, ADIFAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpADIFPort, temp );
    ADIFPort = static_cast<quint16>(temp);
    if (ADIFAddr.isEmpty())
    {
        ADIFAddr = "127.0.0.1";
    }
    if (ADIFPort == 0)
    {
        ADIFPort = 12060;
    }

    if (ADIFSelect)
    {
        setAddress(ADIFAddr, ADIFHost);
    }
}
void N1MMBroadcast::afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct)
{
    // This can generate modified, deleted/recreated, or straight contact
    if (contactsSelect && !contactsHost.isNull())
    {
        if (tct->getHistory().size())
        {
            // history doesn't get set up until after the QSO has ben saved,
            //and this message sent

            QSharedPointer<BaseContact> h = tct->getHistory().at(tct->getHistory().size() - 1);

            QString stanza = genDeleteStanza(h);
//            trace(stanza);
            bc.writeDatagram(stanza.toUtf8(), contactsHost, contactsPort);

            if (!(tct->contactFlags.getValue() & DONT_PRINT) && (h->contactFlags.getValue() & DONT_PRINT))
            {
                // was deleted, now isn't
                stanza = genContactStanza("contactinfo", c, tct);
//                trace(stanza);
                bc.writeDatagram(stanza.toUtf8(), contactsHost, contactsPort);
            }
            else if ((tct->contactFlags.getValue() & DONT_PRINT) && !(h->contactFlags.getValue() & DONT_PRINT))
            {
                // was not deleted, now is - just need delete
            }
            else
            {
                // ordinary edit
                stanza = genContactStanza("contactreplace", c, tct);
//                trace(stanza);
                bc.writeDatagram(stanza.toUtf8(), contactsHost, contactsPort);
            }
        }
        else
        {
            QString stanza = genContactStanza("contactinfo", c, tct);
//            trace(stanza);
            bc.writeDatagram(stanza.toUtf8(), contactsHost, contactsPort);
        }
    }
    if (ADIFSelect && !ADIFHost.isNull())
    {
        QString header = tr("Exported by Minos VHF logging system Version %1 %2").arg(STRINGVERSION).arg(PRERELEASETYPE) + "\r\n";

        header += "<EOH>\r\n";

        QString adif = tct->getADIFLine();
//        trace((header + adif).toUtf8());

        bc.writeDatagram((header + adif).toUtf8(), ADIFHost, ADIFPort);
    }
}
void N1MMBroadcast::dxSpots(QSharedPointer<ClusterSpotData> spotMsg, bool delSpot)
{
    // afterQSOSaved gives us QSOs as spots
    // this gives us direct spots

    // We also need purged spots and altered spots; these all come from bandmap
    // we may need to have bandmap enabled when not shown for all this to work

    if (spotsSelect)
    {
        QString sp = genSpotsStanza(spotMsg, delSpot);
        bc.writeDatagram(sp.toUtf8(), spotsHost, spotsPort);
    }
}

void N1MMBroadcast::wsjtxDatagram(int, QByteArray *datagram)
{
    // rebroadcast wsjt-x datagram
    if (wsjtxRbSelect && !wsjtxRbHost.isNull())
    {
        bc.writeDatagram(*datagram, wsjtxRbHost, wsjtxRbPort);
    }
}

void N1MMBroadcast::callsignLookup(BaseContestLog *c, QString call)
{
    if (call.isEmpty())
        return;

    // contact stanza but with callsign only
    if (extCSSelect && !extCSHost.isNull())
    {
        QSharedPointer<BaseContact> tct(new BaseContact(c, true));

        tct->cs.setFullCall(call);

        tct->ctryMult = findCtryPrefix( tct->cs );


        QString stanza = genContactStanza("lookupinfo", c, tct);
        bc.writeDatagram(stanza.toUtf8(), extCSHost, extCSPort);
 //       trace("callsignLookup Datagram written " + stanza);
    }
}

QString makeTag(const QString &tag, const QString &arg)
{
    QString temp = "<" + tag + ">" +  escapeXML(arg) + "</" + tag + ">\n";
    return temp;
}

QString N1MMBroadcast::genDeleteStanza(QSharedPointer<BaseContact> tct)
{
    //                <?xml version="l.0" encoding="utf-8"?>
    //                <contactdelete>
    //                    <app>N1MM</app>
    //                    <timestamp>2020-01-17 16 :43:38</timestamp>
    //                    <call>WlAW</call>
    //                    <contestnr>73</contestnr>
    //                    <StationName>CONTEST-PC</StationName>
    //                </contactdelete>
    QString xml = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
                  + "<contactdelete>\n"
                   + makeTag("app", "Minos")
                   + makeTag("contestnr", "0")                          //        <contestnr>10</contestnr>
                   + makeTag("timestamp", tct->timeOff.getN1mmDTG())       //        <timestamp>2016-04-10 16:17:41</timestamp>
                   + makeTag("call", tct->cs.getFullCall())       //        <call>W2BBB</call>
                   + makeTag("StationName", "")                         //        <StationName>PHONE-15M</StationName>
            + "</contactdelete>\n";

    return xml;
}

QString N1MMBroadcast::genContactStanza(QString type, BaseContestLog *b, QSharedPointer<BaseContact> tct)
{
    LoggerContestLog *c = dynamic_cast<LoggerContestLog *>(b);

    QString cb;
    double freq = c->getAdifFreqBand(tct->frequency.getValue(), cb);

    // freq sent is only to the tens digit...
    freq = floor(freq/10.0);
    QString sfreq = QString::number(freq, 'f', 0).remove('.');

    QString continent = (tct->ctryMult?tct->ctryMult->getContinent():QString());

    QString mode = tct->mode.getValue();
    QString mgmSubmode = tct->mgmSubmode.getValue();
    if ( (mode.compare( hamlibData::MGM) == 0 || mode.compare( "MFSK") == 0) && !mgmSubmode.trimmed().isEmpty())
    {
         mode = mgmSubmode;
    }


    QString xml = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
                  + "<" + type + ">\n"
                   + makeTag("app", "Minos")
                   + makeTag("contestname", c->name.getValue())         //        <contestname>DXPEDITION</contestname>
                   + makeTag("contestnr", "0")                          //        <contestnr>10</contestnr>
                   + makeTag("timestamp", tct->timeOff.getN1mmDTG())       //        <timestamp>2016-04-10 16:17:41</timestamp>
                   + makeTag("mycall", c->mycall.getFullCall())   //        <mycall>K8UT</mycall>
                   + makeTag("band", cb)                                //        <band>21</band>
                   + makeTag("rxfreq", sfreq)                          //        <rxfreq>2125500</rxfreq>
                   + makeTag("txfreq", sfreq)                          //        <txfreq>2125500</txfreq>
                   + makeTag("operator", tct->op1.getValue())           //        <operator>K8UT</operator>
                   + makeTag("mode", mode)                              //        <mode>USB</mode>
                   + makeTag("call", tct->cs.getFullCall())       //        <call>W2BBB</call>
                   + makeTag("countryprefix", tct->cs.locCtryPrefix)    //        <countryprefix>K</countryprefix>
                   + makeTag("wpxprefix", tct->cs.wpxPrefix)            //        <wpxprefix>W2</wpxprefix>
                   + makeTag("stationprefix",c->mycall.getFullCall()) //    <stationprefix>K8UT</stationprefix>
                   + makeTag("continent", continent)                    //        <continent>NA</continent>
                   + makeTag("snt", tct->reps.getValue())               //        <snt>59</snt>
                   + makeTag("sntnr", tct->serials.getValue())          //        <sntnr>2</sntnr>
                   + makeTag("rcv", tct->repr.getValue())               //        <rcv>59</rcv>
                   + makeTag("rcvnr", tct->serialr.getValue())          //        <rcvnr>0</rcvnr>
                   + makeTag("gridsquare", tct->loc.getLoc())     //        <gridsquare></gridsquare>
                   + makeTag("exchange1", tct->extraText.getValue())    //        <exchange1></exchange1>
                   + makeTag("section", c->entSect.getValue())          //        <section></section>
                   + makeTag("comment", tct->comments.getValue())       //        <comment></comment>
                   + makeTag("qth", "")                                 //        <qth></qth>
                   + makeTag("name", "")                                //        <name></name>
                   + makeTag("power", c->power.getValue())              //        <power></power>
                   + makeTag("misctext", "")                            //        <misctext></misctext>
                   + makeTag("zone", "")                                //        <zone>5</zone>
                   + makeTag("prec", "")                                //        <prec></prec>
                   + makeTag("ck", "")                                  //        <ck>0</ck>
                   + makeTag("ismultiplier1", "")                       //        <ismultiplier1>0</ismultiplier1>
                   + makeTag("ismultiplier2", "")                       //        <ismultiplier2>0</ismultiplier2>
                   + makeTag("ismultiplier3", "")                       //        <ismultiplier3>0</ismultiplier3>
                   + makeTag("points", "")                              //        <points>1</points>
                   + makeTag("radionr", "")                             //        <radionr>1</radionr>
                   + makeTag("RoverLocation", "")                       //        <RoverLocation></RoverLocation>
                   + makeTag("RadioInterfaced", "")                     //        <RadioInterfaced>0</RadioInterfaced>
                   + makeTag("NetworkedCompNr", "")                     //        <NetworkedCompNr>0</NetworkedCompNr>
                   + makeTag("IsOriginal", "True")                      //        <IsOriginal>True</IsOriginal>
                   + makeTag("NetBiosName", "")                         //        <NetBiosName>DEV-PC</NetBiosName>
                   + makeTag("IsRunQSO", "0")                           //        <IsRunQSO>0</IsRunQSO>
//                   + makeTag("Run1Run2", "")                          //        <Run1Run2></Run1Run2>
//                   + makeTag("ContactType", "")                       //        <ContactType></ContactType>
                   + makeTag("StationName", "")                         //        <StationName>PHONE-15M</StationName>
            + "</" + type + ">\n";

    return xml;

}
QString N1MMBroadcast::genSpotsStanza(QSharedPointer<ClusterSpotData> spotMsg, bool delSpot)
{
//    <?xml version="1.0" encoding="utf-8"?>
//    <spot>
//        <app>N1MM</app>
//        <StationName>CONTEST-PC</StationName>
//        <dxcall>AL3CDE</dxcall>
//        <frequency>7061.2</frequency>
//        <spottercall>K2PO/7-#</spottercall>
//        <timestamp>2O20-Ol-l7 17:19:37</timestamp>
//        <action>add</action>
//        <mode>CW</mode>
//        <comment>CW 9 DB 18 WPM CQ AK </comment>
//        <status>single mult</status>
//        <statuslist>single mult</statuslist>
//    </spot>

//    The Spot Data packet contains all spots processed by the program whether from Telnet
//    (including RBN), Logging QSOs, or local spotting.
//    The values for action are:

//    add
//    delete

//    The values for status are:

//    busy – marked by N1MM user as a frequency to note
//    bust – a busted call (when CT1BOH tags are present)
//    cq – the cq frequency on this band (last place F1 was pressed)
//    dupe – duplicate contact
//    qtc – a WAE qtc
//    single mult – this spot is a single multiplier in this contest
//    double mult – this spot is a double (or more) mult in this contest
//    new qso – a logged qso (this is now a dupe by definition)


//    StationName – the callsign shown in the station dialog
//    dxcall – the station that is spotted
//    spottercall – the station that spotted the call (StationName for stations worked, or spotted locally)
//    comment – the comment from the spot
//    action – whether this spot was added or deleted (spots are deleted when they move within a band)
//    status – dupe, mult etc. See above for values
//    timestamp – the time of the spot

    Frequency f= spotMsg.data()->getFreq();

    double freq = f.toInt64();
    // freq sent is KHz
    freq = freq/1000.0;
    QString sfreq = QString::number(freq, 'f', 3);

    QDateTime  dt = spotMsg.data()->getSpotDateTime();
    dtg dg(false);
    dg.setDateTime(dt);
    QString ts = dg.getN1mmDTG();

    QString adddel = "add";
    QString comment = spotMsg.data()->getSpotComment();
    QString dxcall = spotMsg.data()->getDxCallStr();
    QString status;
    bandmapSpotType::SPOT_TYPE st = spotMsg->getSpotType();
    // enum SPOT_TYPE {NONE, CLUSTER, CLUSTER_MARKED, LOGGED, MARKED, SAVED, CQ, DELETED};

    if (st == bandmapSpotType::CQ)
    {
        status = "cq";
        comment = tr("CQ frequency");
    }
    else if (st == bandmapSpotType::LOGGED)
    {
        status = "new qso";
    }
    else if (st == bandmapSpotType::DELETED)
    {
        adddel = "delete";
    }
    else
    {
        status = "busy";
    }
    if (delSpot)
    {
        adddel = "delete";
    }
    QString xml = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
            + "<spot>\n"
                   + makeTag("app", "Minos")
                   + makeTag("StationName", "")                         // <StationName>PHONE-15M</StationName>
                   + makeTag("dxcall", dxcall)
                   + makeTag("frequency", sfreq)
                   + makeTag("spottercall", spotMsg.data()->getSpotterCallStr())
                   + makeTag("timestamp", ts)
                   + makeTag("action", adddel)
                   + makeTag("mode", spotMsg.data()->getMode())
                   + makeTag("comment", comment)
                   + makeTag("status", status)
                   + makeTag("statuslist", status)
            + "</spot>\n";

    return xml;
}
