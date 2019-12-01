#include <QHostInfo>
#include <QSettings>

#include "cutils.h"

#include "kstconfigure.h"

#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"

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
KSTMainWindow::KSTMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::KSTMainWindow)
{
    ui->setupUi(this);

    QSettings settings;
    QByteArray geometry = settings.value("geometry/Main").toByteArray();
    if (geometry.size() > 0)
        restoreGeometry(geometry);

    QByteArray state;
    state = settings.value("kstSplitterState").toByteArray();
    ui->kstSplitter->restoreState(state);

    state = settings.value("msgSplitterState").toByteArray();
    ui->msgSplitter->restoreState(state);

    messageVector = QSharedPointer<QVector <QSharedPointer<KstMessageLine> > >( new QVector<QSharedPointer<KstMessageLine> >);

    kstMessageModel.setChatVector(messageVector);

    kstMessageFilterModel.setSourceModel(&kstMessageModel);

    ui->messageTable->setModel(&kstMessageFilterModel);
    ui->messageTable->horizontalHeader()->setStretchLastSection(true);

    kstMeepModel.setChatVector(messageVector);

    kstMeepFilterModel.setSourceModel(&kstMeepModel);
    ui->meepTable->setModel(&kstMeepFilterModel);
    ui->meepTable->horizontalHeader()->setStretchLastSection(true);

    callVector =    QSharedPointer<QVector <QSharedPointer<KstUser> > >( new QVector<QSharedPointer<KstUser> > );
    kstCallModel.setCallVector(callVector);

    kstCallFilterModel.setSourceModel(&kstCallModel);
    ui->CSTable->setModel(&kstCallFilterModel);

    ui->CSTable->horizontalHeader()->setStretchLastSection(true);

    meepDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    messageDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    CSDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;

    kstMeepModel.delegate = meepDelegate;
    kstMessageModel.delegate = messageDelegate;
    kstCallModel.delegate = CSDelegate;

    ui->meepTable->setItemDelegate(meepDelegate.data());
    ui->messageTable->setItemDelegate(messageDelegate.data());
    ui->CSTable->setItemDelegate(CSDelegate.data());

    QHeaderView *verticalHeader = ui->meepTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setVisible(false);

    QSize ms = messageDelegate->docSize("XX");
    verticalHeader->setDefaultSectionSize(ms.height());
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);

    verticalHeader = ui->CSTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);



    state = settings.value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);

    state = settings.value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    state = settings.value("meepTable/state").toByteArray();
    ui->meepTable->horizontalHeader()->restoreState(state);

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->messageTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->meepTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);

    tnclient = new QtTelnet(this);

    connect(tnclient, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    //connect(tnclient, SIGNAL(connected(bool)), this, SLOT(connected(bool)));
    connect(tnclient, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(tnclient, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(tnclient, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
    connect(tnclient, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));

    serverName = settings.value("hostname", "www.on4kst.info").toString();
    serverPort = settings.value("port", "23000").toString();
    myCallsign = settings.value("username", "").toString();
    password = settings.value("password", "").toString();
    kstChatSelection = settings.value("service", "1").toString();
    autoConnect = settings.value("autoConnect", false).toBool();

    QStringList services =
    {"50/70 MHz..............1",
    "144/432 MHz............2",
    "Microwave..............3",
    "EME/JT65...............4",
    "Low Band...............5",
    "50 MHz IARU Region 3...6",
    "50 MHz IARU Region 2...7",
    "144/432 MHz IARU R 2...8",
    "144/432 MHz IARU R 3...9",
    "kHz (2000-630m).......10",
    "Warc (30,17,12m)......11"};

    ui->serviceCombo->addItems(services);
    int sel = kstChatSelection.toInt();
    ui->serviceCombo->setCurrentIndex(sel - 1);


    if (autoConnect)
        connectToHost();

}

KSTMainWindow::~KSTMainWindow()
{
    delete ui;
}
void KSTMainWindow::resizeEvent(QResizeEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::resizeEvent(event);
}
void KSTMainWindow::moveEvent(QMoveEvent * event)
{
    QSettings settings;
    settings.setValue("geometry/Main", saveGeometry());
    QWidget::moveEvent(event);
}
void KSTMainWindow::connectToHost()
{
    ui->includeLabel->setText("Including " + myCallsign);
    kstMeepFilterModel.setFilterString(myCallsign);
    tnclient->login(QString("%1\r\n").arg(myCallsign), QString(password) + "\r\n");
    tnclient->connectToHost(serverName , serverPort.toInt());

}


void KSTMainWindow::connectionEstab()
{
    trace("connection to ON4KST established");
    tnclient->login(QString("%1\r\n").arg(myCallsign), QString(password) + "\r\n");
    ui->connectButton->setText("Disconnect");
}

void KSTMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString("ON4KST Connection failed error %1").arg(error);
    trace(msg);
}



void KSTMainWindow::logIn()
{
    kstMessageModel.setCacheSize();

    QString msg = QString("Login Start - Send logon message\n");
    trace(msg);
    //tnclient->login(QString("%1\r\n").arg("G0GJV"), "62rosehill");

}

void KSTMainWindow::loggedOut()
{
    QString msg = QString("Logged Out of ON4KST");
    trace(QString(msg));
    userLoggedIn = false;
    setupComplete = false;
    bandChooseComplete = false;
    ui->connectButton->setText("Connect");
}

void KSTMainWindow::messageRx(QString msg)
{
    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
    }
    trace(QString("messageRx: %1").arg(traceMsg));
    if (setupComplete)
    {
        // break into lines...
        msgbuf.append(msg);

        int p = msgbuf.indexOf("\n");
        while (p >= 0)
        {
            QString m = msgbuf.left(p + 1);
            msgbuf = msgbuf.mid(p + 1);
            p = msgbuf.indexOf("\n");

            analyseTelnetMessage(m);
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
                tnclient->sendData(kstChatSelection + "\r\n");
                setupComplete = true;
            }
        }
    }
}
bool loadStringListFromFile (QStringList &list, const QString fname )
{
    QStringList stringsRead;
    QFile textFile( fname );
    if ( textFile.open( QIODevice::ReadOnly ) )
    {
        QTextStream textStream( &textFile );
        while ( true )
        {
            QString line = textStream.readLine();
            if ( line.isNull() )
                break;
            stringsRead.append( line.trimmed() );
        }
        list = stringsRead;
        return true;
    }
    return false;
}
void KSTMainWindow::on_analyseButton_clicked()
{
    filelines.clear();
    kstMessageModel.setCacheSize();

    QString InitialDir/* = GetCurrentDir()*/;

    QString Filter = "KST Chat Files (*.txt);Log Files (*.log);;"
                     "All Files (*.*)" ;

    QStringList KSTFileNames = QFileDialog::getOpenFileNames( this,
                       "Chat dumps from KST",
                       InitialDir,                   // opendir
                       Filter );

    if (KSTFileNames.size() == 0)
    {
        return;
    }
    TWaitCursor fred(this);

    if (KSTFileNames.size())
    {
        for (int i = 0; i < KSTFileNames.size(); i++)
        {
            QString fname = KSTFileNames[i].trimmed();

            if (loadStringListFromFile ( filelines, fname ))
            {
                // list is in reverse time order, so reverse it; this will make things easier later

                // but .log is already i the right order
                //std::reverse(filelines.begin(), filelines.end());

                ui->includeLabel->setText("Including " + myCallsign);
                kstMeepFilterModel.setFilterString(myCallsign);

                QTimer *timer = new QTimer(this);

                connect(timer, &QTimer::timeout, [=]()
                {
                    // NB a lambda function
                    if (curline < filelines.size())
                    {
                        if ( !filelines.at( curline ).isEmpty() && filelines.at( curline ) [ 0 ] != '#' )
                        {
                            QString atj = QString::fromLatin1(filelines.at( curline ).toLatin1());
                            int p = atj.indexOf("messageRx") ;
                            if (p >= 0)
                            {
                                atj = atj.mid( p + QString("messageRx: ").size());
                            }
                            analyseTelnetMessage(atj);
                        }
                        curline++;
                    }
                    else
                    {
                        timer->stop();
                        timer->deleteLater();
                        filelines.clear();
                    }
                }
                );

                timer->start(0);

             }
        }
    }
}
void KSTMainWindow::analyseTelnetMessage(QString atj)
{
//    18:58:18.640 messageRx: 1858Z ES4RM Sergei> (OH3DP) i am on 1558
//    18:58:44.037 messageRx: 1858Z OH3DP Hannu 2m, 70, 23> 1000


    atj = atj.trimmed();

    QStringList sl;
    sl = atj.split(">");
    if (sl.size() >= 2)
    {
        if (sl[0].endsWith(" chat"))
        {
            if (sl[1].isEmpty())
            {
                if (!bandChooseComplete)
                {
                    bandChooseComplete = true;

                    // 1st prompt - just logged in
                        QString t = sl[0].mid(6);
                        setWindowTitle("Minos KST Client : " + t);

                        tnclient->sendData(
                                           "/set ann\r\n"   // announce messages - not useful
                                           "/unset dx\r\n"    // dx messages - cluster - not useful
                                           "/sh us\r\n"     // users
                                           // "/sh msg 20\r\n" // last 20 messages - reversed, unfortunately
                                           );
                        // We seem to get the next prompt when we have all the info
                }
                return;
            }
            // it is a telnet prompt, so follows a command

            sl.removeAt(0); // get rid of the prompt
        }
    }

    if (sl.size() == 1)
    {
        // no chat message - it will be a command response or the initial blurb
        // e.g. "1337Z DX de ve1sky: 144174.0 W1VD tu FT8"
        // or G0GJV          IO91OK Mike io91

        if (sl[0].indexOf(" DX de ") >= 0 || (sl[0].indexOf(" ALL de ") >= 0))
        {
            // DX cluster message
            return;
        }

        QSharedPointer< KstUser > user(new KstUser());

        int part = 0;
        int pstart = 0;

        for (int i = 0; i < sl[0].length(); i++)
        {
            if (sl[0][i] == " ")
            {
                QString p = sl[0].mid(pstart, i - pstart);

                while (sl[0][i] == " " && i < sl[0].length())
                {
                    i++;
                }
                if (part == 0)
                {
                    user->call = p;
                    if (user->call.startsWith("(") && user->call.endsWith(")"))
                    {
                        user->away = true;
                        user->call = user->call.mid(1, user->call.size() - 2);
                    }
                    Callsign cs(user->call);
                    cs.validate();
                    if (cs.valRes == CS_OK)
                    {
                        pstart = i;
                        part = 1;
                    }
                    else
                        return;
                }
                else if (part == 1)
                {
                    user->loc = p;
                    user->name = sl[0].mid(pstart);
                    break;
                }
            }
        }

        if (!user->call.isEmpty())
        {
            if (!std::binary_search(callVector->begin(), callVector->end(), user, KstUserCompare))
            {
                int row = (std::lower_bound(callVector->begin(), callVector->end(), user, KstUserCompare ) - callVector->begin());
                callVector->insert(row, user);
                kstCallModel.insertRow(row);
            }
        }

        return;
    }


    QSharedPointer<KstMessageLine> kst(new KstMessageLine());

    kst->fullLine = atj;

    int part = 0;
    int pstart = 0;
    for (int i = 0; i < sl[0].length(); i++)
    {
        if (sl[0][i] == " ")
        {
            QString p = sl[0].mid(pstart, i - pstart);

            while (sl[0][i] == " " && i < sl[0].length())
            {
                i++;
            }
            pstart = i;
            if (part == 0)
            {
                kst->dtg = p;
                part++;
            }
            else if (part == 1)
            {
                kst->call = p;
                kst->name = sl[0].mid(pstart);
                break;
            }
        }
    }

    QString s3 = sl[1].trimmed();
    QString other;
    QString text = s3;

    if (s3[0] == '(')
    {
        while (s3[0] == '(')
        {
            s3 = s3.mid(1);
        }
        int closeBracket = s3.indexOf(')');
        other = s3.mid(0, closeBracket).trimmed();

        text = s3.mid(closeBracket + 1).trimmed();
    }
    else
    {
        QString temp = s3.section(' ', 0, 0);
        Callsign cs(temp);
        cs.validate();
        if (cs.valRes == CS_OK)
        {
            other = temp.toUpper().trimmed();
            text = s3.section(' ', 1).trimmed();
        }
    }
    kst->otherCall = other;
    kst->message = text;

    messageVector->push_back(kst);

    kstMessageModel.appendLastRow();

    kstMeepModel.appendLastRow();

    ui->messageTable->scrollToBottom();
    ui->meepTable->scrollToBottom();

    if (!kst->call.isEmpty())
    {
        QSharedPointer<KstUser> test(new KstUser());
        test->call = kst->call;
        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            callVector->insert(row, test);
            kstCallModel.insertRow(row);
        }
    }

    if (!kst->call.isEmpty())
    {
        QSharedPointer<KstUser> test(new KstUser());
        test->call = kst->otherCall;
        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            callVector->insert(row, test);
            kstCallModel.insertRow(row);
        }
    }
}
void KSTMainWindow::on_connectButton_clicked()
{
    if (userLoggedIn)
    {
        tnclient->sendData("/Q\r\n");
        tnclient->logout();
    }
    else
    {
        connectToHost();
    }
}
void KSTMainWindow::on_closeButton_clicked()
{
    close();
}


