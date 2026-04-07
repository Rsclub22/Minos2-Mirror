#include "MinosParameters.h"
#include "MinosLoggerEvents.h"
#include "qsogridmodel.h"
#include "ListContact.h"
#include "MatchArchiveFrame.h"

MatchArchiveFrame::MatchArchiveFrame(QWidget *parent) :
    MatchTreeFrame(parent)
{
    SharedMatchCollection matchCollection;
    archiveMatchModel.initialise(ArchiveMatch, matchCollection);
    getTreeView()->setModel(&archiveMatchModel);
}

MatchArchiveFrame::~MatchArchiveFrame()
{
}
void MatchArchiveFrame::initialise()
{
    MatchTreeFrame::initialise();
    connect(&MinosLoggerEvents::mle, &MinosLoggerEvents::ReplaceListList, this, &MatchArchiveFrame::on_ReplaceListList, Qt::QueuedConnection);
}

void MatchArchiveFrame::showMatchList( SharedMatchCollection matchCollection )
{
    if (matchCollection && matchCollection->contactCount())
    {
        MinosLoggerEvents::SendXferEnabled(true, contest, baseName);
    }
    archiveMatchModel.initialise(ArchiveMatch, matchCollection);
    getTreeView()->setModel(&archiveMatchModel);
    if (!matchCollection)
        return;

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
    if (b == baseName && contest && contest == MinosParameters::getMinosParameters() ->getCurrentContest())
        showMatchList( matchCollection );
}


void MatchArchiveFrame::doAfterMatchTreeClicked()
{
    if ( treeClickIndex.isValid() )
    {
        MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * >(treeClickIndex.internalPointer());
        QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
        if (mc)
        {
            ListContact *lct = mc->getListContact();

            QString bearing = lct->getField(egBrg, contest);
            MinosLoggerEvents::SendMatchBrgStrToRot(bearing);
        }
    }
}
void MatchArchiveFrame::doDoubleClick(const QModelIndex &/*index*/)
{
    MinosLoggerEvents::SendXferPressed(contest, baseName);
}

void MatchArchiveFrame::doMatchTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    MinosLoggerEvents::SendMatchTreeSelected(ArchiveMatch, contest, baseName, selected);
}


void MatchArchiveFrame::doMatchTreeSelected(MatchType m, BaseContestLog *c, QString b, const QItemSelection &selected)
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
