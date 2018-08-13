#include "base_pch.h"
#include "MinosLoggerEvents.h"
#include "ContestApp.h"
#include "ListContact.h"
#include "list.h"
#include "contest.h"
#include "htmldelegate.h"
#include "MatchThread.h"
#include "tlogcontainer.h"
#include "cutils.h"

#include "MatchTreeFrame.h"
#include "ui_MatchTreeFrame.h"

MatchTreeFrame::MatchTreeFrame(QWidget *parent) :
    QTreeView(parent),
    ui(new Ui::MatchTreeFrame),
    contest(nullptr)
{
    ui->setupUi(this);

}
void MatchTreeFrame::initialise()
{
    setModel(getMatchModel());
    header()->setSectionResizeMode(QHeaderView::Interactive);
    setItemDelegate( new HtmlDelegate );

    connect( header(), SIGNAL(sectionResized(int, int , int)),
             this, SLOT( on_sectionResized(int, int , int)));

    connect(this, SIGNAL(matchTreeClicked()), this, SLOT(afterMatchTreeClicked()), Qt::QueuedConnection);

    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this, SLOT(on_matchTreeSelectionChanged(const QItemSelection &, const QItemSelection &)), Qt::UniqueConnection);

    connect(&MinosLoggerEvents::mle, SIGNAL(MatchTreeSelected(MatchType , BaseContestLog *, QString, QItemSelection)),
            this, SLOT(MatchTreeSelected(MatchType, BaseContestLog *, QString, QItemSelection)));
}
MatchTreeFrame::~MatchTreeFrame()
{
    delete ui;
}
QTreeView *MatchTreeFrame::getTreeView()
{
    return this;
}


void MatchTreeFrame::setBaseName(QString b)
{
    baseName = b;
}

void MatchTreeFrame::setContest(BaseContestLog *ct)
{
    contest = ct;
}
void MatchTreeFrame::restoreColumns()
{
    QSettings settings;
    QByteArray state;

    QString treeName = getTreeName();

    state = settings.value(baseName + "/" + getTreeName() + "/state").toByteArray();
    header()->restoreState(state);
    QFont cf = QApplication::font();
    header()->setFont(cf);

}
void MatchTreeFrame::setCurrentModel(bool s)
{
    getMatchModel()->currentModel = s;
}
//---------------------------------------------------------------------------
void MatchTreeFrame::doCustomContextMenuRequested()
{
    QModelIndex index;

    index = treeClickIndex;

    if (index.isValid())
    {
        MatchTreeItem * MatchTreeIndex = static_cast<MatchTreeItem *>( index.internalPointer() );

        QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
        if (mc)
        {
            QSharedPointer<BaseContact> bct = mc->getBaseContact();
            ListContact *lct = mc->getListContact();
            if (bct)
            {
                LogContainer->setMemoryAction->call = bct->cs.fullCall.getValue();
                LogContainer->setMemoryAction->loc = bct->loc.loc.getValue();
            }
            else if (lct)
            {
                LogContainer->setMemoryAction->call = lct->cs.fullCall.getValue();
                LogContainer->setMemoryAction->loc = lct->loc.loc.getValue();
            }
            LogContainer->setMemoryAction->ct = contest;
            LogContainer->setMemoryAction->setVisible(true);
        }
    }
}
//---------------------------------------------------------------------------
void MatchTreeFrame::on_sectionResized(int, int, int)
{
    QSettings settings;
    QByteArray state;
    QString treeName = getTreeName();

    state = header()->saveState();
    settings.setValue(baseName + "/" + treeName + "/state", state);

    MinosLoggerEvents::SendLogColumnsChanged();
}
void MatchTreeFrame::on_matchTree_clicked(const QModelIndex &)
{
    emit matchTreeClicked();
}

