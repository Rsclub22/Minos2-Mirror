#include "pstRotControl.h"


PstRotControl::PstRotControl(QObject *parent) :
    QObject(parent),
    pstNetAddress("127.0.0.1"),
    pstCommandPortNumber(12000),
    pstReportPortNumber(12001)
{
    pstCommandSocket = new QUdpSocket(this);
    pstReportSocket = new QUdpSocket(this);
    connect(pstReportSocket, SIGNAL(readyRead()), this, SLOT(processPendingReportDatagrams()));

}


void PstRotControl::initPstSockets()
{

    if (pstCommandSocket->state() == QAbstractSocket::BoundState)
    {
        pstCommandSocket->close();
    }

    if (pstReportSocket->state() == QAbstractSocket::BoundState)
    {
        pstReportSocket->close();
    }

    pstAddress.setAddress(pstNetAddress);

    pstReportSocket->bind(pstAddress, pstReportPortNumber);




}


void PstRotControl::setPstNetAddress(QString address)
{
    pstNetAddress = address;
}

void PstRotControl::setPstPortAddress(QString port)
{
    pstCommandPortNumber = port.toUShort();
    pstReportPortNumber = pstCommandPortNumber + 1;

}


void PstRotControl::processPendingReportDatagrams()
{
    QByteArray datagram;
    QString b;
    QStringList bl;
    QRegExp re("\\d*");
    do
    {
        datagram.resize(pstReportSocket->pendingDatagramSize());
        pstReportSocket->readDatagram(datagram.data(), datagram.size());
    } while (pstReportSocket->hasPendingDatagrams());

    qDebug() << "Report = "  << datagram.data();
    b = QString(datagram.data());
    if (b.contains(':') && b.contains('\r'))
    {
        b.remove('\r');
        bl = b.split(':');
        if (bl.count() == 2)
        {
            if (re.exactMatch(bl[1]))
            {
                bearing = bl[1];
                emit pstBearing(bearing.toInt());
            }
        }
    }

}



void PstRotControl::sendRequestBearing()
{
    sendCommandToPstRotator("<PST>AZ?</PST>");
}


void PstRotControl::sendRotateTo(const QString bearing)
{
    QString msg = "<PST><AZIMUTH>" + bearing + "</AZIMUTH></PST>";
    sendCommandToPstRotator(msg);

}

void PstRotControl::sendStop()
{
    QString msg = "<PST><STOP>1</STOP></PST>";
    sendCommandToPstRotator(msg);
}

void PstRotControl::sendCommandToPstRotator(const QString msg)
{
    QByteArray datagram = msg.toLatin1();

    pstCommandSocket->writeDatagram(datagram, pstAddress, pstCommandPortNumber);

}
