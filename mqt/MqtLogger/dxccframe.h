#ifndef DXCCFRAME_H
#define DXCCFRAME_H

#include "TreeUtils.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QMenu>

namespace Ui {
class DXCCFrame;
}

class LoggerContestLog;
class StackedInfoFrame;
class TSingleLogFrame;

class DXCCGridModel: public QAbstractItemModel
{
    Q_OBJECT
        static QVector<GridColumn> CountryTreeColumns ;
    public:
        DXCCGridModel();
        ~DXCCGridModel() Q_DECL_OVERRIDE;
        QSharedPointer<HtmlDelegate> delegate;

        LoggerContestLog *ct;
        QString band;

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
class DXCCSortFilterProxyModel : public QSortFilterProxyModel
{
public:
    QString scrolledCountry;
    QString band;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    DXCCSortFilterProxyModel()
    {
    }
};

class DXCCFrame : public QFrame
{
    Q_OBJECT

    LoggerContestLog *ct = nullptr;
    DXCCGridModel model;
    DXCCSortFilterProxyModel proxyModel;
    QSharedPointer<HtmlDelegate> delegate ;
    QString band;
    QMenu columnsMenu;
    bool inRestoreColumns = false;
    TSingleLogFrame *tslf = nullptr;

public:
    explicit DXCCFrame(StackedInfoFrame  *parent);
    ~DXCCFrame() override;

    void setContest(LoggerContestLog *contest);
    void setBand(QString band);
    void reInitialiseCountries();
    void scrollToCountry(const QString &bp, bool makeVisible );
private:
    Ui::DXCCFrame *ui;
    void doScrollToCountry();
    
    void saveDXCCTableColumns();

    void restoreDXCCTableColumns();

private slots:
    void on_sectionResized(int, int , int);
    void on_DXCCTable_clicked(const QModelIndex &index);
    void onDXCCGrid_customContextMenuRequested(const QPoint &pos);
    void onDXCCGrid_sectionMoved(int, int, int);

    void viewColumn();

    void on_doColumnChanges(BaseContestLog *b);
    void on_c1Button_clicked();
    void on_c2Button_clicked();
    void on_c3Button_clicked();
    void on_c4Button_clicked();
    void on_c5Button_clicked();
    void on_c6Button_clicked();
    void on_wkdButton_clicked();
    void on_unwkdButton_clicked();
};

#endif // DXCCFRAME_H
