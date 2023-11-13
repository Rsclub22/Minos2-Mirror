#include <QLineEdit>
#include <QTextEdit>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>


#include "MTrace.h"
#include "delayedaction.h"
#include "dmmainwindow.h"
#include "fileutils.h"
#include "frequency.h"
#include "rxbuffer.h"
#include "enginewindow.h"
#include "engineconfigure.h"
#include "grittyframe.h"
#include "ui_grittyframe.h"

/*
You can run multiple instances of GRITTY, e.g., to decode two
audio streams in the SO2R mode. To allow each instance have its
own settings, specify the path to the settings file on the command
line when you start GRITTY

TCP Interface
Third party programs, such as loggers and contest software, can receive decoded
text and other data from GRITTY via its TCP interface.

TCP Port
By default, the interface is listening on the TCP port 7500, a different port number
 may be entered in the ini file as follows:
[Server]
Port=7501

or specified on the command line:

GRITTY.exe port=1234

The command line parameter overrides the ini setting.

Data Format
The data are exchanged in the form of JSON messages separated with linebreaks,
a.k.a. NDJSON. A client that connects to the TCP port of GRITTY receives a
 message like this:
{"MessageType":"mtSystemInfo", "Software":"GRITTY", "Version":"1.0.0.39",
    "AvailableMessageTypes":["mtSystemInfo", "mtSettings", "mtErrorMessage", "mtRawChar",
                             "mtCorrChar", "mtCallStack", "mtSMeter", "mtUserClick"],
    "AvailableCommands":["cmdRequestMessageTypes"]}


The client may send the cmdRequestMessageTypes command to enable certain types of messages.
 In the example below, the client requests the mtSMeter and mtUserClick messages:

{"Command":"cmdRequestMessageTypes","Arguments":["mtSMeter", "mtUserClick"]}

Message Types
The following messages are available in this version of GRITTY:
mtSystemInfo: sent only once when the client connects to the service.

mtSettings: sent when one of the settings changes.

mtErrorMessage: sent as necessary.

mtRawChar: sent when a new character is produced by the decoder.
The folliving values are included:

Ch: - the character;
NewLine: - if true, the character must be printed on a new line;
BitProbs: - an array of 5 bit probabilities of the character code;
Weak: - true if the character is unreliable.
mtCorrChar: smart post-processing re-estimates already decoded characters and
            replaces the them with the new values. The message includes:

Ch, NewLine, BitProbs and Weak: - as in mtRawChar;
DeleteCount - the number of already printed characters that need to be deleted
before the new ones are printed;
WordKind - one of ctCall, ctExchange, ctWord;
mtCallStack: is sent when the callsign stack changes (and sometimes when it does
 not change). Includes an array of records, each consisting of:

Call - the callsign;
CqDe - one of CQ, DE, or blank - an indicator if the station is running;
Count - the number of occurrences of the callsign in the message;
Prob - the probability that the callsign was copied correctly;
Vald - validation result, one of vrWarning, vrGood, vrPerfect, vrDta, vrWatch.
mtSMeter: reports signal to noise radio, in dB.

mtUserClick: is sent when the user clicks on a word in the received text or on a
 callsign in the stack. Includes:

Word - the word clicked;
Kind - one of ctCall, ctExchange, ctWord;
Shift - the attributes of the click, an array containing one or more of
 ssShift, ssAlt, ssCtrl, ssLeft, ssRight, ssMiddle, ssDouble;
Pos - the index of the clicked character in the returned string.

Sample messages
{"MessageType":"mtSettings", "Sideband":"LSB", "AFC":"true", "AutoQSX":"true", "Contest":"BARTG-HF"}

{"MessageType":"mtErrorMessage", "Text": "Invalid command"}

{"MessageType":"mtRawChar", "NewLine":false, "BitProbs":[0.997529447078705, 0.0559618845582008,
0.00233046314679086, 0.00407567294314504, 0.00561404135078192], "Ch":"E", "Weak":false}

{"MessageType":"mtCorrChar", "DeleteCount":0, "NewLine":false, "BitProbs":[0.997529447078705,
0.0559618845582008, 0.00233046314679086, 0.00407567294314504, 0.00561404135078192],
"Ch":"E", "Weak":false, "WordKind":"ctCall"}

{"MessageType":"mtCallStack", "Calls":[ {"Vald":"vrDta", "Prob":1, "CqDe":"CQ", "Call":"W4UK","Count":28},
 {"Vald":"vrDta", "Prob":1, "CqDe":"", "Call":"KS0M", "Count":6} ]}

{"MessageType":"mtSMeter", "ValueDb":12}

{"MessageType":"mtUserClick","Pos":0,"Word":"FP\/KV1J", "Kind":"ctCall", "Shift":["ssCtrl", "ssLeft"]}   */


