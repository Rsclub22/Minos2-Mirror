#ifndef MATCHTHISFRAME_H
#define MATCHTHISFRAME_H

#include <QFrame>
#include "MatchTreeFrame.h"

class MatchThisFrame : public MatchTreeFrame
{
    Q_OBJECT

    QSOMatchGridModel thisMatchModel;


    virtual QString getTreeName() override
    {
        return "ThisMatchTree";
    }
    void showThisMatchQSOs(SharedMatchCollection matchCollection) override;
public:
    explicit MatchThisFrame(QWidget *parent = nullptr);
    virtual void initialise() override;
    virtual ~MatchThisFrame() override;
    virtual QSOMatchGridModel *getMatchModel() override
    {
        return &thisMatchModel;
    }
    virtual MatchType getMatchType() override
    {
        return ThisMatch;
    }
    virtual void doDoubleClick(const QModelIndex &index) override;
    virtual void doMatchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;
    virtual void doAfterMatchTreeClicked() override;
    virtual void doMatchTreeSelected(MatchType mt, BaseContestLog *l, QString s, const QItemSelection &is) override;

private slots:
    void on_ReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
};

#endif // MATCHTHISFRAME_H
