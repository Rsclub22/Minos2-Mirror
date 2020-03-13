#include <QHostInfo>
#include "MinosLoggerEvents.h"
#include "ContestApp.h"
#include "cutils.h"
#include "contest.h"
#include "contacts.h"
#include "LoggerContest.h"
#include "BandList.h"
#include "rigutils.h"
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
bool N1MMBroadcast::setAddress(QString addr, QHostAddress &host)
{
    QHostInfo haddr = QHostInfo::fromName( addr );
    for (int i = 0; i < haddr.addresses().count(); i++)
    {
        bool ok;
        quint32 iaddr;
        iaddr = haddr.addresses()[i].toIPv4Address(&ok);
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
        QString stanza = genContactStanza("contactinfo", c, tct);
        bc.writeDatagram(stanza.toUtf8(), contactsHost, contactsPort);
        trace("afterQSOSaved Datagram written " + stanza);
    }
    if (ADIFSelect && !ADIFHost.isNull())
    {
        QString header = tr("Exported by Minos VHF logging system Version %1 %2").arg(STRINGVERSION).arg(PRERELEASETYPE) + "\r\n";

        header += "<EOH>\r\n";

        QString adif = tct->getADIFLine();

        bc.writeDatagram((header + adif).toUtf8(), ADIFHost, ADIFPort);
    }
}

void N1MMBroadcast::wsjtxDatagram(QByteArray *datagram)
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

        tct->cs = Callsign(call);
        tct->cs.validate();

        tct->ctryMult = findCtryPrefix( tct->cs );


        QString stanza = genContactStanza("lookupinfo", c, tct);
        bc.writeDatagram(stanza.toUtf8(), extCSHost, extCSPort);
        trace("callsignLookup Datagram written " + stanza);
    }
}

QString makeTag(const QString &tag, const QString &arg)
{
    QString temp = "<" + tag + ">" +  escapeXML(arg) + "</" + tag + ">\n";
    return temp;
}
QString N1MMBroadcast::genContactStanza(QString type, BaseContestLog *b, QSharedPointer<BaseContact> tct)
{
    LoggerContestLog *c = dynamic_cast<LoggerContestLog *>(b);

    QString txfreq = tct->frequency.getValue().remove('.');
    QString cband = c->band.getValue();

    QString cb = cband.trimmed();
    BandList &blist = BandList::getBandList();
    BandInfo bi;
    bool bandOK = blist.findBand(cb, bi);
    if (bandOK)
    {
        cb = bi.adif;
        if (txfreq.isEmpty())
        {
            txfreq = QString::number(bi.flow);
        }
    }

    long freq = static_cast<long>(convertStrToFreq(txfreq)/10.0);

    // freq sent is only to the tens digit...

    txfreq = QString::number(freq);

    QString continent = (tct->ctryMult?tct->ctryMult->continent:QString());

    QString xml = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
                  + "<" + type + ">\n"
                   + makeTag("app", "Minos")
                   + makeTag("contestname", c->name.getValue())         //        <contestname>DXPEDITION</contestname>
                   + makeTag("contestnr", "0")                          //        <contestnr>10</contestnr>
                   + makeTag("timestamp", tct->time.getN1mmDTG())       //        <timestamp>2016-04-10 16:17:41</timestamp>
                   + makeTag("mycall", c->mycall.fullCall.getValue())   //        <mycall>K8UT</mycall>
                   + makeTag("band", cb)                                //        <band>21</band>
                   + makeTag("rxfreq", txfreq)                          //        <rxfreq>2125500</rxfreq>
                   + makeTag("txfreq", txfreq)                          //        <txfreq>2125500</txfreq>
                   + makeTag("operator", tct->op1.getValue())           //        <operator>K8UT</operator>
                   + makeTag("mode", tct->mode.getValue())              //        <mode>USB</mode>
                   + makeTag("call", tct->cs.fullCall.getValue())       //        <call>W2BBB</call>
                   + makeTag("countryprefix", tct->cs.locCtryPrefix)    //        <countryprefix>K</countryprefix>
                   + makeTag("wpxprefix", tct->cs.wpxPrefix)            //        <wpxprefix>W2</wpxprefix>
                   + makeTag("stationprefix",c->mycall.fullCall.getValue()) //    <stationprefix>K8UT</stationprefix>
                   + makeTag("continent", continent)                    //        <continent>NA</continent>
                   + makeTag("snt", tct->reps.getValue())               //        <snt>59</snt>
                   + makeTag("sntnr", tct->serials.getValue())          //        <sntnr>2</sntnr>
                   + makeTag("rcv", tct->repr.getValue())               //        <rcv>59</rcv>
                   + makeTag("rcvnr", tct->serialr.getValue())          //        <rcvnr>0</rcvnr>
                   + makeTag("gridsquare", tct->loc.loc.getValue())     //        <gridsquare></gridsquare>
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
