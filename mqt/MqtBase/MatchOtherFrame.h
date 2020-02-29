#ifndef MATCHOTHERFRAME_H
#define MATCHOTHERFRAME_H

#include <QFrame>
#include "MatchTreeFrame.h"

class MatchOtherFrame : public MatchTreeFrame
{
    Q_OBJECT

    QSOMatchGridModel otherMatchModel;


    virtual QString getTreeName() override
    {
        return "OtherMatchTree";
    }
    void showOtherMatchQSOs(SharedMatchCollection matchCollection) override;
public:
    explicit MatchOtherFrame(QWidget *parent = nullptr);
    virtual void initialise() override;
    virtual ~MatchOtherFrame() override;
    virtual QSOMatchGridModel *getMatchModel() override
    {
        return &otherMatchModel;
    }
private slots:

    virtual void on_matchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;

    virtual void on_matchTree_doubleClicked(const QModelIndex &index) override;

    virtual void afterMatchTreeClicked() override;

    virtual void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &selected) override;

    void on_ReplaceOtherLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
};

#endif // MATCHOTHERFRAME_H