void KSTMainWindow::on_messageFilter_textChanged(const QString &arg1)
{
    kstMessageFilterModel.setFilterString(arg1);
    ui->messageTable->scrollToBottom();
}

void KSTMainWindow::on_CSFilter_textChanged(const QString &arg1)
{
    kstCallFilterModel.setFilterString(arg1);
}

void KSTMainWindow::on_kstSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->kstSplitter->saveState();
    settings.setValue("kstSplitterState" , state);
}
void KSTMainWindow::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;

    state = ui->CSTable->horizontalHeader()->saveState();
    settings.setValue("CSTable/state", state);

    state = ui->messageTable->horizontalHeader()->saveState();
    settings.setValue("messageTable/state", state);

    state = ui->meepTable->horizontalHeader()->saveState();
    settings.setValue("meepTable/state", state);
}

void KSTMainWindow::on_CSTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstCallFilterModel.mapToSource(index);
    QString call = callVector->at(sourceIndex.row())->call;
    ui->messageFilter->setText(call);
}

void KSTMainWindow::on_configureButton_clicked()
{
    KSTConfigure conf;

    conf.hostname = serverName;
    conf.port = serverPort;
    conf.username = myCallsign;
    conf.password = password;
    conf.autoConnect = autoConnect;

    if (conf.exec() == QDialog::Accepted)
    {
        serverName = conf.hostname;
        serverPort = conf.port;
        myCallsign = conf.username;
        password = conf.password;
        autoConnect = conf.autoConnect;

        QSettings settings;

        settings.setValue("hostname", serverName);
        settings.setValue("port", serverPort);
        settings.setValue("username", myCallsign);
        settings.setValue("password", password);
        settings.setValue("autoConnect", autoConnect);

        reconnect();
    }
}
void KSTMainWindow::reconnect()
{
    tnclient->sendData("/Q\r\n");
    tnclient->logout();

    QTimer *timer = new QTimer(this);
    timer->setInterval(2000);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, [=]()
    {
        // NB a lambda function
        connectToHost();
        timer->deleteLater();
    }
    );

    timer->start(100);
}
void KSTMainWindow::on_genmsgButton_clicked()
{
    QString msg = ui->msgEdit->text();
    if (!msg.isEmpty())
    {
        tnclient->sendData(msg + "\r\n");
    }
}

