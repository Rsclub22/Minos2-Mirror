#include "airscoutlink.h"
#include "kstmainwindow.h"

const char *AirScoutLink::ASBandStrings[] = {
    QT_TR_NOOP("50MHz"),
    QT_TR_NOOP("70MHz"),
    QT_TR_NOOP("144MHz"),
    QT_TR_NOOP("432MHz"),
    QT_TR_NOOP("1.2GHz"),
    QT_TR_NOOP("2.3GHz"),
    QT_TR_NOOP("3.4GHz"),
    QT_TR_NOOP("5.7GHz"),
    QT_TR_NOOP("10GHz"),
    QT_TR_NOOP("24GHz"),
    QT_TR_NOOP("47GHz"),
    QT_TR_NOOP("76GHz"),
    nullptr
};
// frequencies are in 100 hz unit
static const char * bandFreqStrings[] = {
       "500000",
       "700000",
      "1440000",
      "4320000",
     "12960000",
     "23200000",
     "34000000",
    " 57600000",
    "103680000",
    "240480000",
    "470880000",
    "760320000"
};
AirScoutLink::AirScoutLink():
    qus(new QUdpSocket())
{
    qus->bind(QHostAddress::Any, static_cast<quint16>(mainWindow->getASPort()), (QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint));

    connect(qus.data(), SIGNAL(readyRead( )), this, SLOT(onReadyRead()));

    connect(&ASTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));
    ASTimer.start(1000);
}
AirScoutLink::~AirScoutLink()
{
    qus->abort();   // make sure we don't get any more reads passed
}
void AirScoutLink::onTimeout()
{
    if (mainWindow->getASActive())
    {
        QDateTime now = QDateTime::currentDateTime();
        QDateTime timeoutTime = lastASSEnd.addSecs(mainWindow->getASTimeout());
        if (lastASSEnd.isValid() && now > timeoutTime)
        {
            // send again...
            assetPathInProgress = false;
            askNearest(-1);
        }
    }
}
bool ASUserCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j)
{
    return i->baseCall < j->baseCall;
}
bool WatchEquals (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j)
{
    return i->baseCall == j->baseCall && i->loc == j->loc;
}
bool WatchCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j)
{
    if (i->baseCall == j->baseCall)
        return i->loc < j->loc;
    return i->baseCall < j->baseCall;
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
                qint64 res = qus->writeDatagram(packet->data(), packet->length(), addrs[j].broadcast(), static_cast<quint16>(mainWindow->getASPort()));
                if (res > 0)
                {
                    trace(QString("%1 bytes sent to %2").arg(res).arg(addrs[j].broadcast().toString()));
                    lastASSEnd = QDateTime::currentDateTime();
                    break;
                }
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
            buf.chop(1);    // get rid of checksum

            // If "nearest", analyse for this call and any planes, update userlist

            // then ask for next path, after a short delay. Look down watchlist
            // for this call, and go on to the next. If we can't find it, go back to the start

            //processZConfString(dg, host, sendBeaconResponse);
            QString sbuff = QString(buf);
            QStringList args;

            int sp = 0;
            int sp1 = sbuff.indexOf(" ");
            if (sp1 > 0)
            {
                args.append(sbuff.left(sp1));
                sp = sp1 + 1;
                sp1 = sbuff.indexOf(" ", sp);
                if (sp1 > 0)
                {
                    args.append(sbuff.mid(sp, sp1 - sp));
                    sp = sp1 + 1;
                    sp1 = sbuff.indexOf(" ", sp);
                    if (sp1 > 0)
                    {
                        args.append(sbuff.mid(sp, sp1 - sp));
                        sp = sp1 + 1;
                        args.append(sbuff.mid(sp));
                    }
                }

            }

            if (args[2] == '"' + mainWindow->getASMyName() + '"' && args[1] =='"' + mainWindow->getASServerName() + '"' && args[0] == "ASNEAREST:")
            {
                assetPathInProgress = false;
                trace ("assetPathInProgress = false;");
                if (args[3].startsWith("\""))
                {
                    args[3] = args[3].remove(0, 1);
                }
                if (args[3].endsWith("\""))
                {
                    args[3].chop(1);
                }
                QStringList sl = args[3].split(",");
                QString dxCall = sl[3];
                QString dxLoc = sl[4];

                QSharedPointer<KstUser> test(new KstUser());
                test->baseCall = dxCall;
                test->loc = dxLoc;
                QSharedPointer<KstUser> user;
                int row = 0;
                if (std::binary_search(watchList.begin(), watchList.end(), test, WatchCompare))
                {
                    row = (std::lower_bound(watchList.begin(), watchList.end(), test, WatchCompare ) - watchList.begin());

                    user = watchList.at(row);
                }
                if (user)
                {
                    user->lastCalcTime = sl[0];
                    user->fromCall = sl[1];
                    user->fromLoc = sl[2];
                    user->toCall = sl[3];
                    user->toLoc = sl[4];
                    user->planes.clear();
                    user->planeResponseSeen = true;

                    int account = sl[5].toInt();

                    int acstart = 6;

                    if (account * 5 + acstart == sl.size())
                    {

                        for (int i = 0; i < account; i++)
                        {
                            int acoffset = acstart + i * 5;

                            Aircraft ac(sl, acoffset);
                            user->planes.push_back(ac);

                        }
                    }

                    std::sort(user->planes.begin(), user->planes.end());
                    foreach(const Aircraft &ac, user->planes)
                    {
                        ac.traceAircaft();
                    }

                    emit acChanged(user);

                }
                QTimer *timer = new QTimer(this);
                timer->setSingleShot(true);

                connect(timer, &QTimer::timeout, [=]()
                {
                    trace("lambda fired");
                    // NB a lambda function
                    askNearest(row);
                    timer->deleteLater();
                }
                );

                timer->start(500);  // only ask airscout at a rate of 2 each second
            }

        }
    }
}

