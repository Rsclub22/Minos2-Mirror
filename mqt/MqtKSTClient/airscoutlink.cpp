#include "kstmainwindow.h"
#include "airscoutlink.h"

AirScoutLink::AirScoutLink():
    qus(new QUdpSocket())
{
    qus->bind(QHostAddress::Any, 9872, (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint));

    connect(qus.data(), SIGNAL(readyRead( )), this, SLOT(onReadyRead()));

    if (mainWindow->getASActive())
        sendMessage( "ASSETPATH", "4320000,G0GJV,IO91OK,DF2JP,JO31CO");
}

void AirScoutLink::sendToAllBroadcast(QByteArray *packet)
{
    // Get network interfaces list
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();

    // Interfaces iteration
    for (int i = 0; i < ifaces.size(); i++)
    {
        if (ifaces[i].flags().testFlag(QNetworkInterface::IsLoopBack))
            continue;

        // Now get all IP addresses for the current interface
        QList<QNetworkAddressEntry> addrs = ifaces[i].addressEntries();

        // And for any IP address, if it is IPv4 and the interface is active, send the packet
        for (int j = 0; j < addrs.size(); j++)
        {
            if ((addrs[j].ip().protocol() == QAbstractSocket::IPv4Protocol) && (addrs[j].broadcast().toString() != ""))
            {
                qint64 res = qus->writeDatagram(packet->data(), packet->length(), addrs[j].broadcast(), 9872);
                trace(QString("%1 bytes sent").arg(res));
                if (res > 0)
                    return;
            }
        }
    }
}

qint64 AirScoutLink::sendMessage(QString messagetype, QString messageText)
{
    QString mess = messagetype + ": \"" + mainWindow->getASMyName() + "\" \"" + mainWindow->getASServerName() +  "\" " + messageText;

    int cs = 0;
    QByteArray packet = QByteArray(mess.toUtf8());
    foreach(auto c, packet)
    {
        cs += c;
    }

    packet += static_cast<char>((cs | 0x80)&0xff);
    packet += '\0';
    qint64 res = 0;

    sendToAllBroadcast(&packet);

    trace("Datagram sent: " + mess);

    return res;
}
/*

Command: ASNEAREST
Source: Name of Client
Destination: Name of Server

UTC: Timestamp of calculation finished (YYYY-MM-DD HH:mm:ssZ)
MyCall: My call sign
MyLoc: My grid locator
DXCall: DX call sign
DXLoc: DX grid locator
Count: Number of aircraft data records to follow [0 .. n]
Data: Aircraft data records, each record as follows:
    Call: Call of aircraft
    Category: Category of aircraft according to AirScout’s
        category code:
        L = Light,
        M = Medium,
        H = Heavy,
        S = Super heavy
    Distance: Distance to path crossing [km]
    Potential: Current reflection potential according to
        AirScout’s colour code:
        100 = magenta,
        75 = red,
        50 = orange,
        <50 = grey
    Minutes: Time left to path crossing [mins]

Example (without line breaks, only 3 of 25 data sets shown):
ASNEAREST: ”KST_23” ”AS_23” ”2015-08-19
08:07:06Z,DL2ALF,JO50IW,GB3MHL,JO02PB,25,AFR1044,M,294,75,20,DLH507,H,253,5
0,17, ... ,WZZ933,M,28,75,2”

 */
void AirScoutLink::onReadyRead()
{
    trace("AirScoutLink::onReadyRead()");
    while (qus->hasPendingDatagrams())
    {
        trace("Pending datagrams");
        QByteArray buf;
        buf.resize(static_cast<int>(qus->pendingDatagramSize()));
        QHostAddress host;
        quint16 port;
        qint64 res = qus->readDatagram(buf.data(), buf.size(), &host, &port);
        QString dg = QString(buf);

        trace("Datagram received from " + host.toString() + " " + dg);
        if (res > 0)
        {
            cs = 0;

            qint64 limit = res - 1;
            if (buf.at(static_cast<int>(limit)) == 0)
               limit--;

            for (int i = 0; i < limit; i++)
            {
                cs += buf[i];
            }
            lcs = static_cast<char>((cs & 0x7f)|0x80);
            lastbyte = buf.at(static_cast<int>(limit));

            // If "nearest", analyse for this call and any planes, update userlist

            // then ask for next path, after a short delay. Look down watchlist
            // for this call, and go on to the next. If we can't find it, go back to the start

            //processZConfString(dg, host, sendBeaconResponse);

        }
    }
}

void AirScoutLink::usersChanged(QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector, int chatId, QString filterString)
{
    if (mainWindow->getASActive())
    {
        watchList.clear();
        watchList.append("4320000");        // band
        for(QVector<QSharedPointer<KstUser> >::iterator user = callVector->begin(); user != callVector->end(); user++)
        {
            if (chatId != 0 && user->data()->chat != chatId)
                continue;

            int mind = mainWindow->getASMinDistance();
            int maxd = mainWindow->getASMaxDistance();
            if (user->data()->distance < mind ||( maxd > 0 && user->data()->distance > maxd))
                continue;

            if (user->data()->call.contains(filterString) || user->data()->loc.contains(filterString))
            {
                watchList.append(user->data()->baseCall);
                watchList.append(user->data()->loc);
            }
        }
        if (watchList.count() > 1)
        {
            QString watch = watchList.join(",");

            if (watch != oldWatch)
            {
                QString watchText = /*"\"" +*/ watch /*+ "\""*/;

                sendMessage("ASWATCHLIST", watchText);
                oldWatch = watch;
            }
        }
    }
}

void AirScoutLink::askNearest(QString lastcall)
{
    QString getpath = /*"\""  +*/ watchList[0] + ","
            + "G0GJV,IO91OK,"
            + watchList[11] + "," + watchList[12] /*+ "\""*/;
    sendMessage("ASSETPATH", getpath);
}

