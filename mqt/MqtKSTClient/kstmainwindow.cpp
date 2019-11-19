#include <QHostInfo>
#include <QSettings>

#include "cutils.h"

#include "kstconfigure.h"

#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"

CallGridModel::CallGridModel()
{

}
void CallGridModel::setCallVector(QSharedPointer<QStringList > pcallVector)
{
    beginResetModel();
    callVector = pcallVector;
    endResetModel();
}
QModelIndex CallGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!callVector)
        return QModelIndex();

    if ( row < 0 || row >= callVector->count() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}
QModelIndex CallGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int CallGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    if (!callVector)
        return 0;
    return callVector->count();
}

void CallGridModel::appendRow(QString call)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
    callVector->push_back(call);
    endInsertRows();
}

QVariant CallGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();

    if (role == Qt::DisplayRole)
    {
        QString crec = callVector->at(row);

        return crec;
    }
    return QVariant();
}
QVariant CallGridModel::headerData( int /*section*/, Qt::Orientation orientation,
                     int role ) const
{

    QString cell;
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        return "Callsign";
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            // BUT the headers aren't drawn using the delegate, so this
            // all fails to work

            // Do we lose the vertical header?
            QString s = "Memxx";
            QSize r = delegate->docSize(s);
            return r;
        }
    }
    return QVariant();
}
int CallGridModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return 1;
}
//==========================================================================================
bool CallGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    CallGridModel *cgm = dynamic_cast<CallGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QString call = cgm->callVector->at(sourceRow);
    if (call.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void CallGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}
//==========================================================================================

ChatGridModel::ChatGridModel()
{

}
void ChatGridModel::setChatVector(QSharedPointer<QVector <QSharedPointer<ChatLine> > > pchatVector)
{
    beginResetModel();
    chatVector = pchatVector;
    endResetModel();
}
QModelIndex ChatGridModel::index( int row, int column,
                              const QModelIndex &parent ) const
{
    if (!chatVector)
        return QModelIndex();

    if ( row < 0 || row >= chatVector->count() || ( parent.isValid() && parent.column() != 0 ) )
        return QModelIndex();

    return createIndex( row, column, nullptr );
}
QModelIndex ChatGridModel::parent( const QModelIndex &/*index*/ ) const
{
    return QModelIndex();
}

int ChatGridModel::rowCount( const QModelIndex &/*parent*/ ) const
{
    if (!chatVector)
        return 0;
    return chatVector->count();
}

void ChatGridModel::appendRow(QSharedPointer<ChatLine> kstmsg)
{
    beginInsertRows(QModelIndex(), rowCount() , rowCount());
    chatVector->push_back(kstmsg);
    endInsertRows();
}

enum ChatColumns {eccSrc = 0, eccDTG, eccCall, eccName, eccOther, eccText, eccMaxColumn};
QVariant ChatGridModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();
    int column = index.column();

    if (role == Qt::DisplayRole)
    {
        //QSharedPointer<QVector <QSharedPointer<ChatLine> > > chatVector;
        QSharedPointer<ChatLine> crec = chatVector->at(row);

        QString cell;
        switch (column)
        {
        case eccSrc:
            cell = crec->source;
            break;
        case eccDTG:
            cell = crec->dtg;
            break;
        case eccCall:
            cell = crec->call;
            break;
        case eccName:
            cell = crec->name;
            break;
        case eccOther:
            cell = crec->otherCall;
            break;
        case eccText:
            cell = crec->message;
            break;
        }
        return cell;
    }
    else if (role == Qt::ToolTipRole)
    {
        QSharedPointer<ChatLine> crec = chatVector->at(row);
        return crec->fullLine;
    }
    return QVariant();
}
QVariant ChatGridModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{

    QString cell;
    if ( orientation == Qt::Horizontal && role == Qt::DisplayRole )
    {
        switch (section)
        {
        case eccSrc:
            cell = "Source";
            break;
        case eccDTG:
            cell = "DTG";
            break;
        case eccCall:
            cell = "Call";
            break;
        case eccName:
            cell = "Name";
            break;
        case eccOther:
            cell = "Other Call";
            break;
        case eccText:
            cell = "Text";
            break;
        }
        return cell;
    }
    else if (orientation == Qt::Vertical && role == Qt::SizeHintRole)
    {
        if (delegate)
        {
            // BUT the headers aren't drawn using the delegate, so this
            // all fails to work

            // Do we lose the vertical header?
            QString s = "Memxx";
            QSize r = delegate->docSize(s);
            return r;
        }
    }
    return QVariant();
}
int ChatGridModel::columnCount( const QModelIndex & /*parent*/ ) const
{
    return eccMaxColumn;
}
//==========================================================================================
bool ChatGridSortFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &/*sourceParent*/) const
{
    if (filterString.isEmpty())
        return true;

    ChatGridModel *cgm = dynamic_cast<ChatGridModel *>(sourceModel());
    if (!cgm || sourceRow >= cgm->rowCount())
        return false;

    QSharedPointer<ChatLine> kstmsg = cgm->chatVector->at(sourceRow);
    if (kstmsg->fullLine.indexOf(filterString, 0, Qt::CaseInsensitive) >= 0)
        return true;

    return false;
}

