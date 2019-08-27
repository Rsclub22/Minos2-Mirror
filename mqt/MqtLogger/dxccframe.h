#ifndef DXCCFRAME_H
#define DXCCFRAME_H

#include "base_pch.h"

namespace Ui {
class DXCCFrame;
}

class LoggerContestLog;

class DXCCGridModel: public QAbstractItemModel
{
    public:
        DXCCGridModel();
        ~DXCCGridModel() Q_DECL_OVERRIDE;
        QSharedPointer<HtmlDelegate> delegate;

        LoggerContestLog *ct;

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
    int scrolledCountry;
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    DXCCSortFilterProxyModel(): scrolledCountry(-1)
    {
    }
};

class DXCCFrame : public QFrame
{
    Q_OBJECT

    DXCCGridModel model;
    DXCCSortFilterProxyModel proxyModel;
    QSharedPointer<HtmlDelegate> delegate ;

public:
    explicit DXCCFrame(QWidget *parent = nullptr);
    ~DXCCFrame() override;

    void setContest(LoggerContestLog *contest);
    void reInitialiseCountries();
    void scrollToCountry( int ctry_ind, bool makeVisible );

private:
    Ui::DXCCFrame *ui;
private slots:
    void on_sectionResized(int, int , int);
    void on_DXCCTable_clicked(const QModelIndex &index);
};

#endif // DXCCFRAME_H
