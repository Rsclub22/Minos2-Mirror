#include "kstmainwindow.h"
#include "ui_kstmainwindow.h"
#include "cutils.h"

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
            cell = crec->dtg.toString("HH:mm:ss");
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
    if (role == Qt::ToolTipRole)
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
    if ( orientation == Qt::Vertical && role == Qt::DisplayRole )
    {
        QSharedPointer<ChatLine> crec = chatVector->at(section);
        cell = crec->source;
        return cell;
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

    state = settings.value("CSTable/state").toByteArray();
    ui->CSTable->horizontalHeader()->restoreState(state);

    state = settings.value("messageTable/state").toByteArray();
    ui->messageTable->horizontalHeader()->restoreState(state);

    connect( ui->CSTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
    connect( ui->messageTable->horizontalHeader(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)), Qt::UniqueConnection);
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

void KSTMainWindow::analyseMessage(QString atj)
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
        kst->dtg = qdt;

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
                        analyseMessage(atj);
                    }
                }
            }
        }
        //cgm.setChatVector(chatVector);
    }
}
void KSTMainWindow::on_connectButton_clicked()
{

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