void ChatGridSortFilterModel::setFilterString(QString f)
{
    filterString = f;
    invalidateFilter();
}
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

    QByteArray state = settings.value("kstSplitterState").toByteArray();
    ui->kstSplitter->restoreState(state);

    chatVector = QSharedPointer<QVector <QSharedPointer<ChatLine> > >( new QVector<QSharedPointer<ChatLine> >);
    cgm.setChatVector(chatVector);

    cgsfm.setSourceModel(&cgm);
    ui->messageTable->setModel(&cgsfm);
    ui->messageTable->horizontalHeader()->setStretchLastSection(true);

    callVector = QSharedPointer<QStringList >( new QStringList() );
    clgm.setCallVector(callVector);

    clgsfm.setSourceModel(&clgm);
    ui->CSTable->setModel(&clgsfm);

    clgsfm.sort(0);

    ui->CSTable->horizontalHeader()->setStretchLastSection(true);

    messageDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;
    CSDelegate = QSharedPointer<HtmlDelegate>( new HtmlDelegate(1.0, 1.0)) ;

    cgm.delegate = messageDelegate;
    clgm.delegate = CSDelegate;

    ui->messageTable->setItemDelegate(messageDelegate.data());
    ui->CSTable->setItemDelegate(CSDelegate.data());

    QHeaderView *verticalHeader = ui->messageTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);

    verticalHeader = ui->CSTable->verticalHeader();
    verticalHeader->setVisible(false);
    verticalHeader->setDefaultSectionSize(10);
    verticalHeader->setMinimumSectionSize(10);

    verticalHeader->setSectionResizeMode(QHeaderView::ResizeToContents);



    state = settings.value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);

    state = settings.value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->messageTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);

    tnclient = new QtTelnet(this);

    connect(tnclient, SIGNAL(socketConnected()), this, SLOT(connectionEstab()));
    //connect(tnclient, SIGNAL(connected(bool)), this, SLOT(connected(bool)));
    connect(tnclient, SIGNAL(loginRequired()), this, SLOT(logIn()));
    connect(tnclient, SIGNAL(connectionError(QAbstractSocket::SocketError)), this, SLOT(connectionError(QAbstractSocket::SocketError)));
    connect(tnclient, SIGNAL(loggedOut()), this, SLOT(loggedOut()));
    connect(tnclient, SIGNAL(message(QString)), this, SLOT(messageRx(QString)));

    hostname = settings.value("hostname", "www.on4kst.info").toString();
    port = settings.value("port", "23000").toString();
    username = settings.value("username", "").toString();
    password = settings.value("password", "").toString();
    service = settings.value("service", "1").toString();

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
    tnclient->login(QString("%1\r\n").arg(username), QString(password) + "\r\n");
    tnclient->connectToHost("www.on4kst.info" , 23000);

}


void KSTMainWindow::connectionEstab()
{
    trace("connection to ON4KST established");
    tnclient->login(QString("%1\r\n").arg(username), QString(password) + "\r\n");
}

void KSTMainWindow::connectionError(QAbstractSocket::SocketError error)
{
    QString msg = QString("ON4KST Connection failed error %1").arg(error);
    trace(msg);
}



