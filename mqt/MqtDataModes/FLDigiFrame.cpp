#include <QDateTime>

#include "MTrace.h"
#include "rigcontrolcommonconstants.h"
#include "rxbuffer.h"
#include "dmmainwindow.h"

#include "FLDigiFrame.h"
#include "ui_FLDigiFrame.h"

// look in C:\projects\fldigi-fldigi\src\network\xmlrpc.cxx
// for the RPC commands
// Big list at the end - look for

// #define METHOD_LIST

void FLDigiFrame::createProcess()
{
    // N1MM command line
    //"C:\Program Files\Fldigi-4.1.20\fldigi.exe"  --xmlrpc-server-port 7362 --wo --config-dir "C:\Program Files\Fldigi-4.1.20"  -title "Fldigi Engine 1"

    fldigiProcess = new QProcess(this);
    connect (fldigiProcess, &QProcess::started, this, &FLDigiFrame::on_started);
    connect (fldigiProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &FLDigiFrame::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect (fldigiProcess, &QProcess::errorOccurred, this, &FLDigiFrame::on_error);
#else
    connect (fldigiProcess, &QProcess::error, this, &FLDigiFrame::on_error);
#endif

    connect (fldigiProcess, &QProcess::readyReadStandardError, this, &FLDigiFrame::on_readyReadStandardError);
    connect (fldigiProcess, &QProcess::readyReadStandardOutput, this, &FLDigiFrame::on_readyReadStandardOutput);

    QStringList engineOpts = {"--wo"/*, "--home-dir C:/temp"*/};
    fldigiProcess->start(fname, engineOpts, QProcess::ReadWrite);
}

FLDigiFrame::FLDigiFrame(QWidget *parent, QLineEdit *sendEdit, QString fname) :
    QFrame(parent),
    ui(new Ui::FLDigiFrame),
    sendEdit(sendEdit),
    fname(fname)
{
    ui->setupUi(this);

    connect(mainWindow, &DMMainWindow::sendCharacters, this, &FLDigiFrame::onSendCharacters);
    connect(mainWindow, &DMMainWindow::rigModeFreq, this, &FLDigiFrame::onRigModeFreq);

    rpcClient = new MaiaXmlRpcClient(QUrl("http://localhost:7362"), this);

    createProcess();

    fldigiActive = true;

    getTimer = new QTimer(this);
    connect(getTimer, &QTimer::timeout, this, &FLDigiFrame::onGetTimer);
    getTimer->start(500);

}

FLDigiFrame::~FLDigiFrame()
{
    delete ui;
    delete rpcClient;
}
void FLDigiFrame::onSendCharacters(QString data, int c)
{
    sendCharacters(data, c);
}

void FLDigiFrame::onRigModeFreq(QString m, Frequency f)
{
    QVariantList args;
    if (!f.isClear())
    {
        args << f.str();

        rpcClient->call("rig.set_frequency", args,
           this, SLOT(myRxResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));
    }
    sendMode(m);
}
void FLDigiFrame::onGetTimer()
{
    QVariantList args;


    rpcClient->call("modem.get_carrier", args,
       this, SLOT(myCarrierResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    rpcClient->call("rx.get_data", args,
       this, SLOT(myRxResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    rpcClient->call("tx.get_data", args,
       this, SLOT(myTxResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));
}
void FLDigiFrame::sendCharacters(const QString &s, int carrier)
{
    trace(QString("FLDigi::sendCharacters %1 %2").arg(s).arg(carrier));
    // main.tx sets tx on
    // main.rx_tx "Sets normal Rx/Tx switching."

    // main.abort kills the TX

    // Then there are: )which all use params 0

//    text.add_tx

    // These all apply to the text widget
//    text.add_tx_queu
//    text.add_tx_bytes
//    text.clear_tx
    QVariantList args;

    if (carrier > 0)
    {
        if (mode == hamlibData::RY)
        {
            args << carrier + 170/2;
        }
        else
        {
            args << carrier;
        }
        rpcClient->call("modem.set_carrier", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));
    }

    if (s.size())
    {
        args.clear();
        args << s.toLatin1().data();
        rpcClient->call("text.add_tx_queu", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        args.clear();
        rpcClient->call("main.tx", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        addText("Tx: ");
    }
    else
    {
        args.clear();
        rpcClient->call("main.abort", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

    }

}

void FLDigiFrame::sendMode(QString m)
{
    if (m == hamlibData::RY)
    {
        mode = hamlibData::RY;
        carrierOffset = 170/2;

        QVariantList args;

        args.clear();
        args << "LSB";
        rpcClient->call("rig.set_mode", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        args.clear();
        args << QString("RTTY");
        rpcClient->call("modem.set_by_name", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

    }
    if (m == hamlibData::PSK)
    {
        mode = hamlibData::PSK;
        carrierOffset = 0;
        QVariantList args;

        args.clear();
        args << "USB";
        rpcClient->call("rig.set_mode", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        args.clear();
        args << QString("BPSK63");
        rpcClient->call("modem.set_by_name", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

    }
}

void FLDigiFrame::closeFrame()
{
    if (fldigiProcess)
    {
        trace("About to ask engine to exit");
//        ::PostMessage(mttyHWnd, uMSG_MMTTY, RXM_EXIT, 0);
        fldigiActive = false;

        QVariantList args;
        args << 7;

        rpcClient->call("fldigi.terminate", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        fldigiProcess->waitForFinished(1000);

    }

}
void FLDigiFrame::on_started()
{
    trace("FLDigi:on_started");

    QVariantList args;

    args.clear();
    args << "USB";
    rpcClient->call("rig.set_mode", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    args.clear();
    args << QString("RTTY");
    rpcClient->call("modem.set_by_name", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    rpcClient->call("main.rx_tx", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));
}
void FLDigiFrame::addText(const QString &t)
{
    bool newLine = true;
    for(const auto &s:qAsConst(t))
    {
        RXChar rxch(s, newLine, 0, carrier);
        newLine = false;
        RxBuffer::getRxBuffer()->addChar(rxch);
    }

}


void FLDigiFrame::myResponseMethod(QVariant &v)
{
    //int type = v.userType();
    if (v.canConvert<QString>())
    {
        QString s = v.toString();

        if (!s.isEmpty())
        {
            trace(QString("Response %1").arg(s));

            bool nl = true;
            addText("Response: ");
            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, nl, 0, carrier);
                nl = false;
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
            RXChar rxch(' ', true, 0, carrier);
            RxBuffer::getRxBuffer()->addChar(rxch);
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:qAsConst(sl))
        {
            trace(QString("Response %1").arg(s));
            bool newLine = true;
            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, newLine, 0, carrier);
                newLine = false;
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
        }
    }
}

void FLDigiFrame::myCarrierResponseMethod(QVariant &v)
{
    if (v.canConvert<QString>())
    {
        QString s = v.toString();

        if (!s.isEmpty())
        {
            trace(QString("Carrier 1 %1").arg(s));
            carrier = s.toInt() - carrierOffset;

        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:qAsConst(sl))
        {
            trace(QString("Carrier 2 %1").arg(s));
            carrier = s.toInt() - carrierOffset;
        }
    }
}

void FLDigiFrame::myTxResponseMethod(QVariant &v)
{
    if (v.canConvert<QString>())
    {
        QString s = v.toString();

        if (!s.isEmpty())
        {
            trace(QString("TXResponse %1").arg(s));
            bool newLine = true;
            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, newLine, 0, carrier);
                newLine = false;
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:qAsConst(sl))
        {
            trace(QString("TXResponse %1").arg(s));
            bool newLine = true;
            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, newLine, 0, carrier);
                newLine = false;
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
        }
    }
}
void FLDigiFrame::myRxResponseMethod(QVariant &v)
{
    if (v.canConvert<QString>())
    {
        QString s = v.toString();

        if (!s.isEmpty())
        {
            trace(QString("RXResponse %1").arg(s));

            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, false, 0, carrier);
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:qAsConst(sl))
        {
            trace(QString("RXResponse %1").arg(s));
            bool newLine = true;
            for (auto c:qAsConst(s))
            {
                RXChar rxch(c, newLine, 0, carrier);
                newLine = false;
                RxBuffer::getRxBuffer()->addChar(rxch);
            }
        }
    }

}

void FLDigiFrame::myFaultResponse(int n, const QString &s)
{
    trace(QString("Error %1 : %2").arg(n).arg(s));
}

void FLDigiFrame::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace( "FLDigi:on_finished:" + QString::number(err) + ":" + QString::number(exitStatus));
    if (fldigiProcess)
    {

        fldigiProcess->closeWriteChannel();
        fldigiProcess->deleteLater();
        fldigiProcess = nullptr;
    }
    if (fldigiActive)
    {
        createProcess();
    }
}

void FLDigiFrame::on_error(QProcess::ProcessError error)
{
    trace( "FLDigi:on_error:" + QString::number(error));
    fldigiProcess->deleteLater();
    fldigiProcess = nullptr;
}

void FLDigiFrame::on_readyReadStandardError()
{
    if (fldigiProcess)
    {
        QString r = fldigiProcess->readAllStandardError();
        trace( "FLDigi:stdErr:" + r);
    }
}

void FLDigiFrame::on_readyReadStandardOutput()
{
    if (fldigiProcess)
    {
        QString line = fldigiProcess->readLine();
        trace("FLDigi:stdOut:" + line);
    }
}
