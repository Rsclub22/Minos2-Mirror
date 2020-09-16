#include <QHostAddress>
#include <QHostInfo>
#include "tinyxml.h"
#include "TinyUtils.h"
#include "cutils.h"
#include "rigutils.h"
#include "n1mmlink.h"
/*
 *
The recommended default port number for N1MM Logger applications is 12060.
Destination port numbers are defined by adding a colon “:” to the IP address,
followed by the port number. For example, to send packets at port 12060 to
this PC and to all other PCs on this subnet: 127.0.0.1:12060 192.168.1.255:12060

Radio Info
<?xml version="l.0" encoding="utf-8"?>
<RadioInfo>
    <app>N1MM</app>
    <StationName>CW-80m</StationName>
    <RadioNr>l</RadioNr>
    <Freq>352211</Freq>
    <TXFreq>352211</TXFreq>
    <Mode>CW</Mode>
    <OpCall>W1ABC</OpCall>
    <IsRunning>False</IsRunning>
    <FocusEntry>204626</FocusEntry>
    <EntryWindowHwnd>
    <Antenna>ll</Antenna>
    <Rotors></Rotors>
    <FocusRadioNr>l</FocusRadioNr>
    <IsStereo>False</IsStereo>
    <IsSplit>False</IsSplit>
    <ActiveRadioNr>l</ActiveRadioNr>
    <IsTransmitting>False</IsTransmitting>
    <FunctionKeyCaption></FunctionKeyCaption>
    <RadioName></RadioName>
</RadioInfo>
Radio Info Notes:

StationName is the NetBios name of the computer that is sending these messages.
It is the name used in Multi-Computer networking. Windows limits it to 15 characters.
If the computer name is greater than 15 characters long, the first 15 characters will be used.

RadioNr is the radio number associated with a specific XML packet – in other words, the source
 of the information in that packet. When in SO2V or SO2R mode, N1MM+ sends two packets
every ten seconds – one packet each from RadioNr1 and RadioNr2

Freq is the receive frequency represented as values to the tens digit with no delimiter.
For example: 160 meters: 181234; 40 meters: 712345; 10 meters: 2812345; 6 meters: 5012345

TXFreq is the transmit frequency represented as values to the tens digit with no delimiter.
For example: 160 meters: 181234; 40 meters: 712345; 10 meters: 2812345; 6 meters: 5012345

Mode could be any one of the following: CW, USB, LSB, RTTY, PSK31, PSK63, PSK125, PSK250,
QPSK31, QPSK63, QPSK125, QPSK250, FSK, GMSK, DOMINO, HELL, FMHELL, HELL80, MT63, THOR,
THRB, THRBX, OLIVIA, MFSK8, MFSK16

OpCall is the callsign entered by the operator after OPON (or Ctl-O). Defaults to the station call

IsRunning represents the value of the RUN versus S&P radio buttons in the Entry Window. If you
are on your CQ frequency, IsRunning will be True

FocusEntry is the Windows assigned handle of the Entry Window with program focus

EntryWindowHwnd allows external software to send commands to a specific EntryWindow (i.e. using
AHK to send F1 to EW2 etc.)

Antenna is the currently selected antenna for this radio (0-15), from the Antenna tab in the Configurer

Rotors is the name of the currently selected rotor from the Antenna table in the Configurer

FocusRadioNr Receive Radio Focus – the Green Dot in the Entry Window. Enables signal switching for
SO2R operation – facilitating the routing of Microphone, Audio, PTT, CW signals to/from the selected
radio. FocusRadioNr toggles between 1 and 2 when the program, a mouse click, or the \ key selects the opposite Entry Window

IsStereo enables audio switching for SO2R operation.The ` key (backquote) toggles its value between True and False

ActiveRadioNr Transmit Radio Focus – the Red Dot in the Entry Window. Enables signal switching for
SO2R operation – facilitating the routing of Microphone, Audio, PTT, CW signals to/from the selected
radio. ActiveRadioNr toggles between 1 and 2 when the program or pressing the [Pause] key selects the opposite Entry Window

IsSplit Transceiver is in Split VFO mode (for example: vfoA=Rx, vfoB=Tx). Not available on all – especially older – rigs

IsTransmitting Program is in transmit (True) or receive (False) mode

FunctionKeyCaption The label of the Function Key that was pressed to initiate this transmission

RadioName The name of the radio, as shown in the Entry Window (i.e. “TS-590”)

The program sends Radio Info packets at 10 second intervals, or immediately after an event where any of the
information in one of these fields changes (for example: user changes bands, spins the VFO dial, switches
from Run to S&P, or selects VFOb).

*/
/*
You can send a QSY command to N1MM (and therefore your connected radio) using the Radio_SetFrequency
packet. The packet encoding must use UTF8, and the port set for 13064.

<?xml version="l.0" encoding="utf-8"?>
<radio_setfrequency>
    <app>WaterfallBandmap</app>
    <radionr>1</radionr>
    <frequency>14053.284</frequency>
    <mousebutton>Right</mousebutton>
</radio_setfrequency>


Keyword	Meaning
app	Application sending the command
radionr	The radio to be changed. In SO2V radio 2 means VFO B.
frequency	Requested frequency. NB. Uses period as decimal separator.
mousebutton	Which mouse button was used to create message, if any.
            Right button seems to have a special meaning!
*/

