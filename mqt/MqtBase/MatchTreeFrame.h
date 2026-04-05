#ifndef MATCHTREEFRAME_H
#define MATCHTREEFRAME_H

#include <QTreeView>
#include <QMenu>

#include "htmldelegate.h"
#include "baseconstants.h"
#include "GridColumn.h"
#include "contest.h"
#include "MatchCollection.h"
#include "minospanel.h"

namespace Ui {
class MatchTreeFrame;
}

class MatchTreeItem
{
    BaseMatchContest *matchContest;
    QSharedPointer<MatchContact> matchContact;

    MatchTreeItem *parent;
    QVector<MatchTreeItem *> children;
    int row;

public:
    MatchTreeItem(MatchTreeItem *parent, BaseMatchContest *matchContest, QSharedPointer<MatchContact> matchContact);
    ~MatchTreeItem();

    void addChild(MatchTreeItem *mi)
    {
        children.push_back(mi);
        mi->setRow(children.size() - 1);
    }
    int childCount()
    {
        return children.size();
    }

    QSharedPointer<MatchContact> getMatchContact();
    BaseMatchContest *getMatchContest();
    MatchTreeItem *getParent();
    MatchTreeItem *child(int i)
    {
        return children[i];
    }
    int getRow()
    {
        return row;
    }
    void setRow(int r)
    {
        row = r;
    }
};

class QSOMatchGridModel: public QAbstractItemModel
{
    Q_OBJECT

protected:
    SharedMatchCollection match;
    MatchTreeItem * rootItem;
    MatchType type;

public:
    static QVector<GridColumn>  ThisMatchTreeColumns;
    static QVector<GridColumn>  OtherMatchTreeColumns;
    static QVector<GridColumn>  ArchiveMatchTreeColumns;

    QSOMatchGridModel();
    ~QSOMatchGridModel() override;

    QString baseName;

    QModelIndex firstIndex;
    bool currentModel;

    void initialise( MatchType, SharedMatchCollection pmatch );
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
};

class MatchTreeFrame : public MinosPanel
{
    Q_OBJECT
    Ui::MatchTreeFrame *ui;

    QSharedPointer<HtmlDelegate> delegate;

    void viewColumn();
public:
    QModelIndex treeClickIndex;

    explicit MatchTreeFrame(QWidget *parent = nullptr);
    virtual ~MatchTreeFrame();
    virtual void initialise();
    QTreeView *getTreeView();
    virtual QString getTreeName(){return "";}
    void setCurrentModel(bool);
    virtual QSOMatchGridModel *getMatchModel() = 0;
    virtual MatchType getMatchType() = 0;

    void setBaseName(QString);
    void setContest(BaseContestLog *);
    void restoreColumns();

    void doCustomContextMenuRequested();

    void setCurScreenLayout(const QString &value);
    void saveHeaderLayout();

protected:
    QString baseName;
    BaseContestLog *contest;
    QMenu columnsMenu;
    QString curScreenLayout;
    bool inRestoreColumns = false;

    virtual void showThisMatchQSOs(SharedMatchCollection /*matchCollection*/ ){}
    virtual void showOtherMatchQSOs( SharedMatchCollection /*matchCollection*/ ){}
    virtual void showMatchList(SharedMatchCollection /*matchCollection*/ ){}

private slots:
    virtual void on_MatchTreeSelectionChanged(const QItemSelection &, const QItemSelection &){}

    virtual void on_MatchTreeFrame_doubleClicked(const QModelIndex &){}

    virtual void on_MatchTreeFrame_clicked(const QModelIndex &);

    virtual void afterMatchTreeClicked(){}

    virtual void MatchTreeSelected(MatchType, BaseContestLog *, QString, const QItemSelection &){}

    void on_sectionResized(int, int, int);

    void on_doColumnChanges(BaseContestLog*);
    void onMatch_customContextMenuRequested(const QPoint &pos);
    void onSectionMoved(int, int, int);
signals:
    void editContact(CheckableContact *bct, bool nc);

    void matchTreeClicked();
};

#endif // MATCHTREEFRAME_H