void KSTMainWindow::logIn()
{
    QString msg = QString("Login Start - Send logon message\n");
    trace(msg);
    //tnclient->login(QString("%1\r\n").arg("G0GJV"), "62rosehill");

}

void KSTMainWindow::loggedOut()
{
    QString msg = QString("Logged Out of ON4KST");
    trace(QString(msg));
}

void KSTMainWindow::messageRx(QString msg)
{
    trace(QString("messageRx: %1").arg(msg));
    if (setupComplete)
    {
        analyseTelnetMessage(msg);
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
                tnclient->sendData(service + "\r\n");
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
class CsvReader
{
public:
    CsvReader(QChar sep = ',');

    void parseCsvLine(const QString &line, QStringList &csv);
    bool parseCsv(const QString &fileName, QList<QStringList> &csv);

private:
    QChar sep;
    QStringList itemList;

    void checkString(QString &temp, QChar character, QStringList &csv);
    void checkString(QString &temp, QChar character, QList<QStringList> &csv);
};
CsvReader::CsvReader(QChar sep):sep(sep){}

bool CsvReader::parseCsv(const QString &fileName, QList<QStringList> &csv)
{
    QFile file (fileName);
    if (file.open(QIODevice::ReadOnly))
    {
        QString data = file.readAll();
        data.remove( QRegExp("\r") ); //remove all ocurrences of CR (Carriage Return)
        QString temp;
        QChar character;
        QTextStream textStream(&data);
        while (!textStream.atEnd())
        {
            textStream >> character;
            if (character == ',')
            {
                checkString(temp, character, csv);
            }
            else if (character == '\n')
            {
                checkString(temp, character, csv);
            }
            else if (textStream.atEnd())
            {
                temp.append(character);
                checkString(temp, 0, csv);
            }
            else
            {
                temp.append(character);
            }
        }
        itemList.clear();
        return true;
    }
    return false;
}
void CsvReader::parseCsvLine(const QString &line, QStringList &csv)
{
     QString temp;
     QChar character;

     int lsize = line.length();
     for (int i = 0; i < lsize; i++)
     {
         character = line[i];
         if (character == sep)
         {
             checkString(temp, character, csv);
         }
         else if (character == '\n')
         {
             checkString(temp, character, csv);
         }
         else if (character == nullptr)
         {
             checkString(temp, 0, csv);
         }
         else
         {
             temp.append(character);
         }
     }
     checkString(temp, 0, csv);
}
void CsvReader::checkString(QString &temp, QChar character, QStringList &csv)
{
    if(temp.count("\"")%2 == 0)
    {
        if (temp.startsWith( QChar('\"')) && temp.endsWith( QChar('\"') ) )
        {
             temp.remove( QRegExp("^\"") );
             temp.remove( QRegExp("\"$") );
        }
        //FIXME: will possibly fail if there are 4 or more reapeating double quotes
        temp.replace("\"\"", "\"");
        csv.append(temp.trimmed());
        if (character != QChar(sep))
        {
            return;
        }
        temp.clear();
    }
    else
    {
        temp.append(character);
    }
}
void CsvReader::checkString(QString &temp, QChar character, QList<QStringList> &csv)
{
    if(temp.count("\"")%2 == 0)
    {
        if (temp.startsWith( QChar('\"')) && temp.endsWith( QChar('\"') ) )
        {
            temp.remove( QRegExp("^\"") );
            temp.remove( QRegExp("\"$") );
        }
        temp.replace("\"\"", "\"");
        itemList.append(temp.trimmed());
        if (character != QChar(','))
        {
            csv.append(itemList);
            itemList.clear();
        }
        temp.clear();
    }
    else
    {
        temp.append(character);
    }
}

void CSVToStringList( const QString &qs, QStringList &sl )
{
    sl.clear();

    CsvReader csv;

    csv.parseCsvLine(qs, sl);
}
void TSVToStringList( const QString &qs, QStringList &sl )
{
    sl.clear();

    CsvReader csv('\t');

    csv.parseCsvLine(qs, sl);
}

void KSTMainWindow::analyseFileMessage(QString atj)
{
    QStringList slc;
    TSVToStringList( atj, slc );

    if (slc.size() == 3)
    {
        // get DTG; file format is 2018-01-16 22:30:56Z

        QString dts = slc[0].left(19);

        QDateTime qdt = QDateTime::fromString(dts, "yyyy-MM-dd hh:mm:ss");

        if (!qdt.isValid())
            return;

        QSharedPointer<ChatLine> kst(new ChatLine());

        kst->fullLine = atj;
        kst->dtg = qdt.toString("HHmmZ");

        QString s2 = slc[1];
        s2.replace(QChar::Nbsp, QChar::Space);
        QString call = s2.section(' ', 0, 0).trimmed();
        QString name = s2.section(' ', 1).trimmed();

        kst->call = call;
        kst->name = name;

        QString s3 = slc[2];
        QString other;
        QString text = s3;
        if (s3[0] == '(')
        {
            while (s3[0] == '(')
            {
                s3 = s3.mid(1);
            }
            int closeBracket = s3.indexOf(')');
            other = s3.mid(0, closeBracket - 1).trimmed();

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

        cgm.appendRow(kst);

        if (!callVector->contains(call))
        {
            clgm.appendRow(call.toUpper());
            clgsfm.sort(0);
        }

        if (!callVector->contains(other))
        {
            clgm.appendRow(other.toUpper());
            clgsfm.sort(0);
        }
    }
}
void KSTMainWindow::on_analyseButton_clicked()
{
    QString InitialDir = GetCurrentDir();

    QString Filter = "KST Chat Files (*.txt);;"
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

            QStringList sl;
            if (loadStringListFromFile ( sl, fname ))
            {
                // list is in reverse time order, so reverse it; this will make things easier later

                std::reverse(sl.begin(), sl.end());
                for ( int j = 0; j < sl.size(); j++ )
                {
                    if ( !sl.at( j ).isEmpty() && sl.at( j ) [ 0 ] != '#' )
                    {
                        QString atj = QString::fromLatin1(sl.at( j ).toLatin1());
                        analyseFileMessage(atj);
                    }
                }
            }
        }
        //cgm.setChatVector(chatVector);
    }
}
void KSTMainWindow::analyseTelnetMessage(QString atj)
{
//    18:58:18.640 messageRx: 1858Z ES4RM Sergei> (OH3DP) i am on 1558
//    18:58:44.037 messageRx: 1858Z OH3DP Hannu 2m, 70, 23> 1000


    atj = atj.trimmed();

    QStringList sl;
    sl = atj.split(">");
    if (sl.size() < 2)
        return;
    if (sl[1].isEmpty())
        return;

    QSharedPointer<ChatLine> kst(new ChatLine());

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

    cgm.appendRow(kst);
    ui->messageTable->scrollToBottom();

    if (!callVector->contains(kst->call))
    {
        clgm.appendRow(kst->call.toUpper());
        clgsfm.sort(0);
    }

    if (!callVector->contains(kst->otherCall))
    {
        clgm.appendRow(kst->otherCall.toUpper());
        clgsfm.sort(0);
    }

}
void KSTMainWindow::on_connectButton_clicked()
{
    connectToHost();
}
void KSTMainWindow::on_closeButton_clicked()
{
    close();
}


void KSTMainWindow::on_messageFilter_textChanged(const QString &arg1)
{
    cgsfm.setFilterString(arg1);
}

void KSTMainWindow::on_CSFilter_textChanged(const QString &arg1)
{
    clgsfm.setFilterString(arg1);
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
}

void KSTMainWindow::on_CSTable_clicked(const QModelIndex &index)
{
    QModelIndex sourceIndex = clgsfm.mapToSource(index);
    QString call = callVector->at(sourceIndex.row());
    ui->messageFilter->setText(call);
}

void KSTMainWindow::on_configureButton_clicked()
{
    KSTConfigure conf;

    conf.hostname = hostname;
    conf.port = port;
    conf.username = username;
    conf.password = password;
    conf.service = service;

    if (conf.exec() == QDialog::Accepted)
    {
        hostname = conf.hostname;
        port = conf.port;
        username = conf.username;
        password = conf.password;
        service = conf.service;

        QSettings settings;

        settings.setValue("hostname", hostname);
        settings.setValue("port", port);
        settings.setValue("username", username);
        settings.setValue("password", password);
        settings.setValue("service", service);
    }
}
