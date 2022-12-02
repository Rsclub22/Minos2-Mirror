#ifndef MONITORTREEMODEL_H
#define MONITORTREEMODEL_H

#include <QSharedPointer>
#include <QAbstractItemModel>

class MonitorMain;
class MonitoredLog;

struct MonitoredLogCmp
{
   QString cmpstr;
   MonitoredLogCmp( const QString &s ) : cmpstr( s )
   {}

   bool operator() ( QSharedPointer<MonitoredLog> s1 ) const;
};
//=============================================================================================
enum NodeType{entRoot, entRouter, entLog};
class TreeNode: public QObject
{
    NodeType ntype;
protected:
    QString NodeName;
    QString hintString;
    int childNo;
    TreeNode *parentItem;
    QSharedPointer< MonitoredLog> mlog;

public:
    MonitorMain *monmain;

    TreeNode(NodeType sn, TreeNode *parent, QString name, MonitorMain *mm);
    TreeNode(NodeType sn, TreeNode *parent, QSharedPointer< MonitoredLog> log, MonitorMain *mm);
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
    QSharedPointer< MonitoredLog> getLog()
    {
        return mlog;
    }
//    void setLog(QSharedPointer< MonitoredLog>l)
//    {
//        mlog = l;
//    }
};
class RootTreeNode:public TreeNode
{
public:
    RootTreeNode(MonitorMain *mm):TreeNode(entRoot, nullptr, "Root", mm)
    {

    }
    virtual QString data( int column );
};
class RouterTreeNode:public TreeNode
{
public:
    RouterTreeNode(TreeNode *parent, QString name):TreeNode(entRouter, parent, name, parent->monmain)
    {
    }
    virtual QString data( int column );
};
class LogTreeNode:public TreeNode
{
    Q_OBJECT
public:
    LogTreeNode(TreeNode *parent,QSharedPointer< MonitoredLog> log):TreeNode(entLog, parent, log, parent->monmain)
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

#endif // MONITORTREEMODEL_H
