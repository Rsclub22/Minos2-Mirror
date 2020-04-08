/////////////////////////////////////////////////////////////////////////////
// $Id$
//
// PROJECT NAME 		Minos Amateur Radio Control and Logging System
//                      Rotator Control
// Copyright        (c) D. G. Balharrie M0DGB/G8FKH 2016 - 2020
//
//
// PSTRotator
//
/////////////////////////////////////////////////////////////////////////////


#include "pstRotControl.h"
#include "rotatorfactory.h"


PstRotControl::PstRotControl(QObject *parent) : RotatorBase(parent)
{
    pstCommandSocket = new QUdpSocket(this);
    pstReportSocket = new QUdpSocket(this);
    connect(pstReportSocket, SIGNAL(readyRead()), this, SLOT(processPendingReportDatagrams()));

}


PstRotControl::~PstRotControl()
{

}


void PstRotControl::register_rotators(RotatorFactory::Rotators *rotatorsList, int rotatorId)
{

    (*rotatorsList)["PSTRotator"] = RotCapabilities(rotatorId, RotCapConstants::PortType::network,
                                                           "", "PSTRotator",
                                                           false,
                                                           COMPASS_MIN0, COMPASS_MAX360,
                                                           RotCapConstants::PollData::pollDataOn,
                                                           RotCapConstants::RotatorDisplay::displayPart);




}



int PstRotControl::rotInit(srotParams &selectedAntenna)
{

    traceMsg(QString("Initialise"));

    int retCode = 0;

    closeSockets();

    setRotConnected(false);

    commsTimeoutTimer = new QTimer(this);
    connect(commsTimeoutTimer, SIGNAL(timeout()), this, SLOT(onCommsTimeout()));

    pstNetAddress = selectedAntenna.networkAdd.trimmed();
    pstCommandPortNumber = selectedAntenna.networkPort.trimmed().toUShort();
    pstReportPortNumber = pstCommandPortNumber + 1;

    pstAddress.setAddress(pstNetAddress);

    if (!pstReportSocket->bind(pstAddress, pstReportPortNumber))
    {
        // failed to bind
        retCode = BIND_FAILURE;
    }



    if (retCode >= 0)
    {

        setRotConnected(true);


    }
    else
    {

        setRotConnected(false);
    }

    return retCode;


}


void PstRotControl::closeSockets()
{
    if (pstCommandSocket->state() == QAbstractSocket::BoundState)
    {
        pstCommandSocket->close();
    }

    if (pstReportSocket->state() == QAbstractSocket::BoundState)
    {
        pstReportSocket->close();
    }
}


int PstRotControl::closeRotator()
{
    int retCode = 0;

    closeSockets();
    traceMsg(QString("Close Rotator"));
    return retCode;
}



void PstRotControl::setTraceComms(bool value)
{
    traceCommsFlag = value;
}

bool PstRotControl::getTraceComms()
{
    return traceCommsFlag;
}


void PstRotControl::processPendingReportDatagrams()
{
    commsTimeoutTimer->stop();

    QByteArray datagram;
    QString b;
    QStringList bl;
    QRegExp re("\\d*");
    do
    {
        datagram.resize(pstReportSocket->pendingDatagramSize());
        pstReportSocket->readDatagram(datagram.data(), datagram.size());
    } while (pstReportSocket->hasPendingDatagrams());

    //qDebug() << "Report = "  << datagram.data();
    b = QString(datagram.data());

    traceCommsMsg(QString("received %1 chars, message %2").arg(b.count()).arg(b));

    if (b.contains(':') && b.contains('\r'))
    {
        b.remove('\r');
        bl = b.split(':');
        if (bl.count() == 2)
        {
            if (re.exactMatch(bl[1]))
            {
                bearing = bl[1];

                emit bearing_updated(bearing.toInt());

            }
        }
    }

}



int PstRotControl::request_bearing()
{
    int retCode = 0;
    QString txMsg = QString("<PST>AZ?</PST>");
    commsTimeoutTimer->start(timeoutDur);

    traceMsg(QString("request bearing %1").arg(txMsg));
    retCode = sendCommandToPstRotator(txMsg);

    return retCode;
}


int PstRotControl::rotate_to_bearing(const int bearing)
{
    int retCode = 0;
    QString txMsg = "<PST><AZIMUTH>" + QString::number(bearing) + "</AZIMUTH></PST>";
    traceCommsMsg(QString("rotate to bearing %1").arg(txMsg));

    retCode = sendCommandToPstRotator(txMsg);

    return retCode;

}

int PstRotControl::stop_rotation()
{
    int retCode = 0;
    QString txMsg = "<PST><STOP>1</STOP></PST>";
    traceCommsMsg(QString("stop rotation %1").arg(txMsg));
    retCode= sendCommandToPstRotator(txMsg);

    return retCode;
}




QString PstRotControl::getRotLibVersion()
{
    QString ver = QString("PSTRotator");
    return ver;
}

int PstRotControl::sendCommandToPstRotator(const QString msg)
{
    QByteArray datagram = msg.toLatin1();
    qint64 bytesSent = 0;
    bytesSent = pstCommandSocket->writeDatagram(datagram, pstAddress, pstCommandPortNumber);
    if (bytesSent >= 0)
    {
        traceCommsMsg(QString("sendCommandToPstRotator Bytes Sent = %1").arg(QString::number(bytesSent)));
        return PST_OK;
    }
    else
    {
        traceCommsMsg(QString("sendCommandToPstRotator Write Datagram Error"));
        return DATAGRAM_WRITE_ERROR;
    }

}




int PstRotControl::rotateCClockwise(const int speed)
{
    Q_UNUSED(speed)
    return 0;
}
int PstRotControl::rotateClockwise(const int speed)
{
    Q_UNUSED(speed)
    return 0;
}

void PstRotControl::set_rotatorSpeed(int speed)
{
    rot_speed = speed;
}

int PstRotControl::get_rotatorSpeed()
{
    return rot_speed;
}


void PstRotControl::onCommsTimeout()
{
    commsTimeoutTimer->stop();
    traceMsg(QString("commsTimeout - %1").arg(timeoutDur));
}


void PstRotControl::traceMsg(QString msg)
{
    emit traceCommsMsg(QString("[PstRotator] %1").arg(msg));
}


void PstRotControl::traceCommsMsg(QString msg)
{
    if (traceCommsFlag)
    {
        traceMsg(msg);
    }
}

QString PstRotControl::getErrorMsgText(int errorCode)
{

}

QString PstRotControl::getLibraryName()
{
    return QString("PstRotControl");
}