//=============================================================================
static GridColumn ThisMatchTreeColumns[ THISMATCHTREECOLS ] =
{
    GridColumn( egTime, "XXXXXXXXXX", "UTC", taLeftJustify ),               // time
    GridColumn( egCall, "MMMMMMMMMMM", "Callsign", taLeftJustify ),         // call
    GridColumn( egRSTTx, "599XXX", "RepTx", taLeftJustify ),                 // RST
    GridColumn( egSNTx, "1234X", "SnTx", taLeftJustify /*taRightJustify*/ ),   // serial
    GridColumn( egRSTRx, "599XXX", "RepRx", taLeftJustify ),                 // RST
    GridColumn( egSNRx, "1234X", "SnRx", taLeftJustify /*taRightJustify*/ ),   // Serial
    GridColumn( egLoc, "MM00MM00X", "Loc", taLeftJustify ),            // LOC
    GridColumn( egScore, "12345XX", "dist", taLeftJustify /*taRightJustify*/ ),  // score
    GridColumn( egBrg, "3601X", "brg", taLeftJustify ),                // bearing
    GridColumn( egExchange, "XXXXXXXXXXXXXXXX", "Exchange", taLeftJustify ),    // QTH
    GridColumn( egComments, "XXXXXXXXXXXXXXXX", "Comments", taLeftJustify ),     // comments
    GridColumn( egFrequency, "1.296.123.456XXX", "Freq", taLeftJustify ),
    GridColumn( egRotatorHeading, "XXXXXX", "Rot Heading", taLeftJustify ),
    GridColumn( egRigName, "XXXXXX", "Rig", taLeftJustify )
};
//---------------------------------------------------------------------------
static GridColumn OtherMatchTreeColumns[ OTHERMATCHTREECOLS ] =
{
    GridColumn( egTime, "XXXXXXXXXX", "UTC", taLeftJustify ),               // time
    GridColumn( egCall, "MMMMMMMMMMM", "Callsign", taLeftJustify ),         // call
    GridColumn( egLoc, "MM00MM00X", "Loc", taLeftJustify ),            // LOC
    GridColumn( egScore, "12345XX", "dist", taLeftJustify /*taRightJustify*/ ),  // score
    GridColumn( egBrg, "3601X", "brg", taLeftJustify ),                // bearing
    GridColumn( egExchange, "XXXXXXXXXXXXXXXX", "Exchange", taLeftJustify ),    // QTH
    GridColumn( egComments, "XXXXXXXXXXXXXXXX", "Comments", taLeftJustify ),     // comments
    GridColumn( egFrequency, "1.296.123.456XXX", "Freq", taLeftJustify ),
    GridColumn( egRotatorHeading, "XXXXXX", "Rot Heading", taLeftJustify ),
    GridColumn( egRigName, "XXXXXX", "Rig", taLeftJustify )
};
//---------------------------------------------------------------------------
static GridColumn ArchiveMatchTreeColumns[ ARCHIVEMATCHTREECOLS ] =
{
    GridColumn( egCall, "MMMMMMMMMMM", "Callsign", taLeftJustify ),         // call
    GridColumn( egLoc, "MM00MM00X", "Loc", taLeftJustify ),            // LOC
    GridColumn( egScore, "12345XX", "dist", taLeftJustify /*taRightJustify*/ ),  // score
    GridColumn( egBrg, "3601X", "brg", taLeftJustify ),                // bearing
    GridColumn( egExchange, "XXXXXX", "Exchange", taLeftJustify ),     // exchange
    GridColumn( egComments, "XXXX", "Comments", taLeftJustify )     // comments
};
//---------------------------------------------------------------------------

MatchTreeItem::MatchTreeItem(MatchTreeItem *parent, BaseMatchContest *matchContest, QSharedPointer<MatchContact> matchContact)
    :
      matchContest(matchContest),
      matchContact(matchContact),
      parent(parent),
      row(-1)
{

}

MatchTreeItem::~MatchTreeItem()
{
    for (int i = 0; i < children.size(); i++)
    {
        delete children[i];
        children[i] = nullptr;
    }
}

bool MatchTreeItem::isMatchLine()
{
    return matchContact != nullptr;
}
MatchTreeItem *MatchTreeItem::getParent()
{
    return parent;
}

QSharedPointer<MatchContact> MatchTreeItem::getMatchContact()
{
    return matchContact;
}
BaseMatchContest *MatchTreeItem::getMatchContest()
{
    return matchContest;
}


