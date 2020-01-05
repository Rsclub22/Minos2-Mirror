#include <QHostInfo>
#include <QSettings>

#include "cutils.h"

#include "kstconfigure.h"

#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"

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

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    createCloseEvent();
    connect(&CloseTimer, SIGNAL(timeout()), this, SLOT(CloseTimerTimer()));
    CloseTimer.start(100);

    messageVector = QSharedPointer<QVector <QSharedPointer<KstMessageLine> > >( new QVector<QSharedPointer<KstMessageLine> >);

    kstMessageModel.setChatVector(messageVector);

    kstMessageFilterModel.setSourceModel(&kstMessageModel);

    ui->messageTable->setModel(&kstMessageFilterModel);
    ui->messageTable->horizontalHeader()->setStretchLastSection(true);

    kstMeepFilterModel.setSourceModel(&kstMessageModel);
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

    // these are used for sizing when adjust to content
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
    verticalHeader->setDefaultSectionSize(ms.height() *4/5);
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

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sortIndicatorChanged(int, Qt::SortOrder)),
             this, SLOT( on_sortIndicatorChanged(int, Qt::SortOrder)));


    kstclient = new QTcpSocket(this);

    connect(kstclient, SIGNAL(connected()), this, SLOT(connected()));
    connect(kstclient, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(kstclient, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(kstclient, SIGNAL(readyRead()), this, SLOT(onReadyRead()));

    serverName = settings.value("hostname", "www.on4kst.info").toString();
    serverPort = settings.value("port", "23001").toString();
    myCallsign = settings.value("username", "").toString();
    password = settings.value("password", "").toString();
    kstChatSelection = settings.value("service", "1").toString();
    autoConnect = settings.value("autoConnect", false).toBool();
    myLoc = settings.value("locator", "").toString();

    QStringList services =
    {"50/70 MHz",
    "144/432 MHz",
    "Microwave",
    "EME/JT65",
    "Low Band",
    "50 MHz IARU Region 3",
    "50 MHz IARU Region 2",
    "144/432 MHz IARU R 2",
    "144/432 MHz IARU R 3",
    "kHz (2000-630m)",
    "Warc (30,17,12m)"};

    ui->serviceCombo->addItems(services);
    int sel = kstChatSelection.toInt();
    ui->serviceCombo->setCurrentIndex(sel - 1);

    ui->CSFilter->installEventFilter(this);
    ui->messageFilter->installEventFilter(this);
    ui->callEdit->installEventFilter(this);
    ui->msgEdit->installEventFilter(this);
    started = true;

    if (autoConnect)
        connectToHost();

    ui->genmsgButton->setDefault(true);

    ui->analyseButton->setVisible(false);
    ui->messageFilter->setFocus();

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
void KSTMainWindow::changeEvent( QEvent* e )
{
    if( e->type() == QEvent::WindowStateChange )
    {
        QSettings settings;
        settings.setValue("geometry/Main", saveGeometry());
    }
}

void KSTMainWindow::CloseTimerTimer(  )
{
   static bool closed = false;
   if ( !closed )
   {
      if ( checkCloseEvent() )
      {
          trace("closing down due to close signalled");
         closed = true;
         close();
      }
   }
}

void KSTMainWindow::connectToHost()
{
    kstCallModel.locator = myLoc;
    kstclient->connectToHost(serverName, serverPort.toUShort());
}


void KSTMainWindow::connected()
{
    trace("connection to ON4KST established");
    ui->includeLabel->setText("Including " + myCallsign);
    kstMeepFilterModel.setFilterString(myCallsign);
    kstMessageModel.setCacheSize();
    ui->connectButton->setText("Disconnect");
}


void KSTMainWindow::disconnected()
{
    trace("Disconnected from ON4KST");
    ui->includeLabel->clear();
    kstMeepFilterModel.setFilterString("myCallsign""");
    ui->connectButton->setText("Connect");
    kstconnected = false;
}

void KSTMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString("ON4KST Connection failed error %1").arg(error);
    trace(msg);
}

void KSTMainWindow::onReadyRead()
{
    QByteArray b = kstclient->readAll();
    QString msg = QString(b);

    QString traceMsg = msg.remove("\r");
    if (traceMsg.endsWith("\n"))
    {
        traceMsg.chop(1);
    }
    trace(QString("messageRx: %1").arg(traceMsg));

    // break into lines...
    msgbuf.append(msg);

    int p = msgbuf.indexOf("\n");
    while (p >= 0)
    {
        QString m = msgbuf.left(p + 1);
        msgbuf = msgbuf.mid(p + 1);
        p = msgbuf.indexOf("\n");

        analyseKstMessage(m);
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
                        if ( !filelines.at( curline ).isEmpty() )
                        {
                            QString atj = QString::fromLatin1(filelines.at( curline ).toLatin1());
                            int p = atj.indexOf("messageRx") ;
                            if (p >= 0)
                            {
                                atj = atj.mid( p + QString("messageRx: ").size());
                            }
                            analyseKstMessage(atj);
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
void KSTMainWindow::sendKST(QString msg)
{
        kstclient->write((msg + "\r\n").toLocal8Bit());
        trace("Send to KST: " + msg);
}
void KSTMainWindow::analyseKstMessage(QString atj)
{
//    18:58:18.640 messageRx: 1858Z ES4RM Sergei> (OH3DP) i am on 1558
//    18:58:44.037 messageRx: 1858Z OH3DP Hannu 2m, 70, 23> 1000


    atj = atj.trimmed();

    if (!kstconnected && atj.contains(" login "))
    {
//        11:28:01.769 Client read : LOGINC|G0GJV|62rosehill|2|KST2Me 1.2.0.0
//        11:28:01.769 Client read : |20|20|1|0|0|

        QString loginMessage = "LOGINC|"
                + myCallsign
                + "|" + password
                + "|" + kstChatSelection
                + "|" + "Minos 0.0.0.999"   // client software version
                + "|20" // past messages
                + "|20"  // past DX/map messages
                + "|1"  // users list/update flags - If the users list/update flags = 0, no Uxx frames will be sent (even after the login)
                + "|0"   // last Unix timestamp for messages
                + "|0"   // last Unix timestamp for dx/map
                + "|";

        sendKST(loginMessage);

        //Optional allowed frames between LOGINC and SDONE are SDXQ, SMAQ, RDXQ and RMAQ.
        // we don't need them - yet


        kstconnected = true;
        return;
    }

    QStringList sl;
    sl = atj.split("|");

    if (sl[0] == "LOGSTAT")
    {
        // user config
//        LOGSTATS if LOGINC:
//        LOGSTAT|100|2|20040703a|239E038F12E685FB75C6C03A79A1DE8A|11|Alain/telnet|Stiévenart|JO20HI|on4kst@skynet.be|
//        LOGSTAT|100|chat id|client software version|session key|config|first name|last name|locator|email|
        QString sdone = "SDONE|" + kstChatSelection +"|";
        sendKST(sdone);
    }
    else if (sl[0] == "CR")
    {
        // message frame at login
        // CR|chat id|Unix time|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->fullLine = atj;

        QString unixTime = sl[2];
        QDateTime dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);
        kst->dtg = dtg.toString("HH:mm");

        kst->call = sl[3];
        kst->name = sl[4];

        kst->message = sl[6];
        kst->otherCall = sl[7];
        if (kst->otherCall == "0")
            kst->otherCall.clear();

        messageVector->push_front(kst);

    }
    else if (sl[0] == "CE")
    {
        // end of CR frames
        kstMessageModel.setChatVector(messageVector);

        ui->messageTable->scrollToBottom();
        ui->meepTable->scrollToBottom();
    }
    else if (sl[0] == "CH")
    {
        // message frame after login
        // CH|chat id|date|callsign|firstname|destination|msg|highlight|

        QSharedPointer<KstMessageLine> kst(new KstMessageLine());

        kst->fullLine = atj;

        QString unixTime = sl[2];
        QDateTime dtg = QDateTime::fromMSecsSinceEpoch(unixTime.toLongLong() * 1000);
        kst->dtg = dtg.toString("HH:mm");

        kst->call = sl[3];
        kst->name = sl[4];
        // 5 is destination
        kst->message = sl[6];
        kst->otherCall = sl[7];
        if (kst->otherCall == "0")
            kst->otherCall.clear();

        kstMessageModel.appendLastRow(kst);

        ui->messageTable->scrollToBottom();
        ui->meepTable->scrollToBottom();

    }
    else if (sl[0] == "DL")
    {
        // DX frames for the DX window
    }
    else if (sl[0] == "DE")
    {
        // end of DX frames
    }
    else if (sl[0] == "ML")
    {
        // DX frames for the map window
    }
    else if (sl[0] == "ME")
    {
        // end of ML frames
    }
    else if (sl[0] == "DM")
    {
        // DX frames for the DX and the MAP windows
    }
    else if (sl[0] == "DF")
    {
        // end of DM frames
    }
    else if (sl[0] == "LOC")
    {
        //Locator update
        // LOC|Unix time|callsign|locator|
    }

    else if (sl[0] == "UA0")
    {
        //    User frame at login
//    UA0|chat id|callsign|firstname|locator|state|
//    UA0|2|DK5EW|Erwin MMMonVHF|JN47NX|0|
//    UA0|2|DK5OX|Boris 6/2m|JN59MO|2|
//    UA0|2|DL3JIN|Peter|JO60LX|1|
//    User state:
//        bit 0: the user is away -displayed "(callsign)" -
//        bit 1: logged within the last 5 minutes
//        bit 2: sysop
//        bit 3: it would be not used (user with privileges)

        QSharedPointer<KstUser> test(new KstUser());
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            callVector->insert(row, test);
        }
    }

    else if (sl[0] == "UE")
    {
//    Users statistics/end of users frames
//    UE|chat id|nb registered users|
//    UE|2|4777|

        kstCallModel.setCallVector(callVector);

    }

    else if (sl[0] == "US4")
    {
//    User state (here/not here/more than 5 min logged)
//    US4|chat id|callsign|state|
//    US4|2|OH2JXA|state|

        QSharedPointer<KstUser> test(new KstUser());
        test->call = sl[2];
        QString state = sl[3];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call)
        {
            // as it should be...
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            int row = l - callVector->begin();
            emit kstCallModel.dataChanged(kstCallModel.index(row, ecscCall), kstCallModel.index(row, ecscDistance));
        }
    }

    else if (sl[0] == "UM3")
    {
//    User already logged
//    UM3|chat id|callsign|firstname|locator|state|
//    UM3|2|OZ2M|Bo|JO65FR|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call)
        {
            // as it should be...
            l->data()->name = test->name;
            l->data()->loc = test->loc;
            l->data()->away = test->away;
            l->data()->recent = test->recent;
            int row = l - callVector->begin();
            emit kstCallModel.dataChanged(kstCallModel.index(row, ecscCall), kstCallModel.index(row, ecscDistance));

        }

    }

    else if (sl[0] == "UR6")
    {
//    User disconnected (to remove)
//    UR6|chat id|callsign|
//    UR6|2|RA3MR/3|
        QSharedPointer<KstUser> test(new KstUser());
        test->call = sl[2];

        QVector<QSharedPointer<KstUser> >::iterator l = std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare);
        if (l != callVector->end() && l->data()->call == test->call)
        {
            // as it should be...

            // if we remove the last row then the call model
            // is one short as we have already removed it from the vector
            int row = l - callVector->begin();

            kstCallModel.removeRow(row);

        }
    }

    else if (sl[0] == "UA5")
    {
//    UA5 user connected (to add)
//    UA5|chat id|callsign|firstname|locator|state|
//    UA5|2|PA0GUS|GUUS|JO23TA|2|

        QSharedPointer<KstUser> test(new KstUser());
        test->call = sl[2];
        test->name = sl[3];
        test->loc = sl[4];
        QString state = sl[5];
        int istate = state.toInt();
        if (istate & 1)
            test->away = true;
        if (istate & 2)
            test->recent = true;

        if (!std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
        {
            int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());
            kstCallModel.insertRow(row, test);
        }

    }
    else if (sl[0] == "CK")
    {
        // link check
        sendKST("\r\n");
    }

    QSharedPointer<KstUser> test(new KstUser());
    test->call = myCallsign.toUpper();
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        if (user->away)
        {
            ui->awayButton->setText("Set Back");
        }
        else
        {
            ui->awayButton->setText("Set Away");
        }
    }


}
void KSTMainWindow::on_connectButton_clicked()
{
    if (kstconnected)
    {
        // MSG|chat id|destination|command|0|
        QString quitMsg = "MSG|" + kstChatSelection + "|0|/QUIT|0|";
        sendKST(quitMsg);
        kstclient->waitForBytesWritten(1000);
        kstclient->disconnectFromHost();
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
    QSharedPointer<KstUser> user = callVector->at(sourceIndex.row());
    QString call = user->call;

    setNameFromCall(call);

    ui->messageFilter->setText(call);
    ui->callEdit->setText(call);
    ui->msgEdit->setFocus();
}

void KSTMainWindow::on_configureButton_clicked()
{
    KSTConfigure conf;

    conf.hostname = serverName;
    conf.port = serverPort;
    conf.username = myCallsign;
    conf.password = password;
    conf.autoConnect = autoConnect;
    conf.locator = myLoc;

    if (conf.exec() == QDialog::Accepted)
    {
        serverName = conf.hostname;
        serverPort = conf.port;
        myCallsign = conf.username;
        password = conf.password;
        autoConnect = conf.autoConnect;
        myLoc = conf.locator;

        QSettings settings;

        settings.setValue("hostname", serverName);
        settings.setValue("port", serverPort);
        settings.setValue("username", myCallsign);
        settings.setValue("password", password);
        settings.setValue("autoConnect", autoConnect);
        settings.setValue("locator", myLoc);

        if  (kstconnected)
        {
            reconnect();
        }
    }
}
void KSTMainWindow::reconnect()
{
    if (kstconnected)
    {
        // MSG|chat id|destination|command|0|
        QString quitMsg = "MSG|" + kstChatSelection + "|0|/QUIT|0|";
        sendKST(quitMsg);

        kstclient->waitForBytesWritten(1000);
        kstclient->disconnectFromHost();
    }

    QTimer *timer = new QTimer(this);
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
        QString msg2 = "MSG|" + kstChatSelection + "|0|" + msg + "|0|";
        sendKST(msg2);
    }
    ui->msgEdit->clear();
}

