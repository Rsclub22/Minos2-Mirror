#ifndef QSOGRIDMODEL_H
#define QSOGRIDMODEL_H
#include <QString>
#include <QAbstractItemView>
#include "GridColumn.h"

class HtmlDelegate;

class BaseContestLog;

enum eLogGridCols {egDate,
                    egTime,
                    egBand,
                    egMode,
                    egCall,
                    egRSTTx, egSNTx, egRSTRx, egSNRx,
                    egLoc, egBrg, egScore,
                    egExchange, egComments,
                    egRigName, egFrequency, egRotatorHeading,
                    egOperator,
                    egLogMaxCol
};

class QSOGridModel: public QAbstractItemModel
{
    Q_OBJECT
protected:
    BaseContestLog *contest = nullptr;
public:
    static QVector<GridColumn> QSOTreeColumns;


    QSOGridModel();
    ~QSOGridModel() Q_DECL_OVERRIDE;
    QSharedPointer<HtmlDelegate> delegate;

    void reset();
    void setContest( BaseContestLog * pcontest );
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
    QVariant headerData( int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
    QModelIndex index( int row, int column,
                      const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;

    virtual bool insertRows(int row, int count, const QModelIndex &index = QModelIndex()) override;
    void changeRow(int row);
};

#endif // QSOGRIDMODEL_H
