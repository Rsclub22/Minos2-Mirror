#include "rigctldclient.h"
#include "base_pch.h"
#include <QDebug>

/*
 * Currently this is specific to obaining caps dump
 * from the ritctld client.
*/

static inline QByteArray IntToArray(qint32 source);

RigCtldClient::RigCtldClient(QObject *parent) :
    QObject(parent)
    ,retCode(0)
    ,msgComplete(false)
    ,clientConnected(false)
{
    trace(QString("RigCtldClient - Creating Client"));
    socket = new QTcpSocket(this);
    //socket->open(QIODevice::ReadWrite);

    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

bool RigCtldClient::connectToHost(QString host, quint16 port)
{
    trace(QString("RigCtldClient - Connecting to Host - %1:%2").arg(host).arg(QString::number(port)));
    socket->connectToHost(host, port);
    return socket->waitForConnected();
}


void RigCtldClient::disconnectFromHost()
{
    trace(QString("RigCtldClient - Disconnecting from host"));
    socket->disconnectFromHost();

}

bool RigCtldClient::writeData(const QByteArray &data)
{

    trace(QString("RigCtldClient - Sending Data"));
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //socket->write(IntToArray(data.size())); //write size of data
        socket->write(data, qstrlen(data)); //write the data itself

        return socket->waitForBytesWritten();
    }
    else
    {
        trace(QString("RigCtldClient - Sending Data failed - Not connected"));
        return false;
    }
}




void RigCtldClient::connected()
{
    trace(QString("RigCtldClient - Connected"));
    clientConnected = true;

}

void RigCtldClient::disconnected()
{
    trace(QString("RigCtldClient - Disconnected"));
    clientConnected = false;
    msg.clear();
}

void RigCtldClient::readyRead()
{
    trace(QString("RigCtldClient - Reading Data"));
    msgComplete = false;

    QString line;

    qint64 byteAvail = socket->bytesAvailable();
    trace(QString("RigCtldClient - Bytes Available = %1").arg(QString::number(byteAvail)));
    while (socket->canReadLine())
    {
        line = QString(socket->readLine());
        qDebug() << line;
        bytes += line.count();
        msg.append(line);
    }

    // reached the end of message?
    if (!line.contains("RPRT"))
    {
        // no wait
        return;
    }

    retCode = getErrorCode(line);
    if (retCode >= 0)
    {
        msgComplete = true;
        trace(QString("RigCtldClient - Message Completed OK - %1").arg(bytes));
    }
    else
    {
        msgComplete = false;
        trace(QString("RigCtldClient - Message Failed - %1").arg(bytes));
    }

    trace(QString("RigCtldClient - Finished Signal"));
    emit finished();

}

int RigCtldClient::getRetCode()
{
    return retCode;
}


bool RigCtldClient::checkMsgRecieved()
{
    return msgComplete;
}

bool RigCtldClient::checkConnected()
{
    return clientConnected;
}

QString RigCtldClient::findItemMessage(QString item)
{
    QString match;
    for (int i = 0; i < msg.count(); i++)
    {
        if (msg[i].contains(item))
        {
            match = msg[i];
            break;
        }
    }
    return match;
}

QString RigCtldClient::getRadioModel()
{
    QString model = findItemMessage("dump for model");
    if (model == "")
    {
        return model;
    }

    QStringList ml = model.split(':');
    if (ml.count() == 2)
    {
        return ml[1].trimmed().remove('\x20');
    }

    return "";

}

QString RigCtldClient::getRadioModelName()
{
    QString name = findItemMessage("Model name");
    if (name == "")
    {
        return name;
    }

    QStringList nl = name.split(':');
    if (nl.count() == 2)
    {
        return nl[1].trimmed();
    }
    else
    {
        return "";
    }
}

QString RigCtldClient::getRadioManufacturerName()
{
    QString name = findItemMessage("Mfg name");
    if (name == "")
    {
        return name;
    }

    QStringList nl = name.split(':');
    if (nl.count() == 2)
    {
        return nl[1].trimmed();
    }
    else
    {
        return "";
    }
}

int RigCtldClient::getErrorCode(QString msg)
{

    if (msg.contains("RPRT"))
    {
        QStringList ml = msg.split('\x20');
        bool ok;
        if (ml.count() == 2)
        {
            int errorCode = ml[1].trimmed().toInt(&ok, 10);
            if (ok)
            {
                return errorCode;
            }

        }
    }

    return -51;

}

QByteArray IntToArray(qint32 source) //Use qint32 to ensure that the number have 4 bytes
{
    //Avoid use of cast, this is the Qt way to serialize objects
    QByteArray temp;
    QDataStream data(&temp, QIODevice::ReadWrite);
    data << source;
    return temp;
}
