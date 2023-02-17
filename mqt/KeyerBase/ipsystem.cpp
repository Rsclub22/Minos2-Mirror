#include <QApplication>
#include <QDateTime>
#include <QDataStream>
#include <QUdpSocket>
#include <QTimer>

#include <QDebug>

#include "MTrace.h"
#include "inbuff.h"
#include "databuffer.h"
#include "ipsystem.h"

/*static*/ IPSystem *IPSystem::createIPSystem()
{
    return new IPSystem();
}

IPSystem::IPSystem()
{

}

IPSystem::~IPSystem()
{}

void IPSystem::initialise(bool pds, IPADataBuffer *db, QHostAddress phost, int16_t pport)
{
    dataBuffer = db;
    ds = pds;
    host = phost;
    port = pport;
}

void IPSystem::onReadyRead()
{
    // This appears to be in the GUI thread
    while(bc && bc->bytesAvailable())
    {
        receiving = true;
        QDataStream sockStream(bc);
        sockStream.setVersion(QDataStream::Qt_5_2);

        if(_pendingPacketSize == 0)
        {
            if(bc->bytesAvailable() < static_cast<int>(sizeof(_pendingPacketSize)))
                return;

            sockStream >> _pendingPacketSize;
        }

        if(bc->bytesAvailable() < _pendingPacketSize)
            return;

        if (!ds)
        {
            // and process

            BuffHeader buffHeader;
            sockStream >> buffHeader;

            QByteArray buf;
            sockStream >> buf;

            // As we are now TCP we should never get out of sequence packets
 //           if (buffHeader.sequence > sequence)
            {
                sequence = buffHeader.sequence;
                InBuff *inBuff = dataBuffer->getNextInputBuffer();
                if (inBuff)
                {
                    inBuff->bh = buffHeader;
                    memcpy(reinterpret_cast<char *>(inBuff->buff), buf, buffHeader.frameCount * 4);
                    dataBuffer->unlockNextInput();
                }
            }
            emit sequenceCount(buffHeader.sequence);
        }
        else
        {
            // nothing yet!
        }

        _pendingPacketSize = 0;
    }
}
void IPSystem::onBytesWritten(qint64 /*cnt*/)
{
    // There is also a bytesToWrite call
    emit sequenceCount(sequence);

}

void IPSystem::handleSocketAcceptError()
{
    trace("SocketAcceptError");
}

void IPSystem::dostop()
{
    terminated = true;
}

bool IPSystem::isListening() const
{
    return listening;
}

void IPSystem::setListening(bool newListening)
{
    listening = newListening;
}
void IPSystem::onNewConnection()
{
    QTcpSocket *s = sv->nextPendingConnection();

    if (s)
    {
        setListening(false);
        if (bc)
        {
            s->close();
            s->deleteLater();
        }
        else
        {
            bc = s;

            connect(bc, &QAbstractSocket::readyRead, this, &IPSystem::onReadyRead);
            connect(bc, &QTcpSocket::connected, this, &IPSystem::connected);
            connect(bc, &QTcpSocket::disconnected, this, &IPSystem::disconnected);
        #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            connect(bc, &QTcpSocket::errorOccurred, this, &IPSystem::connectionError);
        #else
            connect(bc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(IPSystem(QAbstractSocket::SocketError)));
        #endif

            connect(bc, &QAbstractSocket::bytesWritten, this, &IPSystem::onBytesWritten);
        }
    }
}
bool IPSystem::listen()
{
    if(isListening())
        return true;

    setListening(true);
    if(!sv)
    {
        sv = new QTcpServer();
        connect(sv, &QTcpServer::acceptError, this, &IPSystem::handleSocketAcceptError);

        connect(sv,
                &QTcpServer::newConnection,
                this,
                &IPSystem::onNewConnection);
    }

    return sv->listen(host, port);
}
void IPSystem::doStart()
{
    terminated = false;

    if (ds)
    {
        listen();
    }
    else if (!host.isNull())
    {
        if (bc)
        {
            bc->deleteLater();
            bc = nullptr;
        }
        trace(QString("Connecting to h %1 p%2").arg(host.toString()).arg(port));
        bc = new QTcpSocket();
        connect(bc, &QAbstractSocket::readyRead, this, &IPSystem::onReadyRead);
        connect(bc, &QTcpSocket::connected, this, &IPSystem::connected);
        connect(bc, &QTcpSocket::disconnected, this, &IPSystem::disconnected);
    #if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        connect(bc, &QTcpSocket::errorOccurred, this, &IPSystem::connectionError);
    #else
        connect(bc, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(IPSystem(QAbstractSocket::SocketError)));
    #endif
        bc->connectToHost(host, port);
    }
}
void IPSystem::connected()
{
    trace("Socket connected");
    // all OK, nothing to do here

    sending = true;
}
void IPSystem::disconnected()
{
    // connection failed; restart it

    trace("Socket disconnected");

    if (bc)
    {
        bc->deleteLater();
        bc = nullptr;
    }

    doStart();
}

void IPSystem::connectionError(QAbstractSocket::SocketError /*error*/)
{
    trace("Socket error: " + bc->errorString());
    // an error occurred. Close down current process, restart
    if (bc)
    {
        bc->deleteLater();
        bc = nullptr;
    }
    doStart();
}

int IPSystem::tryOutput()
{
    qint64 nbytes = 0;

    InBuff *inBuff = dataBuffer->getNextOutputBuffer();
    if (inBuff)
    {
        if (!terminated && ds && bc)
        {
            if (bc->state() == QAbstractSocket::ConnectedState)
            {
                QByteArray nm;
                QDataStream os(&nm, QIODevice::WriteOnly);
                os.setVersion (QDataStream::Qt_5_2);

                QByteArray buff(reinterpret_cast<char *>(inBuff->buff), inBuff->bh.frameCount * 4);

                qint16 blen = 0;
                os << blen;

                BuffHeader buffHeader;
                buffHeader.frameCount = inBuff->bh.frameCount;
                buffHeader.ptt = inBuff->bh.ptt;
                buffHeader.rms = inBuff->bh.rms;
                buffHeader.peak = inBuff->bh.peak;
                buffHeader.sequence = sequence++;
                buffHeader.tnow = QDateTime::currentMSecsSinceEpoch();
                os << buffHeader;

                os << buff;
                os.device()->seek(0);
                os << static_cast<qint16>(nm.size() - sizeof(qint16));

                sends++;
                nbytes = bc->write(nm);
                if (nbytes == 0)
                {
                    qDebug() << bc->errorString() + " - No data sent\n";
                    return 0;
                }
                else if (nbytes < 0)
                {
                    qDebug() << "Write failed " + bc->errorString() + "\n";
                    return -1;  // will kill the thread
                }

            }
        }
        dataBuffer->unlockNextOutput();
    }
    return nbytes;
}

void IPSystem::closedown()
{
    if (bc)
    {
        bc->deleteLater();
        bc = nullptr;
    }
    if (sv)
    {
        sv->deleteLater();
        sv = nullptr;
    }

}