void KSTMainWindow::on_meepButton_clicked()
{
    QString msg = ui->msgEdit->text();
    QString call = ui->callEdit->text();
    if (!msg.isEmpty() && !call.isEmpty())
    {
        QString msg = ui->msgEdit->text();
        if (!msg.isEmpty())
        {
            QString msg2 = "MSG|" + kstChatSelection + "|0|/CQ " + call + " " + msg + "|0|";
            sendKST(msg2);
        }
        ui->msgEdit->clear();
    }
}

void KSTMainWindow::on_msgSplitter_splitterMoved(int /*pos*/, int /*index*/)
{
    QSettings settings;
    QByteArray state = ui->msgSplitter->saveState();
    settings.setValue("msgSplitterState" , state);
}

void KSTMainWindow::setNameFromCall(QString call)
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = call;
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        QStringList name = user->name.split(' ');

        ui->msgEdit->setText("Hi " + name[0] + " ");
    }
    else
    {
        ui->msgEdit->clear();
    }
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

    setNameFromCall(call);

    ui->callEdit->setText(call);
    ui->msgEdit->setFocus();
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
    setNameFromCall(call);
    ui->callEdit->setText(call);
    ui->msgEdit->setFocus();

}

void KSTMainWindow::on_serviceCombo_currentIndexChanged(int index)
{
    if (started)
    {
        kstChatSelection = QString::number(index + 1);

        QSettings settings;
        settings.setValue("service", kstChatSelection);

        if (kstconnected)
            reconnect();
    }
}

