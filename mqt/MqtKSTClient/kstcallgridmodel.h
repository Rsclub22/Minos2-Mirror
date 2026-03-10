#ifndef KSTCALLGRIDMODEL_H
#define KSTCALLGRIDMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "callsign.h"
#include "htmldelegate.h"
#include "airscoutlink.h"



enum CallColumns {ecscChat, ecscCall, ecscLoc, ecscDistance, ecscBearing, ecscAirscout, ecscName, ecscCountryPrefix, ecscCountryName, ecscMaxColumn};

class Aircraft;
class KstUser
{
public:
    int chat;
    Callsign call;
    QString loc;
    QString name;
    QString prefix;
    QString country;
    QString dxcc;
    bool away = false;
    bool recent = false;
    int distance = -1;
    int bearing = -1;
    int messageCount = 0;

    QString lastCalcTime;
    QString fromCall;
    QString fromLoc;
    QString toCall;
    QString toLoc;
    QVector<Aircraft> planes;
    bool planeResponseSeen = false;

    KstUser()
    {}
    KstUser(const Callsign &c, int achat):call(c),chat(achat)
    {}

    bool operator< ( const KstUser& rhs ) const;
    bool operator== ( const KstUser& rhs ) const;

    qHashRet qHash() const;
};
extern bool KstUserCompare (QSharedPointer<KstUser> i, QSharedPointer<KstUser> j);

class KstCallGridModel: public QAbstractItemModel
{
    Q_OBJECT

    QString filterString;
    int chatFilter = 0;
    bool awayFilter = false;
    bool inactiveFilter = false;

    void checkDistBear(QSharedPointer<KstUser> crec) const;
    bool isFiltered() const
    {
        return !filterString.isEmpty() || chatFilter != 0;
    }
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
        int rawCount( const QModelIndex &parent = QModelIndex() )const;

        void appendRow(QSharedPointer<KstUser> kstmsg);
        void insertRow(int row, QSharedPointer<KstUser> call);
        void reset();
        void removeRow(int _row);

        void setFilterString(QString f);
        void setChatFilter(int value);
        void setAwayFilter(bool value);
        void setInactiveFilter(bool value);

};

class KstCallGridSortFilterModel: public QSortFilterProxyModel
{
    QStringList filterStrings;
    QStringList filterSyns;
    int chatFilter = 0;
    bool awayFilter = false;
    bool inactiveFilter = false;
    bool workedFilter = false;
    bool filterDxcc = false;
    bool isFiltered() const
    {
        return !filterStrings.isEmpty() || chatFilter != 0;
    }
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);

    void setChatFilter(int value);

    void setStringDXCC(bool dxcc);

    void setAwayFilter(bool value);
    void setInactiveFilter(bool value);
    void setWorkedFilter(bool value);
protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

#endif // KSTCALLGRIDMODEL_H
