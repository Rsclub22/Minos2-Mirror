#ifndef RIGMEMORYFRAME_H
#define RIGMEMORYFRAME_H

#include "base_pch.h"
#include "rigmemcommondata.h"

namespace Ui {
class RigMemoryFrame;
}
class RigMemoryFrame;

class RigMemoryGridModel: public QAbstractItemModel
{
    public:
        RigMemoryGridModel();
        ~RigMemoryGridModel();

        BaseContestLog *ct = 0;
        RigMemoryFrame *frame = 0;

        void beginResetModel(){QAbstractItemModel::beginResetModel();}
        void endResetModel(){QAbstractItemModel::endResetModel();}
        void reset();
        QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
        QVariant headerData( int section, Qt::Orientation orientation,
                             int role = Qt::DisplayRole ) const Q_DECL_OVERRIDE;
        QModelIndex index( int row, int column,
                           const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

        int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
        int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
};
class RigMemorySortFilterProxyModel : public QSortFilterProxyModel
{
public:
    BaseContestLog *ct = 0;

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    RigMemorySortFilterProxyModel()
    {
    }
protected:
    virtual bool lessThan(const QModelIndex &left,
                          const QModelIndex &right) const Q_DECL_OVERRIDE;

};

class HeaderData
{
public:
    QString text;
    QColor colour = Qt::black;
};
class RigMemoryFrame : public QFrame
{
    Q_OBJECT

    void sendUpdateMemories();
    RigMemoryGridModel model;
    RigMemorySortFilterProxyModel proxyModel;

public:
    explicit RigMemoryFrame(QWidget *parent = 0);
    ~RigMemoryFrame();

    QMap<int, HeaderData> headerVal;

    void setContest( BaseContestLog *ct );

    void doMemoryUpdates();


private slots:
    void checkTimerTimer();

    void on_AfterLogContact( BaseContestLog *ct);

    void onRigFreqChanged(QString /*f*/, BaseContestLog *c);

    void onRotBearingChanged(int /*f*/, BaseContestLog *c);

    void on_newMemoryButton_clicked();

    void vsectionClicked(int logicalIndex);

    void on_sectionMoved(int logicalIndex, int oldVisualIndex, int newVisualIndex);
    void on_sectionResized(int logicalIndex, int oldSize, int newSize);
    void on_sortIndicatorChanged(int logicalIndex, Qt::SortOrder order);
    void on_rigMemTable_customContextMenuRequested( const QPoint &pos );
    void rigMemTable_Hdr_customContextMenuRequested( const QPoint &pos );

    void readActionSelected();
    void bearingActionSelected();
    void editActionSelected();
    void writeActionSelected();
    void clearActionSelected();
    void clearAllActionSelected();
    void clearWorkedActionSelected();

    void onMenuShow();

    void on_rigMemTable_doubleClicked(const QModelIndex &index);

    void on_rigMemTable_clicked(const QModelIndex &index);

private:
    Ui::RigMemoryFrame *ui;
    LoggerContestLog *ct = 0;
    bool suppressSendUpdate = false;
    QString lastRigFreq;
    int lastBearing = 0;
    bool lastVisible = false;
    bool doTimer = false;

    QMenu* memoryMenu;

    QAction* newAction;
    QAction* readAction;
    QAction* bearingAction;
    QAction* writeAction;
    QAction* editAction;
    QAction* clearAction;
    QAction* clearAllAction;
    QAction* clearWorkedAction;

    void reloadColumns();
    void saveAllColumnWidthsAndPositions();
    void setRigMemoryData(int memoryNumber, memoryData::memData m);
    void writeMemory(int n);
    int getSelectedLine();
    void traceMsg(QString msg);
    void scrollIntoView ( int firstMatch );

};

#endif // RIGMEMORYFRAME_H
