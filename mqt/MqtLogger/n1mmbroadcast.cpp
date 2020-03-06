#include "MinosLoggerEvents.h"
#include "ContestApp.h"

#include "n1mmbroadcast.h"

N1MMBroadcast::N1MMBroadcast()
{
    connect(&MinosLoggerEvents::mle, SIGNAL(afterQSOSaved(BaseContestLog *, QSharedPointer<BaseContact>)),
            this, SLOT(afterQSOSaved(BaseContestLog *, QSharedPointer<BaseContact> )));
    connect(&MinosLoggerEvents::mle, SIGNAL(wsjtxDatagram(QByteArray *)),
            this, SLOT(wsjtxDatagram(QByteArray *)));
    connect(&MinosLoggerEvents::mle, SIGNAL(callsignLookup(BaseContestLog *, QString)),
            this, SLOT(callsignLookup(BaseContestLog *, QString)));

}
void N1MMBroadcast::configure()
{
    int temp;
    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpcontactsSelect, contactsSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpcontactsAddr, contactsAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpcontactsPort, temp );
    contactsPort = static_cast<quint16>(temp);
    if (!contactsHost.setAddress(contactsAddr))
    {

    }

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpextCSSelect, extCSSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpextCSAddr, extCSAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpextCSPort, temp );
    extCSPort = static_cast<quint16>(temp);
    if (!extCSHost.setAddress(extCSAddr))
    {

    }

    TContestApp::getContestApp() ->loggerBundle.getBoolProfile( elpwsjtxRbSelect, wsjtxRbSelect );
    TContestApp::getContestApp() ->loggerBundle.getStringProfile( elpwsjtxRbAddr, wsjtxRbAddr );
    TContestApp::getContestApp() ->loggerBundle.getIntProfile( elpwsjtxRbPort, temp );
    wsjtxRbPort = static_cast<quint16>(temp);
    if (!wsjtxRbHost.setAddress(wsjtxRbAddr))
    {

    }


}
void N1MMBroadcast::afterQSOSaved(BaseContestLog *c, QSharedPointer<BaseContact> tct)
{
    // This can generate modified, deleted/recreated, or straight contact
    if (contactsSelect)
    {
        QString stanza = genContactStanza("contact", c, tct);
        bc.writeDatagram(stanza.toUtf8(), extCSHost, extCSPort);
    }
}

void N1MMBroadcast::wsjtxDatagram(QByteArray *datagram)
{
    // rebroadcast wsjt-x datagram
    if (wsjtxRbSelect)
    {
        bc.writeDatagram(*datagram, wsjtxRbHost, wsjtxRbPort);
    }
}

void N1MMBroadcast::callsignLookup(BaseContestLog *c, QString call)
{
    // contact stanza but with callsign only
    if (extCSSelect)
    {
        QSharedPointer<BaseContact> tct(new BaseContact(c, true));

        tct->cs = Callsign(call);
        tct->cs.validate();

        QString stanza = genContactStanza("lookup", c, tct);
        bc.writeDatagram(stanza.toUtf8(), extCSHost, extCSPort);
    }
}

//QString N1MMBroadcast::genLookupStanza(BaseContestLog *c, QString call)
//{
//    <?xml version="1.0" encoding="utf-8"?>
//        <lookup>
//            <contestname>DXPEDITION</contestname>
//            <contestnr>10</contestnr>
//            <timestamp>2016-04-10 16:17:41</timestamp>
//            <mycall>K8UT</mycall>
//            <band>21</band>
//            <rxfreq>2125500</rxfreq>
//            <txfreq>2125500</txfreq>
//            <operator>K8UT</operator>
//            <mode>USB</mode>
//            <call>W2BBB</call>
//            <countryprefix>K</countryprefix>
//            <wpxprefix>W2</wpxprefix>
//            <stationprefix>K8UT</stationprefix>
//            <continent>NA</continent>
//            <snt>59</snt>
//            <sntnr>2</sntnr>
//            <rcv>59</rcv>
//            <rcvnr>0</rcvnr>
//            <gridsquare></gridsquare>
//            <exchange1></exchange1>
//            <section></section>
//            <comment></comment>
//            <qth></qth>
//            <name></name>
//            <power></power>
//            <misctext></misctext>
//            <zone>5</zone>
//            <prec></prec>
//            <ck>0</ck>
//            <ismultiplier1>0</ismultiplier1>
//            <ismultiplier2>0</ismultiplier2>
//            <ismultiplier3>0</ismultiplier3>
//            <points>1</points>
//            <radionr>1</radionr>
//            <RoverLocation></RoverLocation>
//            <RadioInterfaced>0</RadioInterfaced>
//            <NetworkedCompNr>0</NetworkedCompNr>
//            <IsOriginal>True</IsOriginal>
//            <NetBiosName>DEV-PC</NetBiosName>
//            <IsRunQSO>0</IsRunQSO>
//            <Run1Run2></Run1Run2>
//            <ContactType></ContactType>
//            <StationName>PHONE-15M</StationName>
//        </lookup>

//}
QString N1MMBroadcast::genContactStanza(QString type, BaseContestLog *c, QSharedPointer<BaseContact> tct)
{
//    <?xml version="1.0" encoding="utf-8"?>
//    <contactinfo>
//        <contestname>DXPEDITION</contestname>
//        <contestnr>10</contestnr>
//        <timestamp>2016-04-10 16:17:41</timestamp>
//        <mycall>K8UT</mycall>
//        <band>21</band>
//        <rxfreq>2125500</rxfreq>
//        <txfreq>2125500</txfreq>
//        <operator>K8UT</operator>
//        <mode>USB</mode>
//        <call>W2BBB</call>
//        <countryprefix>K</countryprefix>
//        <wpxprefix>W2</wpxprefix>
//        <stationprefix>K8UT</stationprefix>
//        <continent>NA</continent>
//        <snt>59</snt>
//        <sntnr>2</sntnr>
//        <rcv>59</rcv>
//        <rcvnr>0</rcvnr>
//        <gridsquare></gridsquare>
//        <exchange1></exchange1>
//        <section></section>
//        <comment></comment>
//        <qth></qth>
//        <name></name>
//        <power></power>
//        <misctext></misctext>
//        <zone>5</zone>
//        <prec></prec>
//        <ck>0</ck>
//        <ismultiplier1>0</ismultiplier1>
//        <ismultiplier2>0</ismultiplier2>
//        <ismultiplier3>0</ismultiplier3>
//        <points>1</points>
//        <radionr>1</radionr>
//        <RoverLocation></RoverLocation>
//        <RadioInterfaced>0</RadioInterfaced>
//        <NetworkedCompNr>0</NetworkedCompNr>
//        <IsOriginal>True</IsOriginal>
//        <NetBiosName>DEV-PC</NetBiosName>
//        <IsRunQSO>0</IsRunQSO>
//        <Run1Run2></Run1Run2>
//        <ContactType></ContactType>
//        <StationName>PHONE-15M</StationName>
//    </contactinfo>

}
