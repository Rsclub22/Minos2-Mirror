#ifndef MATCHARCHIVEFRAME_H
#define MATCHARCHIVEFRAME_H

#include <QFrame>
#include "MatchTreeFrame.h"

class MatchArchiveFrame : public MatchTreeFrame
{
    Q_OBJECT

    QSOMatchGridModel archiveMatchModel;
    virtual QString getTreeName() override
    {
        return "ArchiveMatchTree";
    }
    void showMatchList(SharedMatchCollection matchCollection) override;
public:
    explicit MatchArchiveFrame(QWidget *parent = nullptr);
    virtual void initialise() override;
    virtual ~MatchArchiveFrame() override;
    virtual QSOMatchGridModel *getMatchModel() override
    {
        return &archiveMatchModel;
    }
    virtual MatchType getMatchType() override
    {
        return ArchiveMatch;
    }
    virtual void doDoubleClick(const QModelIndex &index) override;
    virtual void doMatchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;
    virtual void doAfterMatchTreeClicked() override;
    virtual void doMatchTreeSelected(MatchType mt, BaseContestLog *l, QString s, const QItemSelection &is) override;

    void on_ReplaceListList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
};

#endif // MATCHARCHIVEFRAME_H
