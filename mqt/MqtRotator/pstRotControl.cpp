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
#include "minosNetUtils.h"

const char * PstRotControl::pstRotatorErrorMsg[] = { QT_TR_NOOP("PSTRotator Command OK"),
                                                     QT_TR_NOOP("Network Address failed to bind"),
                                                     QT_TR_NOOP("Datagram Write Error"),
                                                     QT_TR_NOOP("Request Bearing Command Timeout"),
                                                     QT_TR_NOOP("Rotate to Bearing Command Timeout"),
                                                     QT_TR_NOOP("Stop Commond Timeout"),


                                                    };



PstRotControl::PstRotControl(QObject *parent) : RotatorBase(parent)
{
    pstCommandSocket = new QUdpSocket(this);
    pstReportSocket = new QUdpSocket(this);
    connect(pstReportSocket, SIGNAL(readyRead()), this, SLOT(processPendingReportDatagrams()));
    cmdSent = pstCmdSent::NO_CMDSENT;

}


PstRotControl::~PstRotControl()
{

}


void PstRotControl::register_rotators(RotatorFactory::Rotators *rotatorsList, int rotatorId)
{

    (*rotatorsList)["PSTRotator"] = RotCapabilities(rotatorId, RotCapConstants::PortType::network,
                                                           "", "PSTRotator",
                                                           false,
                                                           true,        // support stop command
                                                           COMPASS_MIN0,
                                                           COMPASS_MAX360,
                                                           RotCapConstants::PollData::pollDataOn,
                                                           RotCapConstants::SelectDisplayCompass::enableSelectDisplayDial,
                                                           false);




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
    //if (pstNetAddress.isEmpty() || isHostLocal(pstNetAddress))
    //{
    //    pstNetAddress = "127.0.0.1";
    //}

    //if (selectedAntenna.networkPort.trimmed().isEmpty())
    //{
        pstCommandPortNumber = 12000;
    //}
   // else
    //{
        pstCommandPortNumber = selectedAntenna.networkPort.trimmed().toUShort();

   // }

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
    setRotConnected(false);

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
    cmdSent = pstCmdSent::NO_CMDSENT;

    QByteArray datagram;
    QString b;
    QStringList bl;

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
             bool ok;
             float bearingFl = bl[1].toFloat(&ok);
             if (ok)
             {

                 int bearing = static_cast<int>(bearingFl);
                 traceCommsMsg(QString("extracted bearing = %1 ok, send to dial").arg(bearing));
                 emit bearing_updated(bearing);

             }


        }
        else
        {
            traceCommsMsg(QString("error splitting received message"));
        }
    }

}



int PstRotControl::request_bearing()
{
    int retCode = 0;
    if (cmdSent == pstCmdSent::NO_CMDSENT)
    {
        cmdSent = pstCmdSent::REQ_BEARING_CMDSENT;
        QString txMsg = QString("<PST>AZ?</PST>");
        commsTimeoutTimer->start(timeoutDur);

        traceMsg(QString("request bearing %1").arg(txMsg));
        retCode = sendCommandToPstRotator(txMsg);
    }


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




void PstRotControl::setRotConnected(bool connectFlag)
{
    rotConnected = connectFlag;
}


bool PstRotControl::getRotConnected()
{
    return rotConnected;
}

void PstRotControl::onCommsTimeout()
{
    commsTimeoutTimer->stop();
    traceMsg(QString("commsTimeout - %1, errorcode %2").arg(timeoutDur).arg(cmdSent));
    emit sentCommandError(cmdSentErrorCodes[cmdSent], cmdSentString[cmdSent]);
    cmdSent = pstCmdSent::NO_CMDSENT;
}


void PstRotControl::traceMsg(QString msg)
{
    trace(QString("[PstRotator] %1").arg(msg));
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
    if (errorCode > static_cast<int>(sizeof(pstRotatorErrorMsg)/sizeof(const char *)))
    {
        return tr("pstRotator Errorcode too large!");
    }
    return tr(pstRotatorErrorMsg[errorCode]);
}

QString PstRotControl::getLibraryName()
{
    return QString(PSTROTATOR_API);
}


