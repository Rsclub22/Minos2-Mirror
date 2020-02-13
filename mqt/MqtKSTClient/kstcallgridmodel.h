#ifndef KSTCALLGRIDMODEL_H
#define KSTCALLGRIDMODEL_H

#include "base_pch.h"
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "htmldelegate.h"

enum CallColumns {ecscChat, ecscCall, ecscLoc, ecscDistance, ecscBearing, ecscName, ecscCountryPrefix, ecscCountryName, ecscMaxColumn};

class KstUser
{
public:
    int chat;
    QString call;
    QString loc;
    QString name;
    QString prefix;
    QString country;
    bool away = false;
    bool recent = false;
    int distance = -1;
    int bearing = -1;

    bool operator< ( const KstUser& rhs ) const;

};
extern bool KstUserCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j);

class KstCallGridModel: public QAbstractItemModel
{
    Q_OBJECT

    void checkDistBear(QSharedPointer<KstUser> crec) const;
public:
        QString locator;
        KstCallGridModel();
        virtual ~KstCallGridModel() override
        {}
        QSharedPointer<QVector<QSharedPointer<KstUser> > > callVector;
        QSharedPointer<HtmlDelegate> delegate;

        void setCallVector(QSharedPointer<QVector<QSharedPointer<KstUser> > > &pcallVector);
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;


        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index )const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() )const Q_DECL_OVERRIDE;

        void appendRow(QSharedPointer<KstUser> kstmsg);
        void insertRow(int row, QSharedPointer<KstUser> call);
        void reset();
        void removeRow(int _row);
};

class KstCallGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
    int chatFilter = 0;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);

    void setChatFilter(int value);

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // KSTCALLGRIDMODEL_H
