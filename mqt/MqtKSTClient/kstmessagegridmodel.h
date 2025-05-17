#ifndef KSTMESSAGEGRIDMODEL_H
#define KSTMESSAGEGRIDMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QDateTime>
#include "callsign.h"
#include "htmldelegate.h"

enum ChatColumns {eccChat = 0, eccDTG, eccCall, eccName, eccOther, eccText, eccMaxColumn};

class KstMessageLine
{
public:
    bool markedRead = false;
    int sequence = -1;
    int chat = -1;
    QDateTime dtg;
    QString fullLine;
    Callsign call;
    int distance = -1;
    QString name;
    Callsign otherCall;
    int otherDistance = -1;
    QString message;

    KstMessageLine(){}
    ~KstMessageLine(){}

};
bool compMessages ( QSharedPointer<KstMessageLine> q1, const QSharedPointer<KstMessageLine> q2 );

class KstMessageGridModel: public QAbstractItemModel
{
    Q_OBJECT
    private:
        QString filterString;
        int chatFilter = 0;
        bool isFiltered() const
        {
            return !filterString.isEmpty() || chatFilter != 0;
        }
        int pauseIndex = -1;
        bool isPaused() const
        {
            return pauseIndex > 0;
        }
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
        int rawCount(const QModelIndex &parent = QModelIndex()) const;

        void appendLastRow(QSharedPointer<KstMessageLine>);

        void reset();
        void setFilterString(QString f);
        void setChatFilter(int value);
        void setMousePausePoint(int index);

        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role) override;
};

class KstMessageGridSortFilterModel: public QSortFilterProxyModel
{
    QStringList filterStrings;
    int chatFilter = 0;


    bool isFiltered() const
    {
        return !filterStrings.isEmpty() || chatFilter != 0;
    }
    int pauseIndex = -1;
    bool isPaused() const
    {
        return pauseIndex >= 0;
    }

public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setFilterString(QString f);
    void setChatFilter(int value);

    void setMousePausePoint(int index);
};
class KstMeepGridSortFilterModel: public QSortFilterProxyModel
{
    QString myCSfilterString;
    QStringList filterStrings;
    bool showRead = false;
public:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    void setMyCsFilterString(QString f);
    void setFilterString(QString f);
    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
    void setShowRead(bool);
};


#endif // KSTMESSAGEGRIDMODEL_H
