#include <QDebug>
#include <QDateTime>

#include "MTrace.h"

#include "FLDigiFrame.h"
#include "ui_FLDigiFrame.h"

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

FLDigiFrame::FLDigiFrame(QWidget *parent,  QTextEdit *rxChars, QLineEdit *sendEdit, QString fname) :
    QFrame(parent),
    ui(new Ui::FLDigiFrame),
    rxChars(rxChars),
    sendEdit(sendEdit),
    fname(fname)
{
    ui->setupUi(this);

    rpcClient = new MaiaXmlRpcClient(QUrl("http://localhost:7362"), this);

    createProcess();

    fldigiActive = true;


}

FLDigiFrame::~FLDigiFrame()
{
    delete ui;
    delete rpcClient;
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

//    rpcClient->call("fldigi.name_version", args,
//       this, SLOT(myResponseMethod(QVariant&)),
//       this, SLOT(myFaultResponse(int, const QString &)));

//    rpcClient->call("main.rx", args,
//       this, SLOT(myResponseMethod(QVariant&)),
//       this, SLOT(myFaultResponse(int, const QString &)));

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

    args.clear();
    rpcClient->call("rx.get_data", args,
       this, SLOT(myResponseMethod(QVariant&)),
       this, SLOT(myFaultResponse(int, const QString &)));

}

void FLDigiFrame::myResponseMethod(QVariant &v)
{
    //int type = v.userType();
    if (v.canConvert<QString>())
    {
        QString s = v.toString();

        trace(QString("Response %1").arg(s));
    }
    else if (v.canConvert<QStringList>())
    {
        QStringList sl = v.toStringList();
        for(auto s:qAsConst(sl))
        {
            trace(QString("Response %1").arg(s));
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
