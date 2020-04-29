#ifndef MATCHTREEFRAME_H
#define MATCHTREEFRAME_H

#include "base_pch.h"
#include "contest.h"
#include "MatchCollection.h"

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

    bool isMatchLine();
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
    static GridColumn ThisMatchTreeColumns[ THISMATCHTREECOLS ];
    static GridColumn OtherMatchTreeColumns[ OTHERMATCHTREECOLS ];
    static GridColumn ArchiveMatchTreeColumns[ ARCHIVEMATCHTREECOLS ];

protected:
    SharedMatchCollection match;
    MatchTreeItem * rootItem;
    MatchType type;

public:
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

class MatchTreeFrame : public QTreeView
{
    Q_OBJECT
    Ui::MatchTreeFrame *ui;

    QSharedPointer<HtmlDelegate> delegate;

public:
    QModelIndex treeClickIndex;

    explicit MatchTreeFrame(QWidget *parent = nullptr);
    virtual ~MatchTreeFrame();
    virtual void initialise();
    QTreeView *getTreeView();
    virtual QString getTreeName(){return "";}
    void setCurrentModel(bool);
    virtual QSOMatchGridModel *getMatchModel(){return nullptr;}

    void setBaseName(QString);
    void setContest(BaseContestLog *);
    void restoreColumns();
//    void getSplitters();

    void doCustomContextMenuRequested();

//    bool logColumnsChanged;


protected:
    QString baseName;
    BaseContestLog *contest;


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
signals:
    void editContact(QSharedPointer<BaseContact> bct);

    void matchTreeClicked();
};

#endif // MATCHTREEFRAME_H
