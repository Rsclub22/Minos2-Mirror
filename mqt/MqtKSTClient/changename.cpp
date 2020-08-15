#include "delayedaction.h"

#include "changename.h"
#include "ui_changename.h"

//==========================================================================================
// /help command via telnet

// 14:18:29.314 sendData: /help
// 14:18:29.314 sendData hex: 2f 68 65 6c 70 0d 0a
// 14:18:29.335 messageRx: Web http://www.on4kst.com
// /Help              The list of the commands available.
// /CHAT  value       Login into another chat. Values are 50 50R2 50R3 144 144R2 144R3 GHZ EME HF KHZ WARC.
// /CQ    call msg    To send a public msg seen in highlight by the callsign.
// /DX    qrg call [info] To send a DX spot.
// /SET   ANN         Allow announce messages to come out on your terminal.
// /SET   DX          Allow DX messages to come out on your terminal.
// /SET   DXCLX       Allow DX messages to come out on your terminal at CLX format.
// /SET   HERE        Tell the system you are present at your terminal.
// /SET   MYCLx value To give the cluster where to spot the DX.
// /SET   NAme value  Set your name.
// /SET   QRA value   Set your QRA Grid locator.
// /SET   QRG value   Filter the DX spots. Values are 50 70 144 432 GHZ
//                   e.g /SET QRG 50 144 to accept the 50 and 144 MHz dx spots.
// /SET   WWC         Allow World Wide Converse messages to come out on your terminal.
// /SHow  CLx         The list of the available DX clusters.
// /SHow  CONFig      Show your personal settings.
// /SHow  DX [nbr]    Get the last DX spots (QRG as your filter settings).
// /SHow  MSG [nbr]   Get the last chat messages.
// /SHow  MYCLx       To show the DX cluster where the DX spot is sent.
// /SHow  LOC value   To show the locator of a station with QRB and QTF.
// /SHow  NODes       To show the way to access to
// (message broken into parts) 14:18:29.335 messageRx:  the chat from packet radio.
// /SHow  USer [call] Show the users connected to this chat.
// /UNSET ANN         Stop announce messages coming out on your terminal.
// /UNSET DX          Stop DX messages coming out on your terminal.
// /UNSET HERE        Tell the system you are absent from your terminal.
// /UNSET QRG         Remove the QRG filter on DX spots.
// /UNSET WWC         Stop World Wide Converse messages coming out on your terminal.
// /UPDTLOC call loc  To ask to the sysop to update the locator of a station.
// /Quit              Exit from the chat.

//==========================================================================================
ChangeName::ChangeName(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ChangeName)
{
    ui->setupUi(this);
    delayedAction(this, [=]()
    {
        doChangeName();
    }
    );
}

ChangeName::~ChangeName()
{
    delete ui;
}

void ChangeName::doChangeName()
{
    // run up telnet to hange the first name.
    tnclient = new QtTelnet(this);

    connect(tnclient, SIGNAL(socketConnected()), this, SLOT(connectionEstablished()));
    connect(tnclient, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(tnclient, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
    connect(tnclient, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));

    QSettings settings;
    serverName = settings.value("tnhostname", "www.on4kst.info").toString();
    serverPort = settings.value("tnport", "23000").toString();

    connectToHost();

}

void ChangeName::sendData(QString msg)
{
    trace ("Sending telnet " + msg);
    tnclient->sendData(msg);
}
void ChangeName::connectToHost()
{
    //tnclient->login(QString("%1\r\n").arg(myCallsign), QString(password) + "\r\n");
    tnclient->connectToHost(serverName , serverPort.toInt());

}

void ChangeName::connectionEstablished()
{
    trace("connection to ON4KST established, logging in");
    tnclient->login(QString("%1\r\n").arg(myCallsign), QString(password) + "\r\n");
}

void ChangeName::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString("ON4KST telnet Connection failed error %1").arg(error);
    trace(msg);
}

void ChangeName::loggedOut()
{
    QString msg = QString("telnet Logged Out of ON4KST");
    trace(QString(msg));
    close();
}

void ChangeName::messageRx(QString msg)
{
    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
    }
    trace(QString("messageRx: %1").arg(traceMsg));

    if (setupComplete)
    {
        if (!nameChanged)
        {
            int gt = msg.indexOf(">");
            if (gt >= 0)
            {
                sendData ("/SET NAME " + newName + "\r\n");
                nameChanged = true;
            }
        }
        else
        {
            if (msg.contains(newName))
            {
                trace("logging out of telnet");
                sendData("/Q\r\n");
                tnclient->logout();
            }
        }
    }
    else
    {
        if (!userLoggedIn)
        {
            if (msg.indexOf("Chat selection") >= 0)
                 userLoggedIn = true;
        }
        if (userLoggedIn && !setupComplete)
        {
            int colon = msg.indexOf(":");
            if (colon >= 0)
            {
                sendData(QString::number(kstChatSelection) + "\r\n");
                setupComplete = true;
            }
        }
    }
}
