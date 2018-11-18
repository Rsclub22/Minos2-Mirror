#include "ContestApp.h"
#include "MinosLoggerEvents.h"
#include "ListContact.h"
#include "MatchArchiveFrame.h"

MatchArchiveFrame::MatchArchiveFrame(QWidget *parent) :
    MatchTreeFrame(parent)
{
}

MatchArchiveFrame::~MatchArchiveFrame()
{
}
void MatchArchiveFrame::initialise()
{
    MatchTreeFrame::initialise();
    connect(&MinosLoggerEvents::mle, SIGNAL(ReplaceListList(SharedMatchCollection,BaseContestLog*,QString)), this, SLOT(on_ReplaceListList(SharedMatchCollection,BaseContestLog*,QString)), Qt::QueuedConnection);
}

void MatchArchiveFrame::showMatchList( SharedMatchCollection matchCollection )
{
    if (matchCollection->contactCount())
    {
        MinosLoggerEvents::sendXferEnabled(true, contest, baseName);
    }
    archiveMatchModel.initialise(ArchiveMatch, matchCollection);
    getTreeView()->setModel(&archiveMatchModel);
    getTreeView()->expandAll();
    restoreColumns();
    for(int i = 0; i < archiveMatchModel.rowCount(); i++)
    {
        getTreeView()->setFirstColumnSpanned( i, QModelIndex(), true );
    }

    if (archiveMatchModel.firstIndex.isValid())
    {
        QItemSelectionModel *m = getTreeView()->selectionModel();
        m->select(archiveMatchModel.firstIndex, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}

void MatchArchiveFrame::on_ReplaceListList( SharedMatchCollection matchCollection, BaseContestLog*, QString b )
{
    if (b == baseName && contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        showMatchList( matchCollection );
}


void MatchArchiveFrame::afterMatchTreeClicked()
{
    if ( treeClickIndex.isValid() )
    {
        MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(treeClickIndex.internalPointer());
        QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
        if (mc)
        {
            ListContact *lct = mc->getListContact();

            QString bearing = lct->getField(egBrg, contest);
            MinosLoggerEvents::SendBrgStrToRot(bearing);
        }
    }
}
void MatchArchiveFrame::on_matchTree_doubleClicked(const QModelIndex &/*index*/)
{
    MinosLoggerEvents::sendXferPressed(contest, baseName);
}

void MatchArchiveFrame::on_matchTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    MinosLoggerEvents::sendMatchTreeSelected(ArchiveMatch, contest, baseName, selected);
}


void MatchArchiveFrame::MatchTreeSelected(MatchType m, BaseContestLog *c, QString b, const QItemSelection &selected)
{
    if (contest == c && b == baseName)
    {
        bool sel = (m == ArchiveMatch);

        if (m != ThisMatch)
            setCurrentModel(sel);

        if (sel)
            treeClickIndex = selected.indexes().at(0);
        getTreeView()->viewport()->update();
    }
}