// When frequency changes gritty says " QSX nnnn Hz" - on the main display
void GrittyFrame::createProcess()
{
    // we need to modify the gritty INI file with the correct details - port, USB/LSB, etc
    // and provide it as a parameter. We then need to restart gritty with a newly modified  INI file
    // when we band change

    grittyProcess = new QProcess(this);
    connect (grittyProcess, &QProcess::started, this, &GrittyFrame::on_started);
    connect (grittyProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &GrittyFrame::on_finished);
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    connect (grittyProcess, &QProcess::errorOccurred, this, &GrittyFrame::on_error);
#else
    connect (grittyProcess, &QProcess::error, this, &GrittyFrame::on_error);
#endif

    connect (grittyProcess, &QProcess::readyReadStandardError, this, &GrittyFrame::on_readyReadStandardError);
    connect (grittyProcess, &QProcess::readyReadStandardOutput, this, &GrittyFrame::on_readyReadStandardOutput);

    QString configDir = getDirectoryLocation(dlConfiguration) + "/DataModes/";
    CreateDir(configDir);
    QDir dir( configDir );

    QStringList engineOpts = {
                              "ini=" + dir.absolutePath() + "/" + engineName + ".ini",
                              "port=" + QString::number(grittyPort),
                              "caption=" + engineName};
    grittyProcess->start(fname, engineOpts, QProcess::ReadWrite);
}
GrittyFrame::GrittyFrame(EngineWindow *parent, QLineEdit *sendEdit, QString fname, QString name) :
    QFrame(parent),
    ui(new Ui::GrittyFrame),
    engineWindow(parent),
    engineName(name),
    sendEdit(sendEdit),
    fname(fname)

{
    ui->setupUi(this);
    connect(mainWindow, &DMMainWindow::setSpeeds, this, &GrittyFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::setSpeeds, this, &GrittyFrame::onSetSpeeds);
    connect(engineWindow, &EngineWindow::sendCharactersDown, this, &GrittyFrame::onSendCharacters);
    connect(engineWindow, &EngineWindow::rigModeFreq, this, &GrittyFrame::onRigModeFreq);

    grittyPort = EngineConfigure::getEnginePort(name);

    // start gritty
    createProcess();

    grittyClient = new QTcpSocket(this);

    connect(grittyClient, &QTcpSocket::connected, this, &GrittyFrame::connected);
    connect(grittyClient, &QTcpSocket::disconnected, this, &GrittyFrame::disconnected);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(grittyClient, &QTcpSocket::errorOccurred, this, &GrittyFrame::connectionError);
#else
    connect(grittyClient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
#endif
    connect(grittyClient, &QTcpSocket::readyRead, this, &GrittyFrame::onReadyRead);

    if (grittyClient->state() != QAbstractSocket::ConnectedState
       && grittyClient->state() != QAbstractSocket::ConnectingState
       && grittyClient->state() != QAbstractSocket::ClosingState
       && grittyClient->state() != QAbstractSocket::HostLookupState)
    {
        grittyClient->connectToHost("127.0.0.1", grittyPort);
    }
    grittyActive = true;
}

GrittyFrame::~GrittyFrame()
{
    delete ui;
}
void GrittyFrame::onSendCharacters(QString data, int markf)
{
    sendCharacters(data, markf);
}

void GrittyFrame::onSetSpeeds(QString b, QString r)
{
    bpskSpeed = b;
    rttySpeed = r;

}

void GrittyFrame::onRigModeFreq(QString, Frequency)
{

}
void GrittyFrame::sendCharacters(const QString &, int)
{
    // Gritty doesn't transmit
}

void GrittyFrame::sendMode(QString)
{
    // Gritty is RTTY only
}
void GrittyFrame::closeFrame()
{
    if (grittyProcess)
    {
        trace(engineName + " About to ask engine to exit");
        grittyActive = false;

        grittyClient->close();
        grittyClient->deleteLater();
        grittyClient = nullptr;

        grittyProcess->close();
        if (grittyProcess)
        {
            // if close happens quickly then on_finished will
            // clear grittyProcess
            grittyProcess->waitForFinished(1000);
        }

    }

}

//================================================================
// Process callbacks

void GrittyFrame::on_started()
{
    trace(engineName + " on process started");
}


void GrittyFrame::on_finished(int err, QProcess::ExitStatus exitStatus)
{
    trace( engineName + " on process finished:" + QString::number(err) + ":" + QString::number(exitStatus));
    if (grittyProcess)
    {
        grittyProcess->closeWriteChannel();
        grittyProcess->deleteLater();
        grittyProcess = nullptr;
    }
    if (grittyActive)
    {
        createProcess();
    }
}

void GrittyFrame::on_error(QProcess::ProcessError error)
{
    trace( engineName + " on_error:" + QString::number(error));
    grittyProcess->deleteLater();
    grittyProcess = nullptr;
}

void GrittyFrame::on_readyReadStandardError()
{
    if (grittyProcess)
    {
        QString r = grittyProcess->readAllStandardError();
        trace( engineName + " stdErr:" + r);
    }
}

void GrittyFrame::on_readyReadStandardOutput()
{
    if (grittyProcess)
    {
        QString line = grittyProcess->readLine();
        trace(engineName + " stdOut:" + line);
    }
}
//================================================================
// TCP callbacks

void GrittyFrame::connected()
{
    trace("Gritty connected");

    delayedAction(this, [=](){

        QString txString = "{\"Command\":\"cmdRequestMessageTypes\",\"Arguments\":"
        "[\"mtSystemInfo\",\"mtSettings\",\"mtErrorMessage\","
        "\"mtRawChar\",\"mtCorrChar\",\"mtCallStack\", \"mtUserClick\"]}\n";
        qint64 res = grittyClient->write(txString.toLocal8Bit());
        trace(engineName + QString("%1 bytes written; %2").arg(res).arg(txString));
    }, 250);
}

void GrittyFrame::disconnected()
{
    trace(engineName + " disconnected");
}

void GrittyFrame::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString(engineName + "Gritty Connection failed error %1").arg(error);
    trace(msg);
}