void KSTMainWindow::on_clearButton_clicked()
{
    kstMessageModel.reset();
    kstCallModel.reset();
}
bool KSTMainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress )
    {
        QKeyEvent *ke = dynamic_cast<QKeyEvent *>(event);
        if (ke->key() == Qt::Key_Escape)
        {
            if (obj == ui->messageFilter)
            {
                ui->messageFilter->clear();
            }
            else if (obj == ui->CSFilter)
            {
                ui->CSFilter->clear();
            }
            else if (obj == ui->callEdit)
            {
                ui->callEdit->clear();
            }
            else if (obj == ui->msgEdit)
            {
                ui->msgEdit->clear();
            }
        }
    }

   return false;
}
void KSTMainWindow::on_sortIndicatorChanged(int /*logicalIndex*/, Qt::SortOrder /*order*/)
{
    on_sectionResized(0, 0, 0);
}

void KSTMainWindow::on_callEdit_textChanged(const QString &/*arg1*/)
{
    if (ui->callEdit->text().isEmpty())
    {
        ui->meepButton->setDefault(false);
        ui->genmsgButton->setDefault(true);
    }
    else
    {
        ui->genmsgButton->setDefault(false);
        ui->meepButton->setDefault(true);
    }
}

void KSTMainWindow::on_clearMessageButton_clicked()
{
    ui->callEdit->clear();
    ui->msgEdit->clear();
    ui->messageFilter->setFocus();
}

void KSTMainWindow::on_awayButton_clicked()
{
    QSharedPointer<KstUser> test(new KstUser());
    test->call = myCallsign.toUpper();
    if (std::binary_search(callVector->begin(), callVector->end(), test, KstUserCompare))
    {
        int row = (std::lower_bound(callVector->begin(), callVector->end(), test, KstUserCompare ) - callVector->begin());

        QSharedPointer<KstUser> user = callVector->at(row);

        if (user->away)
        {
            QString msg = "MSG|" + kstChatSelection + "|0|/BACK|0|";
            sendKST(msg);

        }
        else
        {
            QString msg = "MSG|" + kstChatSelection + "|0|/AWAY|0|";
            sendKST(msg);
        }
    }
}