QSOMatchGridModel::QSOMatchGridModel():
    match(nullptr),
    rootItem(nullptr),
    type(ThisMatch),
    currentModel(false)
{}
QSOMatchGridModel::~QSOMatchGridModel()
{
    delete rootItem;
}

void QSOMatchGridModel::initialise(MatchType t, SharedMatchCollection pmatch )
{
    beginResetModel();
    type = t;
    firstIndex = QModelIndex();

    match.reset();

    if (rootItem)
    {
        delete rootItem;
        rootItem = nullptr;
    }

    if (pmatch->contactCount() == 0)
    {
        endResetModel();
        return;
    }

    match = pmatch;  // preserve all the tree
    rootItem = new MatchTreeItem(nullptr, nullptr, QSharedPointer<MatchContact>());
    rootItem->setRow(0);
    for (ContestMatchIterator i = pmatch->contestMatchList.begin(); i != pmatch->contestMatchList.end(); i++)
    {
        MatchTreeItem *ci = new MatchTreeItem(rootItem, i->wt.data(), QSharedPointer<MatchContact>());
        rootItem->addChild(ci); // also sets row
        //(*i) is *BaseMatchContest
        foreach(auto mct, i->wt->contactMatchList)
        {
            MatchTreeItem *mi = new MatchTreeItem(ci, i->wt.data(), mct.wt);
            ci->addChild(mi);
            if (!firstIndex.isValid())
            {
                firstIndex = createIndex(mi->getRow(), 0, mi);
            }
        }
    }
    endResetModel();
}
QVariant QSOMatchGridModel::data( const QModelIndex &index, int role ) const
{
    QModelIndex p = index.parent();

    MatchTreeItem *thisItem = static_cast<MatchTreeItem*>(index.internalPointer());

    MatchTreeItem *parentItem;
    if (p.isValid())
        parentItem = static_cast<MatchTreeItem*>(p.internalPointer());
    else
        parentItem = rootItem;

    int row = index.row();
    int column = index.column();

    if ( row >= parentItem->childCount() )
        return QVariant();

    BaseMatchContest *matchContest = thisItem->getMatchContest();
    QSharedPointer<MatchContact> mct = thisItem->getMatchContact();
    QSharedPointer<BaseContact> ct;
    ListContact *lct = nullptr;

    if (mct)
    {
        ct = mct->getBaseContact();
        lct = mct->getListContact();
    }

    if (role == Qt::BackgroundRole)
    {
        if (ct && type == ThisMatch)
        {
            if ( ct->contactFlags.getValue() & FORCE_LOG )
            {
                return static_cast< QColor > ( 0x00FF80C0 );        // Pink(ish)
            }
            else
            {
                if ( ct->getModificationCount() > 1 )
                {
                    return static_cast< QColor > ( 0x00C0DCC0 );    // "money green"
                }
            }
        }
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;       // but HtmlDelegate overrides


    if (role == Qt::DisplayRole)
    {
        QColor lightRed = QColor(Qt::red).lighter(100);
        if (type == ArchiveMatch)
        {
            const ContactList *contactList = matchContest->getContactList();
            if (lct)
            {
                if( column >= 0 && column < columnCount(p))
                {
                    BaseContestLog * ct = TContestApp::getContestApp() ->getCurrentContest();
                    QString cell;
                    contactList->getMatchField( lct, ArchiveMatchTreeColumns[ column ].fieldId, cell, ct );     // col 0 is the tree lines

                    if (currentModel)
                    {
                        cell = HtmlFontColour(lightRed) + "<b>" + cell;
                    }
                    return cell;
                }
            }
            else
            {
                if (column == 0)
                {
                    QString cell = contactList->name;
                    if (currentModel)
                    {
                        cell = HtmlFontColour(lightRed) + "<b>" + cell;
                    }
                    return cell;
                }
            }
        }
        else
        {
            const BaseContestLog *contest = matchContest->getContactLog();
            if (ct)
            {
                if( column >= 0 && column < columnCount(p))
                {

                    QString line;
                    BaseContestLog * act = TContestApp::getContestApp() ->getCurrentContest();


                    if (type == ThisMatch)
                        contest->getMatchField( ct, ThisMatchTreeColumns[ column ].fieldId, line, act );     // col 0 is the tree lines
                    else
                        contest->getMatchField( ct, OtherMatchTreeColumns[ column ].fieldId, line, act );     // col 0 is the tree lines

                    if (type == ThisMatch)
                    {
                        QColor multhighlight = Qt::red;
                        bool setHighlight = false;
                        switch ( QSOTreeColumns[ column ].fieldId )
                        {
                        case egTime:
                            if (!contest->checkTime(ct->time))
                            {
                                setHighlight = true;
                            }
                            break;
                        case egCall:
                            if ( contest->countryMult.getValue() && ct->newCtry )
                                setHighlight = true;
                            break;
                        case egExchange:
                            if ( contest->districtMult.getValue() && ct->newDistrict )
                                setHighlight = true;
                            break;
                        case egLoc:
                            if ( contest->locMult.getValue() && ct->locCount > 0)
                            {
                                setHighlight = true;
                            }
                            else if ( contest->usesBonus.getValue() && ct->bonus > 0)
                            {
                                switch (ct->bonus)
                                {
                                case 500:  //blue
                                    multhighlight = Qt::blue;
                                    break;
                                case 1000: //green
                                    multhighlight = Qt::darkGreen;
                                    break;
                                case 2000: //red
                                    multhighlight = Qt::red;
                                    break;
                                }

                                setHighlight = true;
                            }
                            break;
                        }
                        if (setHighlight)
                            line = HtmlFontColour(multhighlight) + "<b>" + line;
                    }
                    else
                    {
                        if (currentModel)
                        {
                            line = HtmlFontColour(lightRed) + "<b>" + line;
                        }

                    }
                    return line;
                }
            }
            else
            {
                if (column == 0)
                {
                    if (type == ThisMatch)
                        return "Current contest" + TMatchThread::getThisMatchStatus();
                    if (type == OtherMatch)
                    {
                        QString name = contest->name.getValue();
                        QString band = contest->band.getValue();

                        QString cell = "[" + band + "] " + name;
                        if (currentModel)
                        {
                            cell = HtmlFontColour(lightRed) + "<b>" + cell;
                        }
                        return cell;
                    }
                }
            }
        }
    }
    return QVariant();
}
QVariant QSOMatchGridModel::headerData( int section, Qt::Orientation orientation,
                                        int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString cell;
        if (section < columnCount())
        {
            switch (type)
            {
            case ThisMatch:
                cell = ThisMatchTreeColumns[ section ].title;
                break;

            case OtherMatch:
                cell = OtherMatchTreeColumns[ section ].title;
                break;

            case ArchiveMatch:
                cell = ArchiveMatchTreeColumns[ section ].title;
                break;
            }
        }

        return cell;
    }
    if (role == Qt::TextAlignmentRole)
        return Qt::AlignLeft;
    return QVariant();
}

QModelIndex QSOMatchGridModel::index( int row, int column, const QModelIndex &parent) const
{

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    MatchTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<MatchTreeItem*>(parent.internalPointer());

    MatchTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex QSOMatchGridModel::parent( const QModelIndex &index ) const
{
    MatchTreeItem *parentItem = (static_cast<MatchTreeItem*>(index.internalPointer()))->getParent();
    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->getRow(), 0, parentItem);
}

int QSOMatchGridModel::rowCount( const QModelIndex &parent ) const
{
    if ( !rootItem)
        return 0;

    if (!parent.isValid())
        return rootItem->childCount();

    MatchTreeItem *parentItem = static_cast<MatchTreeItem*>(parent.internalPointer());
    return parentItem->childCount();
}

int QSOMatchGridModel::columnCount( const QModelIndex &/*parent*/ ) const
{
    int cols =0;
    switch (type)
    {
    case ThisMatch:
        cols = THISMATCHTREECOLS;
        break;

    case OtherMatch:
        cols = OTHERMATCHTREECOLS;
        break;

    case ArchiveMatch:
        cols = ARCHIVEMATCHTREECOLS;
        break;
    }
    return cols;
}

