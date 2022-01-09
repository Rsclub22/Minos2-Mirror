#ifndef DXCCFRAME_H
#define DXCCFRAME_H

#include "base_pch.h"

namespace Ui {
class DXCCFrame;
}

class LoggerContestLog;

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
    QString band;

public:
    explicit DXCCFrame(QWidget *parent = nullptr);
    ~DXCCFrame() override;

    void setContest(LoggerContestLog *contest);
    void setBand(QString band);
    void reInitialiseCountries();
    void scrollToCountry(const QString &bp, bool makeVisible );

private:
    Ui::DXCCFrame *ui;
    void doScrollToCountry();
    
private slots:
    void on_sectionResized(int, int , int);
    void on_DXCCTable_clicked(const QModelIndex &index);
};

#endif // DXCCFRAME_H
