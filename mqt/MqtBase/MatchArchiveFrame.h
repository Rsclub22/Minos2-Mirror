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
private slots:
    virtual void on_matchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;

    virtual void on_matchTree_doubleClicked(const QModelIndex &index) override;

    virtual void afterMatchTreeClicked() override;

    virtual void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &selected) override;

    void on_ReplaceListList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
};

#endif // MATCHARCHIVEFRAME_H