void AirScoutLink::usersChanged(QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector)
{
    if (mainWindow->getASActive())
    {
        watchList.clear();
        for(QVector<QSharedPointer<KstUser> >::iterator user = callVector->begin(); user != callVector->end(); user++)
        {
            if (user->data()->baseCall == mainWindow->getMyCallsign())
                continue;

            if (user->data()->baseCall.isEmpty() || user->data()->loc.isEmpty())
                continue;

            int mind = mainWindow->getASMinDistance();
            int maxd = mainWindow->getASMaxDistance();
            if (user->data()->distance < mind ||( maxd > 0 && user->data()->distance > maxd))
                continue;

            watchList.append(*user);
        }
        std::sort(watchList.begin(), watchList.end(), WatchCompare);
        watchList.erase( std::unique( watchList.begin(), watchList.end(), WatchEquals ), watchList.end() );

        QString watchFreq = bandFreqStrings[mainWindow->getASActiveBand()];        // band

        if (watchList.count() > 0)
        {
            QString watch = watchFreq;
            foreach(auto user, watchList)
            {
                QString ent = "," + user->baseCall + "," + user->loc;
                watch.append(ent);
            }

            if (watch != oldWatch)
            {
                QString watchText = /*"\"" +*/ watch /*+ "\""*/;

                sendMessage("ASWATCHLIST", watchText);
                oldWatch = watch;
            }
        }

        if (!assetPathInProgress)
            askNearest(-1);
    }
}

void AirScoutLink::asSelected(QSharedPointer<KstUser> user)
{
    if (user)
    {
        QString watchFreq = bandFreqStrings[mainWindow->getASActiveBand()];        // band
        QString getpath = /*"\""  +*/ watchFreq + ","
                + mainWindow->getMyCallsign() + "," + mainWindow->getMyLoc() + ","
                + user->baseCall + "," + user->loc /*+ "\""*/;
        sendMessage("ASSHOWPATH", getpath);
    }
}

void AirScoutLink::clearWatchList()
{
    oldWatch.clear();
}

void AirScoutLink::asShowPath(QSharedPointer<KstUser> user, QSharedPointer<KstUser> other)
{
    if (user && other)
    {
        QString watchFreq = bandFreqStrings[mainWindow->getASActiveBand()];        // band
        QString getpath = /*"\""  +*/ watchFreq + ","
                + user->baseCall+ "," + user->loc + ","
                + other->baseCall + "," + other->loc /*+ "\""*/;
        sendMessage("ASSHOWPATH", getpath);
    }
}

void AirScoutLink::askNearest(int row)
{
    if (assetPathInProgress)
        return;

    if (mainWindow->getASActive() && watchList.size())
    {
        if ((row < 0) || (++row > watchList.size() - 1))
        {
            row = 0;
        }
        QString watchFreq = bandFreqStrings[mainWindow->getASActiveBand()];        // band

        QSharedPointer<KstUser> user = watchList[row];
        QString getpath = /*"\""  +*/ watchFreq + ","
                + mainWindow->getMyCallsign() + "," + mainWindow->getMyLoc() + ","
                + user->baseCall + "," + user->loc /*+ "\""*/;
        sendMessage("ASSETPATH", getpath);
        assetPathInProgress = true;
        trace ("assetPathInProgress = true;");
    }
    else
    {
        trace ("assetPathInProgress = false;");
        assetPathInProgress = false;
    }
}
