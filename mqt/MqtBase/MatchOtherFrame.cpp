#include "MinosParameters.h"
#include "MinosLoggerEvents.h"
#include "MatchOtherFrame.h"

MatchOtherFrame::MatchOtherFrame(QWidget *parent) :
    MatchTreeFrame(parent)
{
}

MatchOtherFrame::~MatchOtherFrame()
{
}
void MatchOtherFrame::initialise()
{
    MatchTreeFrame::initialise();
    connect(&MinosLoggerEvents::mle, SIGNAL(ReplaceOtherLogList(SharedMatchCollection,BaseContestLog*,QString)), this, SLOT(on_ReplaceOtherLogList(SharedMatchCollection,BaseContestLog*,QString)), Qt::QueuedConnection);
}

void MatchOtherFrame::showOtherMatchQSOs(SharedMatchCollection matchCollection )
{
    if (matchCollection->contactCount())
    {
        MinosLoggerEvents::sendXferEnabled(true, contest, baseName);
    }
    otherMatchModel.initialise(OtherMatch, matchCollection);
    getTreeView()->setModel(&otherMatchModel);
    getTreeView()->expandAll();
    restoreColumns();
    int rc = otherMatchModel.rowCount();
    for(int i = 0; i < rc; i++)
    {
        getTreeView()->setFirstColumnSpanned( i, QModelIndex(), true );
    }

    if (otherMatchModel.firstIndex.isValid())
    {
        QItemSelectionModel *m = getTreeView()->selectionModel();
        m->select(otherMatchModel.firstIndex, QItemSelectionModel::Select|QItemSelectionModel::Rows);
    }
}
void MatchOtherFrame::on_ReplaceOtherLogList( SharedMatchCollection matchCollection, BaseContestLog*, QString b )
{
    if (b == baseName && contest && contest == MinosParameters::getMinosParameters() ->getCurrentContest())
        showOtherMatchQSOs( matchCollection );
}
void MatchOtherFrame::on_matchTree_doubleClicked(const QModelIndex &/*index*/)
{
    // needs to be a MinosLogEvent, including Log/Edit
    MinosLoggerEvents::sendXferPressed(contest, baseName);
}
void MatchOtherFrame::on_matchTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    MinosLoggerEvents::sendMatchTreeSelected(OtherMatch, contest, baseName, selected);
}

void MatchOtherFrame::afterMatchTreeClicked()
{
    if ( treeClickIndex.isValid() )
    {
        MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem * > (treeClickIndex.internalPointer());

        QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
        if (mc)
        {
            QSharedPointer<BaseContact> bct = mc->getBaseContact();

            QString bearing = bct->getField(egBrg, contest);
            MinosLoggerEvents::SendBrgStrToRot(bearing);
        }
    }
}

void MatchOtherFrame::MatchTreeSelected(MatchType m, BaseContestLog *c, QString b, const QItemSelection &selected)
{
    if (contest == c && b == baseName)
    {
        bool sel = (m == OtherMatch);
        if (m != ThisMatch)
            setCurrentModel(sel);

        if (sel)
            treeClickIndex = selected.indexes().at(0);
        getTreeView()->viewport()->update();
    }
}