void GrittyFrame::onReadyRead()
{
    QByteArray b = grittyClient->readAll();
    QString msg = QString(b);

    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
    }
    trace(QString(engineName + " messageRx: %1").arg(traceMsg));

    // Now chop into lines and decode the JSON in each one
    msgbuf.append(msg);

    int p = msgbuf.indexOf("\n");
    while (p >= 0)
    {
        QString m = msgbuf.left(p + 1);
        msgbuf = msgbuf.mid(p + 1);
        p = msgbuf.indexOf("\n");

        analyseGrittyMessage(m);
    }

}
QString GrittyFrame::getString(QJsonObject o, QString key, QString def)
{
    QJsonValue pe = o.value(key);
    if (pe.isString())
    {
        return pe.toString();
    }
    return def;
}
bool GrittyFrame::getBool(QJsonObject o, QString key, bool def)
{
    QJsonValue pe = o.value(key);
    if (pe.isBool())
    {
        return pe.toBool();
    }
    return def;
}
int GrittyFrame::getInt(QJsonObject o, QString key, int def)
{
    QJsonValue pe = o.value(key);
    if (pe.isDouble())
    {
        return pe.toDouble();
    }
    return def;
}
void GrittyFrame::analyseGrittyMessage(QString m)
{
    //{"MessageType":"mtCorrChar","DeleteCount":0,"NewLine":true,
    //"BitProbs":[0.00258261733688414,0.998570322990417,0.993843138217926,0.0115495976060629,0.998461544513702],
    //"Ch":"P","Weak":false,"WordKind":"ctLetter"}

    QJsonParseError err;
    QJsonDocument json = QJsonDocument::fromJson(m.toUtf8(), &err);
    if (!err.error)
    {
        if (json.isObject())
        {
            QJsonObject sconf = json.object();
            QString mtype = getString(sconf, "MessageType", QString());
            if (mtype == "mtCorrChar")
            {
                int deleteCount = getInt(sconf, "DeleteCount", 0);
                bool newLine = getBool(sconf, "NewLine", false);
                QString ch = getString(sconf, "Ch", QString());
                //bool weak = getBool(sconf, "Weak", false);
                //QString wordKind = getString(sconf, "WordKind", QString());

                if (newLine)
                {
                    RXChar rxch('\n', 0, markFrequency);
                    engineWindow->rxBuff.addChar(rxch);
                }
                for (auto c:qAsConst(ch))
                {
                    // markFrequency will be zero - we don't get it from gritty
                    RXChar rxch(c, deleteCount, markFrequency);
                    engineWindow->rxBuff.addChar(rxch);
                }
                trace(engineName + " End of decode");
            }
        }
    }

}
