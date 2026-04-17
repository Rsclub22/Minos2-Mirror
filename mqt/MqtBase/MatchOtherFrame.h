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
    virtual MatchType getMatchType() override
    {
        return OtherMatch;
    }
    virtual void doDoubleClick(const QModelIndex &index) override;
    virtual void doMatchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;
    virtual void doAfterMatchTreeClicked() override;
    virtual void doMatchTreeSelected(MatchType mt, BaseContestLog *l, QString s, const QItemSelection &is) override;

private slots:

    void on_ReplaceOtherLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
};

#endif // MATCHOTHERFRAME_H
