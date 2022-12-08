#include "MatchThread.h"
#include "MonitoredLog.h"
#include "MonitoringFrame.h"
#include "MonitorMain.h"

//=============================================================================================
TreeNode::TreeNode(NodeType sn, TreeNode *parent, QString name, MonitorMain *mm):
    ntype(sn), NodeName(name), parentItem(parent), mlog(nullptr), monmain(mm)
{
    if (parent)
        parent->nodes.push_back(this);
}
TreeNode::TreeNode(NodeType sn, TreeNode *parent, QSharedPointer<MonitoredLog> log, MonitorMain *mm):
    ntype(sn), NodeName(log->getDisplayName()), hintString(log->getPublishedName()), parentItem(parent), mlog(log), monmain(mm)
{
    if (parent)
        parent->nodes.push_back(this);
}
TreeNode:: ~TreeNode()
{
    clear();
}
int TreeNode::find( const TreeNode *t ) const
{
    int i = 0;
    for ( auto v : nodes )
    {
        if ( v == t )
            return i;
        i++;
    }
    return 0;
}

TreeNode *TreeNode::parent()
{
    return parentItem;
}

TreeNode *TreeNode::child( int number )
{
    return nodes[ number ];
}

int TreeNode::childCount() const
{
    return nodes.size();
}
int TreeNode::childNumber() const
{
    if ( parentItem )
    {
        return parentItem->find( this );
    }
    return 0;
}
void TreeNode::clear()
{
    for ( auto const &tn: qAsConst(nodes) )
    {
        delete tn;
    }
    nodes.clear();
}
QString RootTreeNode::data(int /*column*/)
{
    return Name();
}
QString RouterTreeNode::data(int column)
{
    if (column == 0)
        return Name();
    return "";
}
static QStringList stateList =
{
   "P",
   "R",
   "NC"
};
QString LogTreeNode::data(int column)
{
    if (column == 1)
        return Name();

    if (column == 0)
    {
        QString state;
        if (mlog->getFrame())
            state = tr("Monitoring");
        return state;
    }
    return "";
}
//=============================================================================
MonitorTreeModel::MonitorTreeModel()
        : QAbstractItemModel( nullptr ), rootData( nullptr )
{}
MonitorTreeModel::~MonitorTreeModel()
{
    delete rootData;
}
void MonitorTreeModel::clear()
{
    beginResetModel();

    delete rootData;
    rootData = nullptr;

    endResetModel();
}
void MonitorTreeModel::setRoot(  TreeNode *root )
{
    beginResetModel();
    delete rootData;
    rootData = root;
    // And we probably need to tell the view that everything has changed
    endResetModel();
}

int MonitorTreeModel::columnCount( const QModelIndex & parent  ) const
{
    TreeNode *parentItem = getItem( parent );
    if (parentItem && parentItem->GetNodeType() == entRouter)
        return 2;

    return 2;
}

QVariant MonitorTreeModel::data( const QModelIndex &index, int role ) const
{
    if ( !index.isValid() )
        return QVariant();

    if ( role == Qt::DisplayRole )
    {
        TreeNode *item = getItem( index );

        return item->data( index.column() );
    }
    if (role == Qt::ToolTipRole)
    {
        TreeNode *item = getItem( index );

        return item->hint();
    }
    return QVariant();
}

QVariant MonitorTreeModel::headerData( int section, Qt::Orientation orientation,
                     int role ) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        QString cell;
        switch (section)
        {
        case 1:
            cell = tr("Contest Name");
            break;

        case 0:
            cell = tr("State");
            break;

        default:
            break;
        }

        return cell;
    }
    return QVariant();
}

QModelIndex MonitorTreeModel::index( int row, int column, const QModelIndex &parent ) const
{
    if ( parent.isValid() && parent.column() != 0 )
        return QModelIndex();

    TreeNode *parentItem = getItem( parent );

    if ( parentItem && row < parentItem->childCount() && row >= 0 )
    {
        TreeNode * childItem = parentItem->child( row );
        if ( childItem )
            return createIndex( row, column, childItem );
    }
    return QModelIndex();
}
QModelIndex MonitorTreeModel::parent( const QModelIndex &index ) const
{
    if ( !index.isValid() )
        return QModelIndex();

    TreeNode *childItem = getItem( index );
    TreeNode *parentItem = childItem->parent();

    if ( parentItem == rootData )
        return QModelIndex();

    return createIndex( parentItem->childNumber(), 0, parentItem );
}
int MonitorTreeModel::rowCount( const QModelIndex &parent ) const
{
    TreeNode * parentItem = getItem( parent );

    if ( parentItem )
        return parentItem->childCount();

    return 0;
}
TreeNode *MonitorTreeModel::getItem( const QModelIndex &index ) const
{
    if ( index.isValid() )
    {
        TreeNode * item = static_cast<TreeNode *>( index.internalPointer() );
        if ( item )
            return item;
    }
    return rootData;
}

