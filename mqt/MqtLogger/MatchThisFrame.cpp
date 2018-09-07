#include "ContestApp.h"
#include "MinosLoggerEvents.h"

#include "MatchThisFrame.h"

MatchThisFrame::MatchThisFrame(QWidget *parent) :
    MatchTreeFrame (parent)
{
}

MatchThisFrame::~MatchThisFrame()
{
}
void MatchThisFrame::initialise()
{
    MatchTreeFrame::initialise();
    connect(&MinosLoggerEvents::mle, SIGNAL(ReplaceThisLogList(SharedMatchCollection,BaseContestLog*,QString)), this, SLOT(on_ReplaceThisLogList(SharedMatchCollection,BaseContestLog*,QString)), Qt::QueuedConnection);
}

void MatchThisFrame::showThisMatchQSOs( SharedMatchCollection matchCollection )
{
    thisMatchModel.initialise(ThisMatch, matchCollection);
    getTreeView()->setModel(&thisMatchModel);
    getTreeView()->setFirstColumnSpanned( 0, QModelIndex(), true );
    getTreeView()->expandAll();
    restoreColumns();
}
void MatchThisFrame::on_ReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog*, QString b )
{
    if (b == baseName && contest && contest == TContestApp::getContestApp() ->getCurrentContest())
        showThisMatchQSOs( matchCollection );
}
void MatchThisFrame::on_matchTreeSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    MinosLoggerEvents::sendMatchTreeSelected(ThisMatch, contest, baseName, selected);
}
void MatchThisFrame::afterMatchTreeClicked()
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
void MatchThisFrame::on_matchTree_doubleClicked(const QModelIndex &index)
{
    MatchTreeItem * MatchTreeIndex = static_cast< MatchTreeItem *>(index.internalPointer());

    QSharedPointer<MatchContact> mc = MatchTreeIndex->getMatchContact();
    QSharedPointer<BaseContact> bct = mc->getBaseContact();

    if ( bct )
    {
        emit editContact( bct );
    }
}

void MatchThisFrame::MatchTreeSelected(MatchType m, BaseContestLog *c, QString b, const QItemSelection &selected)
{
    if (contest == c && b == baseName)
    {
        bool sel = (m == ThisMatch);
        //setCurrentModel(sel);

        if (sel)
            treeClickIndex = selected.indexes().at(0);
        getTreeView()->viewport()->repaint();
    }
}
