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

    //pstNetAddress("127.0.0.1"),
    //pstCommandPortNumber(12000),
    //pstReportPortNumber(12001)
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

    (*rotatorsList)["PSTRotator"] = RotCapabilities(rotatorId, RotCapContstants::PortType::network,
                                                           "", "PSTRotator",
                                                           false,
                                                           COMPASS_MIN0, COMPASS_MAX360,
                                                           RotCapContstants::PollData::pollDataOn,
                                                           RotCapContstants::RotatorDisplay::displayPart);




}



int PstRotControl::rotInit(srotParams &selectedAntenna)
{

    int retCode = 0;

    closeSockets();

    setRotConnected(false);

    pstNetAddress = selectedAntenna.networkAdd.trimmed();
    pstCommandPortNumber = selectedAntenna.networkPort.trimmed().toUShort();
    pstReportPortNumber = pstCommandPortNumber + 1;

    pstAddress.setAddress(pstNetAddress);

    pstReportSocket->bind(pstAddress, pstReportPortNumber);



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

    return retCode;
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
                emit bearing_updated(bearing.toInt());
            }
        }
    }

}



int PstRotControl::request_bearing()
{
    int retCode = 0;
    sendCommandToPstRotator("<PST>AZ?</PST>");

    return retCode;
}


int PstRotControl::rotate_to_bearing(const int bearing)
{
    int retCode = 0;
    QString msg = "<PST><AZIMUTH>" + QString::number(bearing) + "</AZIMUTH></PST>";
    sendCommandToPstRotator(msg);
    return retCode;

}

int PstRotControl::stop_rotation()
{
    int retCode = 0;
    QString msg = "<PST><STOP>1</STOP></PST>";
    sendCommandToPstRotator(msg);
    return retCode;
}


void PstRotControl::enableTraceComms(bool state)
{
    traceComms = state;
}

QString PstRotControl::getRotLibVersion()
{
    QString ver = QString("PSTRotator");
    return ver;
}

void PstRotControl::sendCommandToPstRotator(const QString msg)
{
    QByteArray datagram = msg.toLatin1();

    pstCommandSocket->writeDatagram(datagram, pstAddress, pstCommandPortNumber);

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

