#include "clusternodelistmodel.h"


#include "clusternodelistmodel.h"
#include <QDataStream>
#include <QFile>
#include <QMessageBox>


namespace {
const qint32 MagicNumber = 0x5A697043;
const qint16 FormatNumber = 100;
}

//const int InvalidClusterNode =  - 1;

enum Column {clusterName, clusterAddress, clusterPortNumber, clusterPassword};



ClusterNodeListModel::ClusterNodeListModel(QObject *parent)
    : QStandardItemModel(parent)
{
    initialize();

}


void ClusterNodeListModel::initialize()
{
    setHorizontalHeaderLabels(QStringList() << tr("Name")
            << tr("Address") << tr("Port Number") << tr("Password"));
}


void ClusterNodeListModel::clear()
{
    QStandardItemModel::clear();
    initialize();
}


void ClusterNodeListModel::load(const QString &filename)
{
    if (!filename.isEmpty())
    {
        m_filename = filename;
    }
    if (m_filename.isEmpty())
    {
        errorDialogue(QString("no filename specified"));
        return;
    }
    QFile file(m_filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        errorDialogue(QString("%1").arg(file.errorString()));
        return;

    }

    QDataStream in(&file);
    qint32 magicNumber;
    in >> magicNumber;
    if (magicNumber != MagicNumber)
    {
        errorDialogue(QString("%1").arg("unrecognized file type"));
        return;
    }
    qint16 formatVersionNumber;
    in >> formatVersionNumber;
    if (formatVersionNumber > FormatNumber)
    {
        errorDialogue(QString("%1").arg("file format version is too new"));
        return;
    }
    in.setVersion(QDataStream::Qt_4_5);
    clear();

    QString clusterNodeName;
    QString clusterNodeAddress;
    QString clusterNodePortNumber;
    QString clusterNodePassword;
    QMap<QString, QList<QStandardItem*> > itemsForClusterNodeList;

    while (!in.atEnd())
    {
        in >> clusterNodeName >> clusterNodeAddress >> clusterNodePortNumber >> clusterNodePassword;
        QList<QStandardItem*> items;
        QStandardItem *item = new QStandardItem;
        //item->setData(zipcode, Qt::EditRole);
        items << item;
        foreach (const QString &text, QStringList() << clusterNodeAddress
                                      << clusterNodePortNumber << clusterNodePassword)
        {
            items << new QStandardItem(text);
        }
        itemsForClusterNodeList[clusterNodeName] = items;
    }

    QMapIterator<QString, QList<QStandardItem*> > i(itemsForClusterNodeList);
    while (i.hasNext())
        appendRow(i.next().value());
}


void ClusterNodeListModel::save(const QString &filename)
{
    if (!filename.isEmpty())
        m_filename = filename;
    if (m_filename.isEmpty())
    {
        errorDialogue(QString("no filename specified"));
    }
    QFile file(m_filename);
    if (!file.open(QIODevice::WriteOnly))
    {
        errorDialogue(QString(file.errorString()));
    }
    QDataStream out(&file);
    out << MagicNumber << FormatNumber;
    out.setVersion(QDataStream::Qt_4_5);
    for (int row = 0; row < rowCount(); ++row) {
        out << item(row, clusterName)
            << item(row, clusterAddress)->text()
            << item(row, clusterPortNumber)->text()
            << item(row, clusterPassword)->text();
    }
}



void ClusterNodeListModel::errorDialogue(QString msg)
{

    QMessageBox msgBox;
    msgBox.setText(QString(msg));
    msgBox.exec();
}