void KSTMainWindow::on_meepButton_clicked()
{
    QString msg = ui->msgEdit->text();
    QString call = ui->callEdit->text();
    if (!msg.isEmpty() && !call.isEmpty())
    {
        tnclient->sendData("/CQ " + call + " " + msg + "\r\n");
    }
}

void KSTMainWindow::on_msgSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->msgSplitter->saveState();
    settings.setValue("msgSplitterState" , state);
}

void KSTMainWindow::on_messageTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstMessageFilterModel.mapToSource(index);
    QSharedPointer<KstMessageLine> line = messageVector->at(sourceIndex.row());
    QString call = line->call;
    if (call.compare(myCallsign, Qt::CaseInsensitive) == 0)
    {
        call = line->otherCall;
    }
    ui->callEdit->setText(call);
}

void KSTMainWindow::on_meepTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = kstMeepFilterModel.mapToSource(index);
    QSharedPointer<KstMessageLine> line = messageVector->at(sourceIndex.row());
    QString call = line->call;
    if (call.compare(myCallsign, Qt::CaseInsensitive) == 0)
    {
        call = line->otherCall;
    }
    ui->callEdit->setText(call);
}

void KSTMainWindow::on_serviceCombo_currentIndexChanged(int index)
{
    if (userLoggedIn)
    {
        kstChatSelection = QString::number(index + 1);

        QSettings settings;
        settings.setValue("service", kstChatSelection);

        reconnect();
    }
}

void KSTMainWindow::on_clearButton_clicked()
{
    kstMessageModel.reset();
    kstMeepModel.reset();
    kstCallModel.reset();
}
