#ifndef MINOSTABLEVIEW_H
#define MINOSTABLEVIEW_H
#include "base_pch.h"

class MinosTableView:public QTableView
{
    Q_OBJECT
    QPersistentModelIndex centreCell;
public:
    MinosTableView(QWidget *parent);
    virtual ~MinosTableView() override
    {}
    void scrollTo(const QModelIndex &index, ScrollHint hint) Q_DECL_OVERRIDE;

protected:
    virtual void scrollContentsBy(int x, int y) Q_DECL_OVERRIDE;
    void showEvent(QShowEvent *) Q_DECL_OVERRIDE;


signals:
    void minosViewScrolled();

};

#endif // MINOSTABLEVIEW_H