const int N1MMTimerInterval = 15000;
N1MMLink::N1MMLink(QObject *parent):
    QObject(parent)
{

}

void N1MMLink::initialise()
{
    // set up receive UDP link
    qus = QSharedPointer<QUdpSocket>(new QUdpSocket());
    qus->bind(QHostAddress::Any, 12060, (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint));


    connect (qus.data(), SIGNAL(readyRead()), this, SLOT(onReceiveUDP()));

    connect( &connectTimer, SIGNAL(timeout()), this, SLOT(connectTimeout()));
    connectTimer.start(N1MMTimerInterval);  // longer than the N1MM 10 sec repeat frequency

}

void N1MMLink::connectTimeout()
{
    connected = false;
    currFrequency = "N1MM not connected";
}
static QString makeTag(const QString &tag, const QString &arg)
{
    QString temp = "<" + tag + ">" +  escapeXML(arg) + "</" + tag + ">\n";
    return temp;
}

QString N1MMLink::genFreqStanza(long f)
{/*
    <?xml version="l.0" encoding="utf-8"?>
    <radio_setfrequency>
        <app>WaterfallBandmap</app>
        <radionr>1</radionr>
        <frequency>14053.284</frequency>
        <mousebutton>Right</mousebutton>
    </radio_setfrequency>

*/
    double freq = f/1000.0;
    QString xml = QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>\n")
                  + "<radio_setfrequency>\n"
                   + makeTag("app", "Minos")
                   + makeTag("radionr", "1")
                   + makeTag("frequency", QString::number(freq,'f', 3))
                   + makeTag("mousebutton", "Left")
            + "</radio_setfrequency>\n";

    return xml;
}
//bool N1MMLink::setAddress(QString addr, QHostAddress &host)
//{
//    QHostInfo haddr = QHostInfo::fromName( addr );
//    for (int i = 0; i < haddr.addresses().count(); i++)
//    {
//        bool ok = true;
//        quint32 iaddr;

//        // This is a guess as to when the change came in
//        iaddr = haddr.addresses()[i].toIPv4Address(
//            #if QT_VERSION > QT_VERSION_CHECK(5, 4, 0)
//                    &ok
//            #endif
//                    );
//        if (ok)
//        {
//            host.setAddress(iaddr);
//            return true;
//        }
//    }
//    return false;
//}

void N1MMLink::sendFrequencyRequest(long f)
{
    // send f to N1MM
//    QHostAddress addr;
//    setAddress("127.0.0.1", addr);
    QString stanza = genFreqStanza(f);
    qint64 ret = bc.writeDatagram(stanza.toUtf8(), QHostAddress::LocalHost, 13064);
    if (ret > 0)
    {
        trace(QString("Datagram sent %1").arg(stanza));
    }
    else
    {
        trace(QString("Datagram failed %1").arg(stanza));
    }
}

void N1MMLink::onReceiveUDP()
{
    connected = true;
    connectTimer.start(N1MMTimerInterval);  // longer than the N1MM 10 sec repeat frequency
    trace("N1MMLink::onReadyRead()");
    while (qus->hasPendingDatagrams())
    {
        trace("Pending datagrams");
        QByteArray buf;
        buf.resize(static_cast<int>(qus->pendingDatagramSize()));
        QHostAddress host;
        quint16 port;
        qint64 res = qus->readDatagram(buf.data(), buf.size(), &host, &port);
        QString dg = QString(buf);
        QString dgs = dg.remove("\r").replace("\n", " ");

        trace("Datagram received from " + host.toString() + " " + dgs);
        if (res > 0)
        {
            // Analyse...
            TiXmlBase::SetCondenseWhiteSpace( false );
            TiXmlDocument xdoc;
            TIXML_STRING smessage = dg.toStdString();// allowed conversion through TIXML_STRING
            xdoc.Parse( smessage.c_str(), nullptr );
            TiXmlElement * root = xdoc.RootElement();
            if ( !root )
            {
               //err = "No XML root element (does file exist?)";
                continue;;
            }
            if ( !checkElementName( root, "RadioInfo" ) )
            {
               //err = "Invalid XML root element";
                continue;;
            }
            for ( TiXmlElement * e = root->FirstChildElement(); e; e = e->NextSiblingElement() )
            {
               if ( checkElementName( e, "Freq" ) )
               {
                    currFrequency = QString(e->GetText()) + "0";    // as freq is in tens of Hz
               }
               if ( checkElementName( e, "RadioName" ) )
               {
                    radioName = e->GetText();
               }
               if ( checkElementName( e, "Mode" ) )
               {
                    mode = e->GetText();
               }
            }

        }
    }

}
QString N1MMLink::getFrequency()
{
    return currFrequency;
}

QString N1MMLink::getMode()
{
    return mode;
}

QString N1MMLink::getRadioName()
{
    return radioName;
}

bool N1MMLink::isConnected()
{
    return connected;
}
