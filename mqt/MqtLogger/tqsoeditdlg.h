#ifndef TQSOEDITDLG_H
#define TQSOEDITDLG_H

#include "base_pch.h"

class QTreeWidgetItem;
class MatchTreeItem;
class MatchTreeFrame;
class FocusWatcher;

namespace Ui {
class TQSOEditDlg;
}

class TQSOEditDlg : public QDialog
{
    Q_OBJECT

public:
    explicit TQSOEditDlg( QWidget* Owner, bool unfilled );

    ~TQSOEditDlg() override;

    int exec() override;

private:
    Ui::TQSOEditDlg *ui;

    int splitterHandleWidth;

    BaseContestLog * contest;
    QSharedPointer<BaseContact> firstContact;
    bool unfilled;

    MatchTreeFrame *xferTree = nullptr;
    FocusWatcher *OtherMatchTreeFW = nullptr;
    FocusWatcher *ArchiveMatchTreeFW = nullptr;

    void refreshOps( ScreenContact &screenContact );
    void keyPressEvent( QKeyEvent* event ) override;
    void addTreeRoot(QSharedPointer<BaseContact> lct);
    void addTreeChild(QTreeWidgetItem *parent, QString text);
    void doCloseEvent();
    void transferDetails(MatchTreeItem *MatchTreeIndex );
    void getSplitters();

    MatchTreeItem *getXferItem();
public: 		// User declarations
    void selectContact(BaseContestLog * contest, QSharedPointer<BaseContact> lct );
    void setContest( BaseContestLog * c )
    {
       contest = c;
    }
    void setFirstContact( QSharedPointer<BaseContact> c )
    {
       firstContact = c;
    }
private slots:
    void on_editSplitter_splitterMoved(int pos, int index);
    void on_EditFrameCancelled();
    void on_AfterSelectContact(QSharedPointer<BaseContact> lct, BaseContestLog *contest);
    void onXferPressed(BaseContestLog *, QString);
    void on_MatchStarting(BaseContestLog*);

    void on_archiveSplitter_splitterMoved(int pos, int index);
    void MatchTreeSelected(MatchType m, BaseContestLog *c, QString basename, const QItemSelection &selected);
    void onOtherMatchTreeFocused(QObject *, bool in, QFocusEvent *);
    void onArchiveTreeFocused(QObject *, bool in, QFocusEvent *);

public slots:
    virtual void accept() override;
    virtual void reject() override;


};

#endif // TQSOEDITDLG_H
