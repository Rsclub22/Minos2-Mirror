#include "rigctldclient.h"
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
    socket = new QTcpSocket(this);
    //socket->open(QIODevice::ReadWrite);

    connect(socket, SIGNAL(connected()),this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()),this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
}

bool RigCtldClient::connectToHost(QString host, quint16 port)
{
    socket->connectToHost(host, port);
    return socket->waitForConnected();
}


void RigCtldClient::disconnectFromHost()
{
    socket->disconnectFromHost();

}

bool RigCtldClient::writeData(const QByteArray &data)
{
    qDebug() << "sending = " << data;
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //socket->write(IntToArray(data.size())); //write size of data
        socket->write(data, qstrlen(data)); //write the data itself
        qDebug() << "data being sent";
        return socket->waitForBytesWritten();
    }
    else
    {
        return false;
    }
}




void RigCtldClient::connected()
{
    qDebug() << "connected...";
    clientConnected = true;

}

void RigCtldClient::disconnected()
{
    qDebug() << "disconnected...";
    clientConnected = false;
    msg.clear();
}

void RigCtldClient::readyRead()
{
    qDebug() << "reading...";
    msgComplete = false;
    recvTimer = new QTimer(this);
    recvTimer->start(3000);

    // read the data from the socket
    //qDebug() << socket->readAll();

    QString line;
    do
    {
        line = socket->readLine();
        if (!line.isEmpty())
        {
             msg.append(line);
        }

    }while ((!line.contains("RPRT")) && recvTimer->isActive());

    retCode = getErrorCode(line);
    if (retCode >= 0 && recvTimer->isActive())
    {
        msgComplete = true;
        qDebug() << "got message ok";
    }
    else
    {
        msgComplete = false;
        qDebug() << "message failed";
    }

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
