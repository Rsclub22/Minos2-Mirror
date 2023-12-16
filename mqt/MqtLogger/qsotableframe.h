#ifndef QSOTABLEFRAME_H
#define QSOTABLEFRAME_H

#include <QFrame>
#include <QMenu>

#include "MinosTableView.h"
#include "qsogridmodel.h"
#include "contacts.h"
#include "htmldelegate.h"

namespace Ui {
class QSOTableFrame;
}

class BaseContestLog;
class QTableView;

class QSOTableFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QSOTableFrame(QWidget *parent = nullptr);
    ~QSOTableFrame();

    void setContest(BaseContestLog *contest);
    void setCurScreenLayout(const QString &value);

    void buildFrame();
    void setModel();
    void refreshModel();

    void startNextEntry();
    void onContestChanged();

    void insertRow(int rowNum);
    void restoreQSOTableColumns();
private:
    Ui::QSOTableFrame *ui;
    BaseContestLog *contest = nullptr;
    QString curScreenLayout;

    QMenu columnsMenu;
    MinosTableView *QSOTable = nullptr;

    bool inRestoreColumns = false;
    bool columnsChanged = false;

    QSharedPointer<HtmlDelegate> delegate;
    QSOGridModel qsoModel;

    void saveQSOTableColumns();
    void QSOTreeSelectContact( QSharedPointer<BaseContact> lct );

private slots:
    void viewColumn();
    void on_AfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *ct);

    void onQSOTable_doubleClicked(const QModelIndex &index);
    void onQSOGrid_sectionMoved(int, int, int);
    void onQSOGrid_customContextMenuRequested(const QPoint &pos);
    void onColumnsChanged();
    void on_sectionResized(int a, int b, int c);
};

#endif // QSOTABLEFRAME_H
