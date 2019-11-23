#ifndef KSTCALLGRIDMODEL_H
#define KSTCALLGRIDMODEL_H

#include "base_pch.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "htmldelegate.h"

class KstCallGridModel: public QAbstractItemModel
{
    Q_OBJECT

    public:
        KstCallGridModel();
        virtual ~KstCallGridModel() override
        {}
        QSharedPointer<QStringList> callVector;
        QSharedPointer<HtmlDelegate> delegate;

        void setCallVector(QSharedPointer<QStringList > pcallVector);
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;


        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index )const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() )const Q_DECL_OVERRIDE;

        void appendRow(QString kstmsg);
        void insertRow(int row);
        void reset();
};

class KstCallGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
};

#endif // KSTCALLGRIDMODEL_H
