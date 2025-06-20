#include <QDateTime>
#include "QtUtils.h"

#include "AppStartup.h"
#include "MTrace.h"
#include "dmmainwindow.h"
#include "fileutils.h"
#include "rxbuffer.h"
#include "engineconfigure.h"
#include "enginewindow.h"

#include "FLDigiFrame.h"
#include "ui_FLDigiFrame.h"

// run multi copies by setting separate config dir and title

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

    QString configDir = getDirectoryLocation(dlConfiguration) + "/DataModes/" + engineName;
    CreateDir(configDir);
    QDir dir( configDir );
    QStringList engineOpts = {
        "--xmlrpc-server-port", QString::number(EngineConfigure::getEnginePort(engineName)),
        "--wo",
        "--config-dir", dir.absolutePath(),
//        "--config-dir \"C:\\Program Files\\Fldigi-4.1.20\"",
        "-title", engineName
    };
    fldigiProcess->start(fname, engineOpts, QProcess::ReadWrite);
    fldigiActive = true;

}

FLDigiFrame::FLDigiFrame(EngineWindow *parent, QLineEdit *sendEdit, QString fname, QString name) :
    QFrame(parent),
    ui(new Ui::FLDigiFrame),
    engineWindow(parent),
    engineName(name),
    sendEdit(sendEdit),
    fname(fname)
{
    ui->setupUi(this);

    connect(mainWindow, &DMMainWindow::setSpeeds, this, &FLDigiFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::setSpeeds, this, &FLDigiFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::sendCharactersDown, this, &FLDigiFrame::onSendCharacters);
    connect(engineWindow, &EngineWindow::rigModeFreq, this, &FLDigiFrame::onRigModeFreq);
    connect(this, &FLDigiFrame::txChanged, engineWindow, &EngineWindow::onTxChanged);

    int p = EngineConfigure::getEnginePort(name);
    rpcClient = new MaiaXmlRpcClient(QUrl("http://localhost:" + QString::number(p)), this);

    createProcess();

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

void FLDigiFrame::onSetSpeeds(QString b, QString r)
{
    bpskSpeed = b;
    rttySpeed = r;

    QVariantList args;
    args.clear();
    if (bpskSpeed.contains("31"))
    {
        args << QString("BPSK31");
    }
    else
    {
        args << QString("BPSK63");
    }
    rpcClient->call("modem.set_by_name", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));
}

void FLDigiFrame::onRigModeFreq(QString m, Frequency f)
{
    QVariantList args;
    if (!f.isClear())
    {
        // FLDigi works on the genuine rig frequency
        if (m == RY)
        {
            int rttyOffset = engineWindow->getRttyOffset();
            f = f - Frequency(rttyOffset);
        }
        else if (m == PSK)
        {
            int pskOffset = engineWindow->getPSKOffset();
            f = f + Frequency(pskOffset);
        }

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
void FLDigiFrame::sendCharacters(const QString &s, int mfreq)
{
    trace(QString("FLDigi::sendCharacters %1 %2").arg(s).arg(mfreq));
    // main.tx sets tx on
    // main.rx_tx "Sets normal Rx/Tx switching."

    // main.abort kills the TX, and hopefully clears the TX buffer

    // Then there are: )which all use params 0

//    text.add_tx

    // These all apply to the text widget
//    text.add_tx_queu
//    text.add_tx_bytes
//    text.clear_tx
    QVariantList args;

    if (mfreq > 0)
    {
        if (mode == RY)
        {
            args << mfreq + RttyMSGap/2;
        }
        else
        {
            args << mfreq;
        }
        rpcClient->call("modem.set_carrier", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));
    }

    if (s.size())
    {
        args.clear();
        args << QVariant(s);
        rpcClient->call("text.add_tx_queu", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        args.clear();
        rpcClient->call("main.tx", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        addText("Tx: ");
        emit txChanged(true);
    }
    else
    {
        args.clear();
        rpcClient->call("main.abort", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));
        emit txChanged(false);
    }

}

void FLDigiFrame::sendMode(QString m)
{
    if (m == RY)
    {
        mode = RY;
        carrierOffsetFromMark = RttyMSGap/2;

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

        // How do we set RTTY speed?

    }
    if (m == PSK)
    {
        mode = PSK;
        carrierOffsetFromMark = 0;
        QVariantList args;

        args.clear();
        args << "USB";
        rpcClient->call("rig.set_mode", args,
           this, SLOT(myResponseMethod(QVariant&)),
           this, SLOT(myFaultResponse(int, const QString &)));

        args.clear();
        if (bpskSpeed.contains("31"))
        {
            args << QString("BPSK31");
        }
        else
        {
            args << QString("BPSK63");
        }
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

    trace("modem.set_by_name");

    args.clear();
    args << QString("RTTY");
    rpcClient->call("modem.set_by_name", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    trace("modem.get_name");
    rpcClient->call("modem.get_name", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

    trace("main.tx_rx");
    rpcClient->call("main.rx_tx", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));
}
void FLDigiFrame::addText(const QString &t)
{
    RXChar rxch('\n', 0, markFrequency);
    engineWindow->rxBuff.addChar(rxch);
    for(const auto &s:QASCONST(t))
    {
        RXChar rxch(s, 0, markFrequency);
        engineWindow->rxBuff.addChar(rxch);
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

            addText("Response: ");
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
            }
            RXChar rxch1('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch1);
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:QASCONST(sl))
        {
            trace(QString("Response %1").arg(s));
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
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
            //trace(QString("Carrier 1 %1").arg(s));
            markFrequency = s.toInt() - carrierOffsetFromMark;

        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:QASCONST(sl))
        {
            //trace(QString("Carrier 2 %1").arg(s));
            markFrequency = s.toInt() - carrierOffsetFromMark;
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
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
            }
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:QASCONST(sl))
        {
            trace(QString("TXResponse %1").arg(s));
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
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

            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
            }
        }
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();

        for(const auto &s:QASCONST(sl))
        {
            trace(QString("RXResponse %1").arg(s));
            RXChar rxch('\n', 0, markFrequency);
            engineWindow->rxBuff.addChar(rxch);
            for (auto c:QASCONST(s))
            {
                RXChar rxch(c, 0, markFrequency);
                engineWindow->rxBuff.addChar(rxch);
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
        QString line = fldigiProcess->readAllStandardOutput();
        trace("FLDigi:stdOut:" + line);
    }
}
