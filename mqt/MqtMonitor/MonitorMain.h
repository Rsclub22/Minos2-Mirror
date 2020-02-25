#ifndef MONITORMAIN_H
#define MONITORMAIN_H

#include "base_pch.h"
#include "cutils.h"
#include <QAction>
#include <QMenu>
#include "ScreenContact.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"


class MonitoringFrame;
class MonitoredLog;

class MonitoredStation
{
   private:

      // Plus we want the control (keyer) state, frequency, ?bandmap etc

   public:
      QString stationName;
      QString publisher;
      PublishState state;
      QVector< MonitoredLog *> slotList;

      MonitoredStation()
      {}
      ~MonitoredStation()
      {}   // need to delete slots...
};
struct MonitoredStationCmp
{
   QString cmpstr;
   QString pub;
   MonitoredStationCmp( const QString &s, QString p ) : cmpstr( s ), pub(p)
   {}

   bool operator() ( MonitoredStation * &s1 ) const
   {
      return s1->stationName.compare( cmpstr, Qt::CaseInsensitive ) == 0 && s1->publisher.compare(pub) == 0;
   }
};
struct MonitoredLogCmp
{
   QString cmpstr;
   MonitoredLogCmp( const QString &s ) : cmpstr( s )
   {}

   bool operator() ( MonitoredLog * &s1 ) const
   {
      return s1->getPublishedName().compare(cmpstr, Qt::CaseInsensitive ) == 0;
   }
};
//=============================================================================================
enum NodeType{entRoot, entServer, entLog};
class TreeNode: public QObject
{
    NodeType ntype;
protected:
    QString NodeName;
    QString hintString;
    int childNo;
    TreeNode *parentItem;
    MonitoredLog *mlog;

public:
    MonitorMain *monmain;

    TreeNode(NodeType sn, TreeNode *parent, QString name, MonitorMain *mm);
    TreeNode(NodeType sn, TreeNode *parent, MonitoredLog *log, MonitorMain *mm);
    virtual ~TreeNode();

    virtual NodeType GetNodeType() const
    {
        return ntype;
    }
    QVector<TreeNode *> nodes;
    int childNumber() const;

    int find( const TreeNode *t ) const;

    void clear();

    virtual QString Name (  ) const
    {
        return NodeName;
    }
    virtual QString hint() const
    {
        return hintString;
    }
    virtual QString data( int column ) = 0;

    TreeNode *parent();
    TreeNode *child( int number );
    int childCount() const;
    MonitoredLog *getLog()
    {
        return mlog;
    }
    void setLog(MonitoredLog *l)
    {
        mlog = l;
    }

};
class RootTreeNode:public TreeNode
{
public:
    RootTreeNode(MonitorMain *mm):TreeNode(entRoot, nullptr, "Root", mm)
    {

    }
    virtual QString data( int column );
};
class ServerTreeNode:public TreeNode
{
public:
    ServerTreeNode(TreeNode *parent, QString name):TreeNode(entServer, parent, name, parent->monmain)
    {
    }
    virtual QString data( int column );
};
class LogTreeNode:public TreeNode
{
    Q_OBJECT
public:
    LogTreeNode(TreeNode *parent, MonitoredLog *log):TreeNode(entLog, parent, log, parent->monmain)
    {
    }
    virtual QString data( int column );
};
class MonitorTreeModel: public QAbstractItemModel
{
    Q_OBJECT

    TreeNode *rootData;
public:
    MonitorTreeModel();
    ~MonitorTreeModel() override;
    void setRoot(  TreeNode *root );
    void clear();

    QVariant data( const QModelIndex &index, int role ) const Q_DECL_OVERRIDE;
    QVariant headerData( int section, Qt::Orientation orientation,
                         int role ) const Q_DECL_OVERRIDE;
    QModelIndex index( int row, int column,
                       const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    QModelIndex parent( const QModelIndex &index ) const Q_DECL_OVERRIDE;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    int columnCount( const QModelIndex &parent = QModelIndex() ) const Q_DECL_OVERRIDE;
    TreeNode *getItem( const QModelIndex &index ) const;
};

namespace Ui {
class MonitorMain;
}

class MonitorMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit MonitorMain(QWidget *parent = nullptr);
    ~MonitorMain() override;

    QVector<MonitoredStation *> stationList;
    ScreenContact screenContact;

    void closeTab(MonitoringFrame *tab);

    int getContestSlotCount();
    BaseContestLog *getContestSlot(int);
    BaseContestLog *getCurrentContest();

private slots:
    void on_notify(bool err, QSharedPointer<MinosRPCObj> mro, const QString &from );
    void on_serverCall( bool err, QSharedPointer<MinosRPCObj>, const QString &from );

    void onStdInRead(QString cmd);

    void on_monitorTimeout();

    void on_monitorTree_doubleClicked(const QModelIndex &index);
    void on_monitorSplitter_splitterMoved(int /*pos*/, int /*index*/);
    void on_closeMonitoredLog();
    void on_contestPageControl_customContextMenuRequested(const QPoint &pos);
    void CancelClick();

    void on_contestPageControl_tabCloseRequested(int index);

    void on_callsignEdit_textChanged(const QString &arg1);

    void on_locEdit_textChanged(const QString &arg1);

    void on_exchangeEdit_textChanged(const QString &arg1);

    void on_contestPageControl_currentChanged(int index);

    void on_mainSplitter_splitterMoved(int pos, int index);

    void on_searchSplitter_splitterMoved(int pos, int index);

private:
    Ui::MonitorMain *ui;
    UpperCaseValidator ucValidator;

    QAction *newAction( const QString &text, QMenu *m, const char *atype );
    QMenu TabPopup;
    QAction *closeMonitoredLog;


    int splitterHandleWidth;

    virtual void closeEvent(QCloseEvent *event) override;
    virtual void resizeEvent(QResizeEvent *event) override;
    virtual void moveEvent(QMoveEvent *event) override;
    virtual void changeEvent( QEvent* e ) override;
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

    StdInReader stdinReader;
    QString localServerName;

    MonitorTreeModel *treeModel;

    QTimer *monitorTimer;

    bool syncstat = false;
    void syncStations();
    void addSlot( MonitoredLog *ct );
    MonitoringFrame *findCurrentLogFrame();
    MonitoringFrame *findContestPage( BaseContestLog *ct );
    void searchChanged();
};

extern MonitorMain *monitorMain;

#endif // MONITORMAIN_H
