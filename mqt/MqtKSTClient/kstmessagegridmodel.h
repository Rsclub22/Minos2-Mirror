#ifndef KSTMESSAGEGRIDMODEL_H
#define KSTMESSAGEGRIDMODEL_H

#include "base_pch.h"

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "htmldelegate.h"


enum ChatColumns {eccChat = 0, eccDTG, eccCall, eccName, eccOther, eccText, eccMaxColumn};

class KstMessageLine
{
public:
    int sequence;
    int chat;
    QString dtg;
    QString fullLine;
    QString call;
    QString name;
    QString otherCall;
    QString message;

    KstMessageLine(){}
    ~KstMessageLine(){}

};
bool compMessages ( QSharedPointer<KstMessageLine> q1, const QSharedPointer<KstMessageLine> q2 );

class KstMessageGridModel: public QAbstractItemModel
{
    Q_OBJECT

        QSize cacheSize;
    public:
        KstMessageGridModel();
        virtual ~KstMessageGridModel() override
        {}
        QSharedPointer<QVector <QSharedPointer<KstMessageLine> > > messageVector;
        QSharedPointer<HtmlDelegate> delegate;

        void setChatVector(QSharedPointer<QVector<QSharedPointer<KstMessageLine> > > pchatVector);
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;


        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index )const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() )const Q_DECL_OVERRIDE;

        void appendLastRow(QSharedPointer<KstMessageLine>);

        void setCacheSize();
        void reset();
};

class KstMessageGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
    int chatFilter = 0;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
    void setChatFilter(int value);
};
class KstMeepGridSortFilterModel: public QSortFilterProxyModel
{
    QString filterString;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
};


#endif // KSTMESSAGEGRIDMODEL_H
