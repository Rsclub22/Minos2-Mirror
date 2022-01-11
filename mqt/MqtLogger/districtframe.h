#ifndef DISTRICTFRAME_H
#define DISTRICTFRAME_H

#include "base_pch.h"

namespace Ui {
class DistrictFrame;
}

class BaseContestLog;
class TSingleLogFrame;
class StackedInfoFrame;

class DistrictGridModel: public QAbstractItemModel
{
    Q_OBJECT
    static QVector<GridColumn> DistrictTreeColumns;
    public:
        DistrictGridModel();
        ~DistrictGridModel() override;
        QSharedPointer<HtmlDelegate> delegate ;

        BaseContestLog *ct;
        QString band;

        QVector<QSharedPointer<DistrictEntry> > districts;

        void reset();
        void initialise( );
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
};

class DistrictSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    QString scrolledDistrict;
    QString band;
    DistrictSortFilterProxyModel(): scrolledDistrict(-1)
    {
    }
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
};

class DistrictFrame : public QFrame
{
    Q_OBJECT

    QString band;
    DistrictGridModel model;
    DistrictSortFilterProxyModel proxyModel;
    QSharedPointer<HtmlDelegate> delegate;
    QMenu columnsMenu;
    bool inRestoreColumns = false;
    TSingleLogFrame *tslf = nullptr;

public:
    explicit DistrictFrame(StackedInfoFrame *parent);
    ~DistrictFrame();

    void setContest(BaseContestLog *contest);
    void setBand(QString band);

    void reInitialiseDistricts();
    void scrollToDistrict( const QString &cd, bool makeVisible );
private:
    Ui::DistrictFrame *ui;
    void doScrollToDistrict();
    
    void restoreDistrictTableColumns();
    void saveDistrictTableColumns();

private slots:
    void on_sectionResized(int, int , int);
    void on_DistrictTable_clicked(const QModelIndex &index);
    void onDistrictGrid_customContextMenuRequested(const QPoint &pos);
    void onDistrictGrid_sectionMoved(int, int, int);

    void viewColumn();

    void on_doColumnChanges(BaseContestLog *b);
};

#endif // DISTRICTFRAME_H
