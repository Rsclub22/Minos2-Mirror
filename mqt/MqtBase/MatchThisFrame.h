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
    void showThisMatchQSOs(SharedMatchCollection matchCollection);
public:
    explicit MatchThisFrame(QWidget *parent = nullptr);
    virtual void initialise() override;
    virtual ~MatchThisFrame() override;
    virtual QSOMatchGridModel *getMatchModel() override
    {
        return &thisMatchModel;
    }
private slots:
    virtual void on_matchTreeSelectionChanged(const QItemSelection &, const QItemSelection &) override;

    virtual void on_matchTree_doubleClicked(const QModelIndex &index) override;

    virtual void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &selected) override;

    void on_ReplaceThisLogList(SharedMatchCollection matchCollection, BaseContestLog *, QString b);
    void afterMatchTreeClicked();
};

#endif // MATCHTHISFRAME_H
